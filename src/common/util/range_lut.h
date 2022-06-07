/*******************************************************************************
 * Copyright 2017, 2018 MINRES Technologies GmbH
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

#ifndef _RANGE_LUT_H_
#define _RANGE_LUT_H_

#include <exception>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

/**
 * \ingroup scc-common
 */
/**@{*/
//! @brief SCC common utilities
namespace util {
/**
 * @brief range based lookup table
 */
template <typename T> class range_lut {
public:
    //! the type of lut entry
    enum entry_type { BEGIN_RANGE = 1, END_RANGE = 2, SINGLE_BYTE_RANGE = 3 };
    //! the lut entry
    struct lut_entry {
        T index;
        entry_type type;
    };

    /**
     * constructor or the lookup table
     *
     * @param null_entry the entry to be used for empty slots
     */
    range_lut(T null_entry)
    : null_entry(null_entry) {}
    /**
     * add an T to the lut covering the range starting at base_addr until
     * base_addr+size-1
     *
     * @param i the entry
     * @param base_addr the base address
     * @param size the size of the occupied range
     */
    void addEntry(T i, uint64_t base_addr, uint64_t size);
    /**
     * remove an entry with value i of type T
     *
     * @param i the entry to be found
     * @return true if the entry is found and removed, false otherwise
     */
    bool removeEntry(T i);
    /**
     * get number of entries in the lookup table
     *
     * @return the size of the underlying container
     */
    size_t size() const { return m_size; }
    /**
     * remove all entries from the lut
     */
    void clear() {
        m_lut.clear();
        m_size = 0;
    }
    /**
     * get the entry T associated with a given address
     *
     * @param addr the address
     * @return the entry belonging to the address
     */
    inline T getEntry(uint64_t addr) const {
        auto iter = m_lut.lower_bound(addr);
        return (iter != m_lut.end() && (iter->second.type == END_RANGE || iter->first == addr)) ? iter->second.index
                                                                                                : null_entry;
    }
    /**
     * validate the lookup table wrt. overlaps
     */
    void validate() const;
    /**
     * create a textual representation of the address map (address range->entry
     * association)
     *
     * @return
     */
    std::string toString() const;
    //! the null entry
    const T null_entry;

    using const_iterator = typename std::map<uint64_t, lut_entry>::const_iterator;

    const_iterator begin() const { return m_lut.begin(); }

    const_iterator end() const { return m_lut.end(); }

protected:
    // Loki::AssocVector<uint64_t, lut_entry> m_lut;
    std::map<uint64_t, lut_entry> m_lut{};
    size_t m_size{0};
};

/**
 * overloaded stream operator
 *
 * @param os the output stream
 * @param lut the lookup table to print
 * @return the stream
 */
template <typename T> std::ostream& operator<<(std::ostream& os, range_lut<T>& lut) {
    os << lut.toString();
    return os;
}

template <typename T> inline void range_lut<T>::addEntry(T i, uint64_t base_addr, uint64_t size) {
    auto iter = m_lut.find(base_addr);
    if(iter != m_lut.end() && iter->second.index != null_entry)
        throw std::runtime_error("range already mapped");

    auto eaddr = base_addr + size - 1;
    if(eaddr < base_addr)
        throw std::runtime_error("address wrap-around occurred");

    m_lut[base_addr] = lut_entry{i, size > 1 ? BEGIN_RANGE : SINGLE_BYTE_RANGE};
    if(size > 1)
        m_lut[eaddr] = lut_entry{i, END_RANGE};
    ++m_size;
}

template <typename T> inline bool range_lut<T>::removeEntry(T i) {
    auto start = m_lut.begin();
    while(start->second.index != i && start != m_lut.end())
        start++;
    if(start != m_lut.end()) {
        if(start->second.type == SINGLE_BYTE_RANGE) {
            m_lut.erase(start);
        } else {
            auto end = start;
            end++;
            end++;
            m_lut.erase(start, end);
        }
        --m_size;
        return true;
    }
    return false;
}

template <typename T> inline void range_lut<T>::validate() const {
    auto mapped = false;
    for(auto iter = m_lut.begin(); iter != m_lut.end(); iter++) {
        switch(iter->second.type) {
        case SINGLE_BYTE_RANGE:
            if(iter->second.index != null_entry && mapped)
                throw std::runtime_error("range overlap: begin range while in mapped range");

            break;
        case BEGIN_RANGE:
            if(iter->second.index != null_entry) {
                if(mapped) {
                    throw std::runtime_error("range overlap: begin range while in mapped range");
                }
                mapped = true;
            }
            break;
        case END_RANGE:
            if(!mapped) {
                throw std::runtime_error("range overlap: end range while in unmapped region");
            }
            mapped = false;
            break;
        }
    }
}

template <typename T> inline std::string range_lut<T>::toString() const {
    std::ostringstream buf;
    for(auto iter = m_lut.begin(); iter != m_lut.end(); ++iter) {
        switch(iter->second.type) {
        case BEGIN_RANGE:
            if(iter->second.index != null_entry) {
                buf << "  from 0x" << std::setw(sizeof(uint64_t) * 2) << std::setfill('0') << std::uppercase << std::hex
                    << iter->first << std::dec;
            }
            break;
        case END_RANGE:
            buf << " to 0x" << std::setw(sizeof(uint64_t) * 2) << std::setfill('0') << std::uppercase << std::hex
                << iter->first << std::dec << " as " << iter->second->index << std::endl;
        }
    }
    return buf.str();
}
} // namespace util
/** @}*/
#endif /* _RANGE_LUT_H_ */
