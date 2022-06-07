/*******************************************************************************
 * Copyright 2019-2022 MINRES Technologies GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *******************************************************************************/

#include "io-redirector.h"
#include <array>
#ifdef _MSC_VER
#include <io.h>
#define popen _popen
#define pclose _pclose
#define stat _stat
#define dup _dup
#define dup2 _dup2
#define fileno _fileno
#define close _close
#define pipe _pipe
#define read _read
#define eof _eof
#else
#include <poll.h>
#include <thread>
#include <unistd.h>
#endif
namespace util {
IoRedirector::IoRedirector()
: m_oldStdOut(0)
, m_oldStdErr(0)
, m_capturing(false) {
    // make stdout & stderr streams unbuffered
    std::lock_guard<std::mutex> lock(m_mutex);
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);
}

void IoRedirector::start() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if(m_capturing)
        return;

    create_pipes();
    m_oldStdOut = copy_fd(fileno(stdout));
    m_oldStdErr = copy_fd(fileno(stderr));
    copy_fd_to(m_pipe[WRITE], fileno(stdout));
    copy_fd_to(m_pipe[WRITE], fileno(stderr));
    m_capturing = true;
#ifndef _MSC_VER
    close_fd(m_pipe[WRITE]);
#endif
}

auto IoRedirector::is_active() -> bool {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_capturing;
}

void IoRedirector::stop() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if(!m_capturing)
        return;

    m_captured.clear();
    copy_fd_to(m_oldStdOut, fileno(stdout));
    copy_fd_to(m_oldStdErr, fileno(stderr));

    std::array<char, bufSize> buf;
    int bytesRead = 0;
    bool fd_blocked(false);
    do {
        fd_blocked = false;
#ifdef _MSC_VER
        if(!eof(m_pipe[READ]))
            bytesRead = read(m_pipe[READ], buf.data(), bufSize - 1);
#else
        int flags = fcntl(m_pipe[READ], F_GETFL, 0);
        fcntl(m_pipe[READ], F_SETFL, flags & ~O_NONBLOCK);
        bytesRead = read(m_pipe[READ], buf.data(), bufSize - 1);
#endif
        if(bytesRead > 0) {
            buf[bytesRead] = 0;
            m_captured += buf.data();
        } else if(bytesRead < 0) {
            fd_blocked = (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR);
            if(fd_blocked)
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    } while(fd_blocked || bytesRead == (bufSize - 1));

    close_fd(m_oldStdOut);
    close_fd(m_oldStdErr);
    close_fd(m_pipe[READ]);
#ifdef _MSC_VER
    close_fd(m_pipe[WRITE]);
#endif
    m_capturing = false;
}

auto IoRedirector::get_output(bool blocking) -> std::string {
    std::lock_guard<std::mutex> lock(m_mutex);
    if(m_capturing) {
        std::string ret;
        std::array<char, bufSize> buf;
        int bytesRead = 0;
        bool fd_blocked(false);
        do {
            fd_blocked = false;
#ifdef _MSC_VER
            if(!eof(m_pipe[READ]))
                bytesRead = read(m_pipe[READ], buf.data(), bufSize - 1);
#else
            int flags = fcntl(m_pipe[READ], F_GETFL, 0);
            if(blocking)
                fcntl(m_pipe[READ], F_SETFL, flags & ~O_NONBLOCK);
            else
                fcntl(m_pipe[READ], F_SETFL, flags | O_NONBLOCK);
            bytesRead = read(m_pipe[READ], buf.data(), bufSize - 1);
#endif
            if(bytesRead > 0) {
                buf[bytesRead] = 0;
                ret += buf.data();
            } else if(bytesRead < 0) {
                fd_blocked = errno == EINTR;
                if(fd_blocked)
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        } while(fd_blocked || bytesRead == (bufSize - 1));
        return ret;
    } else {
        return m_captured;
    }
}

auto IoRedirector::copy_fd(int fd) -> int {
    int ret = -1;
    bool fd_blocked = false;
    do {
        ret = dup(fd);
        if(ret < 0 && (errno == EINTR || errno == EBUSY))
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
    } while(ret < 0);
    return ret;
}
void IoRedirector::create_pipes() {
    int ret = -1;
    do {
#ifdef _MSC_VER
        ret = pipe(m_pipe, 65536, O_BINARY);
#else
        ret = pipe(m_pipe) == -1;
#endif
        if(ret < 0 && (errno == EINTR || errno == EBUSY))
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
    } while(ret < 0);
}
void IoRedirector::copy_fd_to(int src_fd, int dest_fd) {
    int ret = -1;
    do {
        ret = dup2(src_fd, dest_fd);
        if(ret < 0 && (errno == EINTR || errno == EBUSY))
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
    } while(ret < 0);
}

void IoRedirector::close_fd(int& fd) {
    int ret = -1;
    do {
        ret = close(fd);
        if(ret < 0 && errno == EINTR)
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
    } while(ret < 0);
    fd = -1;
}
} // namespace util
