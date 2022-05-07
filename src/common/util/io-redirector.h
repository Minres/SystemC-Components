/*******************************************************************************
 * Copyright 2019 MINRES Technologies GmbH
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

#ifndef _UTIL_IO_REDIRECTOR_H_
#define _UTIL_IO_REDIRECTOR_H_

#include <fcntl.h>
#include <mutex>
#include <stdio.h>

/**
 * \ingroup scc-common
 */
/**@{*/
namespace util {
/**
 * @brief allows to capture the strings written to std::cout and std::cerr (MT-safe)
 *
 */
class IoRedirector {
    enum { bufSize = 1024 };

public:
    void start();

    void stop();

    bool is_active();

    std::string get_output(bool blocking = false);

    static IoRedirector& get() {
        static IoRedirector inst;
        return inst;
    }

private:
    enum PIPES { READ, WRITE };

    IoRedirector();

    int copy_fd(int fd);
    void create_pipes();
    void copy_fd_to(int src_fd, int destfd);
    void close_fd(int& fd);

    int m_pipe[2]{};
    int m_oldStdOut;
    int m_oldStdErr;
    bool m_capturing;
    std::mutex m_mutex;
    std::string m_captured;
};
} // namespace util
/** @} */
#endif /* _UTIL_IO_REDIRECTOR_H_ */
