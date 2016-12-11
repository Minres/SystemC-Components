/*
 * resource_access_if.h
 *
 *  Created on: Nov 16, 2016
 *      Author: developer
 */

#ifndef SYSC_RESOURCE_ACCESS_IF_H_
#define SYSC_RESOURCE_ACCESS_IF_H_

#include <cstddef>
#include <cstdint>

namespace sysc {

struct resource_access_if {
    virtual ~resource_access_if(){}
    virtual size_t size() const = 0;
    virtual void reset() = 0;
    // functional accesses
    virtual bool write(const uint8_t* data, size_t length) = 0;
    virtual bool read(uint8_t* data, size_t length) const = 0;
    // non-functional/debug accesses
    virtual bool write_dbg(const uint8_t* data, size_t length) = 0;
    virtual bool read_dbg(uint8_t* data, size_t length) const = 0;
};

}

#endif /* SYSC_RESOURCE_ACCESS_IF_H_ */
