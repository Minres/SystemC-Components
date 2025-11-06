/*******************************************************************************
 * Copyright (C) 2025 MINRES Technologies GmbH
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************************/

#ifndef _ISS_LOGGER_H_
#define _ISS_LOGGER_H_

#include "delegate.h"
#include "logging.h"

namespace util {

struct LoggerDelegate {
    using delegate_fn = void(logging::log_level, std::string const&, std::string const&, unsigned, char const*);
    util::delegate<delegate_fn> log;
    logging::log_level level;
};

/**
 * @brief InstanceLogger -  an instance based logger facade which falls back to the logging based global c++ logger
 *
 * @tparam CATEGORY
 */
template <typename CATEGORY> class InstanceLogger {
public:
    InstanceLogger() = default;

    ~InstanceLogger() = default;

    InstanceLogger(const InstanceLogger&) = delete;

    InstanceLogger& operator=(const InstanceLogger&) = delete;

    logging::log_level get_log_level() {
        if(logger) {
            return logger->level;
        } else {
            return ::logging ::Log<::logging ::Output2FILE<CATEGORY>>::get_reporting_level();
        }
    }
    void log(logging::log_level level, const std::string& message, unsigned line, char const* file) {
        if(logger) {
            logger->log(level, CATEGORY::name, message, line, file);
        } else {
            if(level <= _LOGGER(CATEGORY)::get_reporting_level() && _LOG_OUTPUT(CATEGORY)::stream())
                ::logging::Log<::logging::Output2FILE<CATEGORY>>().get(level, CATEGORY::name) << message;
        }
    }

    void set_logger(LoggerDelegate& logger) { this->logger = &logger; }

private:
    LoggerDelegate* logger;
};
} // namespace util

#define ILOG(LOGGER, LEVEL, MSG) LOGGER.log(LEVEL, MSG, __LINE__, __FILE__)
#endif // _ISS_LOGGER_H_