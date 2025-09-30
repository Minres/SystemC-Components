/*******************************************************************************
 * Copyright 2021 MINRES Technologies GmbH
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

#ifndef SYSC_SCC_SC_THREAD_POOL_H_
#define SYSC_SCC_SC_THREAD_POOL_H_
#include "peq.h"
#include <cci_configuration>
#include <functional>
#include <systemc>

/** \ingroup scc-sysc
 *  @{
 */
/**@{*/
//! @brief SCC SystemC utilities
namespace scc {
/**
 * \brief A thread pool for executing tasks concurrently.
 *
 * The sc_thread_pool class provides a mechanism for executing SystemC tasks concurrently using a pool of worker SC_THREADs.
 * It allows users to specify the maximum number of concurrent threads and provides a simple interface for submitting tasks to be executed.
 *
 * @author Your Name
 * @date YYYY-MM-DD
 */
class sc_thread_pool : sc_core::sc_object {
public:
    /**
     * \brief The maximum number of concurrent threads in the thread pool.
     *
     * This parameter allows users to specify the maximum number of concurrent threads that can be active in the thread pool at any given
     * time. By default, the maximum number of concurrent threads is set to 16.
     */
    cci::cci_param<unsigned> max_concurrent_threads{"max_concurrent_threads", 16};
    /**
     * \brief Constructor for the sc_thread_pool class.
     */
    sc_thread_pool();
    /**
     * \brief Destructor for the sc_thread_pool class.
     */
    virtual ~sc_thread_pool();
    /**
     * \brief Execute the given function in a separate SC_THREAD.
     *
     * This method submits the given function to be executed in a separate thread.
     * The function will be executed concurrently with other tasks submitted to the thread pool.
     *
     * @param fct The function to be executed.
     */
    void execute(std::function<void(void)> fct);

private:
    scc::peq<std::function<void(void)>> dispatch_queue{"dispatch_queue"};
    unsigned thread_avail{0}, thread_active{0};
};
} /* namespace scc */
/** @} */ // end of scc-sysc
#endif    /* SYSC_SCC_SC_THREAD_POOL_H_ */
