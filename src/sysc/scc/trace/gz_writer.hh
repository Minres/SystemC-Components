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
#include <mutex>
#include <deque>
#include <string>
#include <cstring>
#include <zlib.h>
#include <chrono>

namespace scc {
namespace trace {

class gz_writer {
    static const size_t queue_size=16*1024;
    std::deque<char*> pool;
    std::deque<char*> write_queue;
    boost::atomic<bool> done {false};
    std::condition_variable cond;

    std::deque<char*> pool_buffer;
    gzFile vcd_out{nullptr};
    std::thread logger;

    inline void write_out(){
        while (!write_queue.empty()) {
            auto* value = write_queue.front();
            write_queue.pop_front();
            auto len = strlen(value);
            if(len) gzwrite(vcd_out, value, strlen(value));
            memset(value, 0, buffer_size);
            pool.push_back(value);
        }
    }
    void log(void){
        auto const timeout = std::chrono::milliseconds(1);
        while (!done) {
            std::unique_lock<std::mutex> lock(writer_mtx);
            auto now = std::chrono::system_clock::now();
            cond.wait_until(lock, now+timeout, [this]() -> bool { return done || ! write_queue.empty(); });
            write_out();
        }
        write_out();
    }

    void enlarge_pool() {
        auto buffer_pool = new char[queue_size * buffer_size];
        for (auto i = 0u; i < queue_size; ++i) {
            memset(buffer_pool + i * buffer_size, 0, buffer_size);
            pool.push_back(buffer_pool + i * buffer_size);
        }
        pool_buffer.push_back(buffer_pool);
    }

public:
    std::mutex writer_mtx;
    using lock_type=std::unique_lock<std::mutex>;
    static const size_t buffer_size=512;
    gz_writer(std::string const& filename)  {
        vcd_out = gzopen(filename.c_str(), "w3");
        logger=std::thread([this](){log();});
        enlarge_pool();
    }

    ~gz_writer(){
        done=true;
        {
            lock_type lock(writer_mtx);
            write("");
        }
        logger.join();
        gzclose(vcd_out);
        char* value;
        pool.clear();
        for(auto b:pool_buffer) delete b;
    }

    inline void write_single(std::string const& msg){
        lock_type lock(writer_mtx);
        if(pool.empty()) enlarge_pool();
        auto value = pool.front();
        pool.pop_front();
        strncpy(value, msg.c_str(), std::min(buffer_size-1, msg.length()));
        write_queue.push_back(value);
    }
    inline void write(std::string const& msg){
        if(pool.empty()) enlarge_pool();
//        auto value = pool.front();
//        pool.pop_front();
//        strncpy(value, msg.c_str(), std::min(buffer_size-1, msg.length()));
//        write_queue.push_back(value);
    }
    inline void write(char const* msg, size_t size){
        if(pool.empty()) enlarge_pool();
//        auto value = pool.front();
//        pool.pop_front();
//        strncpy(value, msg, std::min(buffer_size-1, size));
//        write_queue.push_back(value);
    }
};
}
}
#endif /* _SCC_TRACE_GZ_WRITER_HH_ */
