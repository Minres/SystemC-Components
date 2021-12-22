/*
 * gz_writer.hh
 *
 *  Created on: Dec 15, 2021
 *      Author: eyckj
 */

#ifndef _SCC_TRACE_GZ_WRITER_HH_
#define _SCC_TRACE_GZ_WRITER_HH_

#include <boost/lockfree/spsc_queue.hpp>
#include <boost/atomic.hpp>
#include <thread>
#include <string>
#include <cstring>
#include <zlib.h>

namespace scc {
namespace trace {

class gz_writer {
    static const size_t queue_size=128*1024;
    boost::lockfree::spsc_queue<char*, boost::lockfree::capacity<queue_size+1> > empty_queue;
    boost::lockfree::spsc_queue<char*, boost::lockfree::capacity<queue_size> > spsc_queue;
    boost::atomic<bool> done {false};

    char* buffer_pool;
    gzFile vcd_out{nullptr};
    std::thread logger;

    void log(void){
        char* value;
        while (!done) {
            while (spsc_queue.pop(value)) {
                gzwrite(vcd_out, value, strlen(value));
                memset(value, 0, buffer_size);
                while(!empty_queue.push(value));
            }
        }
        while (spsc_queue.pop(value)){
            gzwrite(vcd_out, value, strlen(value));
            memset(value, 0, buffer_size);
            while(!empty_queue.push(value));
        }
    }
public:
    static const size_t buffer_size=512;
    gz_writer(std::string const& filename)  {
        vcd_out = gzopen(filename.c_str(), "w0");
        logger=std::thread([this](){log();});
        buffer_pool=new char[queue_size*buffer_size];
        for(auto i=0u; i<queue_size; ++i){
            memset(buffer_pool+i*buffer_size, 0, buffer_size);
            empty_queue.push(buffer_pool+i*buffer_size);
        }
    }

    ~gz_writer(){
        done=true;
        write("");
        logger.join();
        gzclose(vcd_out);
        char* value;
        while (spsc_queue.pop(value));
        delete buffer_pool;
    }

    void write(std::string const& msg){
        while(empty_queue.empty());
        char* value{nullptr};
        empty_queue.pop(value);
        strncpy(value, msg.c_str(), std::min(buffer_size-1, msg.length()));
        while (!spsc_queue.push(value));
    }
    void write(char const* msg, size_t size){
        while(empty_queue.empty());
        char* value{nullptr};
        empty_queue.pop(value);
        strncpy(value, msg, std::min(buffer_size-1, size));
        while (!spsc_queue.push(value));
    }
};
}
}
#endif /* _SCC_TRACE_GZ_WRITER_HH_ */
