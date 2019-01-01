/*
 * io-redirector.h
 *
 *  Created on: 31.12.2018
 *      Author: eyck
 */

#ifndef _UTIL_IO_REDIRECTOR_H_
#define _UTIL_IO_REDIRECTOR_H_

#include <fcntl.h>
#include <stdio.h>
#include <mutex>

class IoRedirector {
    enum { bufSize = 1024 };
public:
    void start();

    void stop();

    bool is_active();

    std::string get_output();

    static IoRedirector& get(){
        static IoRedirector inst;
        return inst;
    }
private:
    enum PIPES { READ, WRITE };

    IoRedirector();

    int copy_fd(int fd);
    void create_pipes();
    void copy_fd_to(int src_fd, int destfd);
    void close_fd(int & fd);

    int m_pipe[2];
    int m_oldStdOut;
    int m_oldStdErr;
    bool m_capturing;
    std::mutex m_mutex;
    std::string m_captured;
};

#endif /* SC_COMPONENTS_INCL_UTIL_IO_REDIRECTOR_H_ */
