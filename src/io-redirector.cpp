/*
 * io-redirector.cpp
 *
 *  Created on: 31.12.2018
 *      Author: eyck
 */

#include <util/io-redirector.h>
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
#include <unistd.h>
//#include <fcntl.h>
#include <poll.h>
#include <thread>
#endif

IoRedirector::IoRedirector()
: m_oldStdOut(0)
, m_oldStdErr(0)
, m_capturing(false)
{
    // make stdout & stderr streams unbuffered
    std::lock_guard<std::mutex> lock(m_mutex);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
}

void IoRedirector::start() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_capturing) return;

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

bool IoRedirector::is_active() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_capturing;
}

void IoRedirector::stop() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_capturing) return;

    m_captured.clear();
    copy_fd_to(m_oldStdOut, fileno(stdout));
    copy_fd_to(m_oldStdErr, fileno(stderr));


    char buf[bufSize];
    int bytesRead = 0;
    bool fd_blocked(false);
    do {
        bytesRead = 0;
        fd_blocked = false;
#ifdef _MSC_VER
        if (!eof(m_pipe[READ]))
        bytesRead = read(m_pipe[READ], buf, bufSize-1);
#else
        int flags = fcntl(m_pipe[READ], F_GETFL, 0);
        fcntl(m_pipe[READ], F_SETFL, flags &  ~O_NONBLOCK);
        bytesRead = read(m_pipe[READ], buf, bufSize - 1);
#endif
        if (bytesRead > 0) {
            buf[bytesRead] = 0;
            m_captured += buf;
        } else
            if (bytesRead < 0) {
                fd_blocked = (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR);
                if (fd_blocked) std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
    } while (fd_blocked || bytesRead == (bufSize - 1));

    close_fd(m_oldStdOut);
    close_fd(m_oldStdErr);
    close_fd(m_pipe[READ]);
#ifdef _MSC_VER
    close_fd(m_pipe[WRITE]);
#endif
    m_capturing = false;
}

std::string IoRedirector::get_output(bool blocking) {
    std::lock_guard<std::mutex> lock(m_mutex);
    char msg[1024];
    if(m_capturing){
        std::string ret;
        char buf[bufSize];
        int bytesRead = 0;
        bool fd_blocked(false);
        do {
            bytesRead = 0;
            fd_blocked = false;
#ifdef _MSC_VER
            if (!eof(m_pipe[READ]))
            bytesRead = read(m_pipe[READ], buf, bufSize-1);
#else
            int flags = fcntl(m_pipe[READ], F_GETFL, 0);
            if(blocking)
                fcntl(m_pipe[READ], F_SETFL, flags &  ~O_NONBLOCK);
            else
                fcntl(m_pipe[READ], F_SETFL, flags | O_NONBLOCK);
            bytesRead = read(m_pipe[READ], buf, bufSize - 1);
#endif
            if (bytesRead > 0) {
                buf[bytesRead] = 0;
                ret += buf;
            } else
                if (bytesRead < 0) {
                    fd_blocked = errno == EINTR;
                    if (fd_blocked) std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
        } while (fd_blocked || bytesRead == (bufSize - 1));
        return ret;
    } else{
        return m_captured;
    }
}

int IoRedirector::copy_fd(int fd) {
    int ret = -1;
    bool fd_blocked = false;
    do {
        ret = dup(fd);
        if (ret<0 && (errno == EINTR || errno == EBUSY)) std::this_thread::sleep_for(std::chrono::milliseconds(10));
    } while (ret < 0);
    return ret;
}
void IoRedirector::create_pipes() {
    int ret = -1;
    do {
#ifdef _MSC_VER
        ret = pipe(pipes, 65536, O_BINARY);
#else
        ret = pipe(m_pipe) == -1;
#endif
        if (ret<0 && (errno == EINTR || errno == EBUSY)) std::this_thread::sleep_for(std::chrono::milliseconds(10));
    } while (ret < 0);
}
void IoRedirector::copy_fd_to(int src_fd, int dest_fd) {
    int ret = -1;
    do {
        ret = dup2(src_fd, dest_fd);
        if (ret<0 && (errno == EINTR || errno == EBUSY)) std::this_thread::sleep_for(std::chrono::milliseconds(10));
    } while (ret < 0);
}

void IoRedirector::close_fd(int & fd) {
    int ret = -1;
    do {
        ret = close(fd);
        if (ret <0 && errno == EINTR) std::this_thread::sleep_for(std::chrono::milliseconds(10));
    } while (ret < 0);
    fd = -1;
}

