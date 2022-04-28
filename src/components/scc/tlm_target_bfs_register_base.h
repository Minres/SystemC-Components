/*******************************************************************************
 * Copyright 2021, 2021 Chair of Electronic Design Automation, TU Munich
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
/**
 * @Author: Philip Dachs
 * @Date:   2019-06-19T12:30:13+02:00
 * @Filename: tlm_target_bfs_register_base.h
 * @Last modified by:   Johannes Geier (contact: johannes.geier@tum.de)
 * @Last modified time: 2021-01-20T18:11:00+02:00
 */

#ifndef __SCC_TLM_TARGET_BFS_REGISTER_BASE_H__
#define __SCC_TLM_TARGET_BFS_REGISTER_BASE_H__

#include <algorithm>
#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include <boost/preprocessor/arithmetic/add.hpp>
#include <boost/preprocessor/arithmetic/mul.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/facilities/overload.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/tuple/elem.hpp>

#include "resetable.h"
#include "resource_access_if.h"
#include "sysc/kernel/sc_module_name.h"
#include "sysc/kernel/sc_object.h"
#include "tlm_target.h"

#define ID_SCC_TLM_TARGET_BFS_REGISTER_BASE "scc: tlm target bitfield support register base"

namespace scc {

/**
 * @brief Abstract baseclass for bitfield
 *
 * Used to resolve cyclic dependency between bitfield_register and bitfield.
 */
template <typename datatype_t> class abstract_bitfield {
public:
    constexpr abstract_bitfield(std::string name, size_t bitOffset, size_t bitSize, std::string urid)
    : bitOffset{bitOffset}
    , bitSize{bitSize}
    , name{std::move(name)}
    , urid{std::move(urid)} {}

    virtual void write(datatype_t& valueToWrite) = 0;
    virtual datatype_t read() = 0;

    constexpr bool affected(size_t byteOffset, size_t byteLength) const noexcept {
        return (byteOffset * 8 < bitOffset + bitSize) && (bitOffset < (byteOffset + byteLength) * 8);
    }

    constexpr datatype_t mask() const noexcept { return ((1ULL << bitSize) - 1) << bitOffset; }

    const size_t bitOffset;
    const size_t bitSize;
    const std::string name;
    const std::string urid;
};

/**
 * @brief Register that can contain bitfields
 *
 * @tparam datatype_t Datatype of the register value
 */
template <typename datatype_t> class bitfield_register : public sc_core::sc_object, public scc::resource_access_if {
public:
    /**
     * @param name The name of this register
     * @param offset Offset of the peripheral base address this register is mapped
     * @param resetValue This value gets written to the register on a reset
     *                   condition
     * @param writeMask Only bits that are set get overwritten on a write access.
     *                  Cleared bits are left unchanged. This field has priority
     *                  over the individual bitfields or the write callback.
     * @param readMask Bits that are cleared in this field always read as 0. Can
     *                 be overridden in bitfields or the read callback.
     */
    constexpr bitfield_register(sc_core::sc_module_name name, size_t offset, datatype_t resetValue = 0,
                                datatype_t writeMask = -1, datatype_t readMask = -1)
    : sc_core::sc_object{name}
    , offset{offset}
    , resetValue{resetValue}
    , writeMask{writeMask}
    , readMask{readMask} {}

    /**
     * @return The size of the register in bytes
     */
    constexpr size_t size() const noexcept override { return sizeof(datatype_t); }

    void reset() override { storage = resetValue; }

    bool write(const uint8_t* data, std::size_t length, uint64_t offset = 0,
               sc_core::sc_time d = sc_core::SC_ZERO_TIME) override {
        assert("Access out of range" && offset + length <= this->size());
        auto valueToWrite{storage};
        std::copy(data, data + length, reinterpret_cast<uint8_t*>(&valueToWrite) + offset);
        for(auto&& bitfield : bitfields) {
            if(bitfield.get().affected(offset, length)) {
                auto mask = bitfield.get().mask();
                auto bits = (valueToWrite & mask) >> bitfield.get().bitOffset;
                bitfield.get().write(bits);
                valueToWrite = (valueToWrite & ~mask) | ((bits << bitfield.get().bitOffset) & mask);
            }
        }
        if(writeCallback)
            writeCallback(*this, valueToWrite);
        storage = (valueToWrite & writeMask) | (storage & ~writeMask);
        return true;
    }

    bool read(uint8_t* data, std::size_t length, uint64_t offset = 0,
              sc_core::sc_time d = sc_core::SC_ZERO_TIME) const override {
        assert("Access out of range" && offset + length <= this->size());
        auto result{storage};
        result &= readMask;
        for(auto&& bitfield : bitfields) {
            if(bitfield.get().affected(offset, length)) {
                auto bitfieldValue = bitfield.get().read();
                auto mask = bitfield.get().mask();
                result = (result & ~mask) | ((bitfieldValue << bitfield.get().bitOffset) & mask);
            }
        }
        if(readCallback)
            readCallback(*this, result);
        auto begin = reinterpret_cast<const uint8_t*>(&result) + offset;
        std::copy(begin, begin + length, data);
        return true;
    }

    bool write_dbg(const uint8_t* data, std::size_t length, uint64_t offset = 0) override {
        assert("Offset out of range" && offset == 0);
        if(length != this->size())
            return false;
        std::copy(data, data + length, reinterpret_cast<uint8_t*>(&storage));
        return true;
    }

    bool read_dbg(uint8_t* data, std::size_t length, uint64_t offset = 0) const override {
        assert("Offset out of range" && offset == 0);
        if(length != this->size())
            return false;
        auto storagePtr = reinterpret_cast<const uint8_t*>(&storage);
        std::copy(storagePtr, storagePtr + length, data);
        return true;
    }

    /**
     * @return the data stored in this register
     */
    constexpr datatype_t get() const { return storage; }
    /**
     * Updates the stored data with \p value
     */
    void put(datatype_t value) { storage = value; }

    void registerBitfield(abstract_bitfield<datatype_t>& bitfield) { bitfields.push_back(bitfield); }

    /**
     * Convenience function to access the stored data
     */
    constexpr operator datatype_t() const { return storage; }
    /**
     * Convenience function to update the stored data
     */
    bitfield_register<datatype_t>& operator=(datatype_t other) {
        storage = other;
        return *this;
    }

    /**
     * Register a \p callback that is called on write. Overwrites previously
     * stored callbacks. Signature: `void onWrite(bitfield_register<datatype_t>&
     * reg, datatype_t& valueToWrite)`
     *
     * Callback is called after the individual bitfields.
     *
     * If you want to change the value that gets written to the register change
     * the value of the parameter valueToWrite. Direct writes to the register do
     * not work.
     */
    void setWriteCallback(std::function<void(bitfield_register<datatype_t>&, datatype_t& valueToWrite)> callback) {
        writeCallback = std::move(callback);
    }
    /**
     * Register a \p callback that gets called on read. Overwrites previously
     * stored callbacks. Signature: `void onRead(const
     * bitfield_register<datatype_t>& reg, datatype_t& result)`
     *
     * Callback is called after the individual bitfields.
     */
    void setReadCallback(std::function<void(const bitfield_register<datatype_t>&, datatype_t& result)> callback) {
        readCallback = std::move(callback);
    }

    const size_t offset;

protected:
    const datatype_t resetValue;
    const datatype_t writeMask;
    const datatype_t readMask;
    datatype_t storage;

    std::function<void(bitfield_register<datatype_t>&, datatype_t&)> writeCallback;
    std::function<void(const bitfield_register<datatype_t>&, datatype_t&)> readCallback;
    std::vector<std::reference_wrapper<abstract_bitfield<datatype_t>>> bitfields;
};

template <typename datatype_t> class bitfield : public abstract_bitfield<datatype_t> {
public:
    enum Access { RW, ReadOnly };
    using abstract_bitfield<datatype_t>::mask;
    using abstract_bitfield<datatype_t>::bitOffset;

    /**
     * @param reg The register that contains this bitfield
     * @param name The name of this bitfield
     * @param bitOffset the position of this bitfield in the containing register
     * @param bitSize the size of this bitfield
     * @param urid Unique resource id. Must be unique accross all bitfields in a
     *             peripheral
     * @param access If RO writes to this bitfield are ignored.
     */
    // constexpr //TODO: requires c++14
    bitfield(bitfield_register<datatype_t>& reg, std::string name, size_t bitOffset, size_t bitSize, std::string urid,
             Access access = RW)
    : reg{reg}
    , abstract_bitfield<datatype_t>{std::move(name), bitOffset, bitSize, std::move(urid)}
    , access{access} {
        reg.registerBitfield(*this);
    }
    bitfield(const bitfield&) = delete;
    bitfield& operator=(const bitfield&) = delete;

    void write(datatype_t& valueToWrite) override {
        if(writeCallback)
            writeCallback(*this, valueToWrite);
        if(access == ReadOnly)
            valueToWrite = get();
    }
    datatype_t read() override {
        if(readCallback)
            return readCallback(*this);
        else
            return get();
    }

    /**
     * @return the data stored in this bitfield
     */
    constexpr datatype_t get() const { return (reg.get() & mask()) >> bitOffset; }
    /**
     * Updates the stored data with \p value
     */
    void put(datatype_t value) { reg.put((reg.get() & ~mask()) | ((value << bitOffset) & mask())); }

    /**
     * Convenience function to access the stored data
     */
    constexpr operator datatype_t() const { return get(); }
    /**
     * Convenience function to update the stored data
     */
    bitfield<datatype_t>& operator=(datatype_t other) {
        put(other);
        return *this;
    }

    /**
     * Register a \p callback that is called on write. Overwrites previously
     * stored callbacks. Signature: `void onWrite(bitfield<datatype_t>& bf,
     * datatype_t& valueToWrite)`
     *
     * Callback is called before the register.
     *
     * If you want to change the value that gets written to the bitfield change
     * the value of the parameter valueToWrite. Direct writes to the bitfield do
     * not work.
     */
    void setWriteCallback(std::function<void(bitfield<datatype_t>&, datatype_t& valueToWrite)> callback) {
        writeCallback = std::move(callback);
    }
    /**
     * Register a \p callback that gets called on read. Overwrites previously
     * stored callbacks. Signature: `datatype_t onRead(const
     * bitfield<datatype_t>& bf)`
     *
     * Callback is called before the register.
     */
    void setReadCallback(std::function<datatype_t(const bitfield<datatype_t>&)> callback) {
        readCallback = std::move(callback);
    }

    bitfield_register<datatype_t>& reg;
    Access access;

protected:
    std::function<void(bitfield<datatype_t>&, datatype_t&)> writeCallback;
    std::function<datatype_t(const bitfield<datatype_t>&)> readCallback;
};

/**
 * Base class the register layout classes for \ref peripheral_base_minres must
 * derive from.
 * @tparam derived_t Type of the concrete register class. Used for CRTP.
 * @tparam use_URID Set this to true if the bitfield/register layout is not the
 *                  same as the original. If true, then register lookups work by
 *                  urid and not by regname and bitfield name.
 */
template <typename derived_t, bool use_URID = false>
class tlm_target_bfs_register_base : public sc_core::sc_object, public scc::resetable {
public:
    tlm_target_bfs_register_base(sc_core::sc_module_name name)
    : sc_core::sc_object{name} {}

    template <unsigned buswidth> void registerResources(scc::tlm_target<buswidth>& target) {
        for(auto&& reg : asDerived().registers) {
            target.addResource(reg, reg.offset);
            this->register_resource(&reg);
        }
    }

    /**
     * @brief Search for a register by name.
     *
     * If no matching register is found a FATALERROR is generated.
     */
    bitfield_register<uint32_t>& getRegister(const std::string& name) {
        auto found =
            std::find_if(asDerived().registers.begin(), asDerived().registers.end(),
                         [name](const bitfield_register<uint32_t>& reg) { return name.compare(reg.basename()) == 0; });
        if(found == asDerived().registers.end()) {
            SC_REPORT_FATAL(ID_SCC_TLM_TARGET_BFS_REGISTER_BASE, ("Register " + name + " not found").c_str());
        }
        return *found;
    }

    /**
     * @brief Search for a bitfield by name and register name.
     *
     * If the register layout could change use getBitfield() instead.
     *
     * If no matching bitfield is found a FATALERROR is generated.
     */
    bitfield<uint32_t>& getBitfieldByName(const std::string& regname, const std::string& name) {
        auto found = std::find_if(asDerived().bitfields.begin(), asDerived().bitfields.end(),
                                  [regname, name](const bitfield<uint32_t>& bf) {
                                      return regname.compare(bf.reg.basename()) == 0 && name == bf.name;
                                  });
        if(found == asDerived().bitfields.end()) {
            SC_REPORT_FATAL(ID_SCC_TLM_TARGET_BFS_REGISTER_BASE,
                            ("Bitfield " + name + " in register " + regname + " not found").c_str());
        }
        return *found;
    }

    /**
     * @brief Search for a bitfield by urid
     *
     * If no matching bitfield is found a FATALERROR is generated.
     * @see getBitfield()
     */
    bitfield<uint32_t>& getBitfieldById(const std::string& urid) {
        auto found = std::find_if(asDerived().bitfields.begin(), asDerived().bitfields.end(),
                                  [urid](const bitfield<uint32_t>& bf) { return urid == bf.urid; });
        if(found == asDerived().bitfields.end()) {
            SC_REPORT_FATAL(ID_SCC_TLM_TARGET_BFS_REGISTER_BASE, ("Bitfield with urid " + urid + " not found").c_str());
        }
        return *found;
    }

    /**
     * @brief Preferred way to get access to a bitfield.
     *
     * If use_URID is true searches a bitfield by the specified urid.
     * If use_URID is false searches a bitfield by name and regname. Also
     * generates a warning if the found bitfield has an urid different from the
     * specified urid.
     *
     * If no matching bitfield is found a FATALERROR is generated.
     */
    bitfield<uint32_t>& getBitfield(const std::string& regname, const std::string& name, const std::string& urid) {
        if(use_URID) {
            return getBitfieldById(urid);
        } else {
            bitfield<uint32_t>& result = getBitfieldByName(regname, name);
            if(result.urid != urid) {
                SC_REPORT_WARNING(ID_SCC_TLM_TARGET_BFS_REGISTER_BASE,
                                  ("URID of register is " + result.urid + " but " + urid + " was passed").c_str());
            }
            return result;
        }
    }

private:
    derived_t& asDerived() { return static_cast<derived_t&>(*this); }
};

} // namespace scc

/**
 * @brief Implementation detail. Not for general use
 *
 * @param i Current index
 * @param elem Tuple of (regname, basename, offset, size, uridbase)
 *
 * generates the following code
 * \code
 * {getRegister(<regname>), <basename>"<i>", <offset> + <size> * <i>, <size>,
 * <uridbase>"<i>"<uridpostfix>} \endcode
 */
#define BITFIELD_ARRAY_ELEMENT(z, i, elem)                                                                             \
    BOOST_PP_COMMA_IF(i) {                                                                                             \
        getRegister(BOOST_PP_TUPLE_ELEM(0, elem)), BOOST_PP_TUPLE_ELEM(1, elem) BOOST_PP_STRINGIZE(i),                 \
            BOOST_PP_TUPLE_ELEM(2, elem) + i *BOOST_PP_TUPLE_ELEM(3, elem), BOOST_PP_TUPLE_ELEM(3, elem),              \
            BOOST_PP_TUPLE_ELEM(4, elem) BOOST_PP_STRINGIZE(i) BOOST_PP_TUPLE_ELEM(5, elem)                            \
    }

/**
 * @brief Generates multiple bitfield definitions packed in one register.
 *
 * repeats the following pattern \p count times:
 * \code
 * {getRegister(<regname>), <basename>"<i>", <offset> + <size> * <i>, <size>,
 * <uridbase>"<i>"} \endcode
 *
 * @param regname Name of the register the bitfields are part of
 * @param basename Prefix of the bitfield name. Resulting name is
 * `<basename><i>`
 * @param offset Offset in bits of the first generated bitfield
 * @param size Size in bits of each bitfield
 * @param uridbase Prefix of the urids. Resulting urid is `<uridbase><i>`
 * @param count The number of bitfields to generate
 */
#define BITFIELD_ARRAY(regname, basename, offset, size, uridbase, count)                                               \
    BOOST_PP_REPEAT(count, BITFIELD_ARRAY_ELEMENT, (regname, basename, offset, size, uridbase, ))

/**
 * @brief See BITFIELD_ARRAY. Adds possibility to append postfix to urid
 * pattern.
 *
 * @param uridpostfix Postfix of the urids. Resulting urid is
 * `<uridbase><i><uridpostfix>`
 */
#define BITFIELD_ARRAY_POSTFIX(regname, basename, offset, size, uridbase, uridpostfix, count)                          \
    BOOST_PP_REPEAT(count, BITFIELD_ARRAY_ELEMENT, (regname, basename, offset, size, uridbase, uridpostfix))

/**
 * @brief Implementation detail. Not for general use
 *
 * @param i Current index
 * @param elem Tuple of (basename, offset, size)
 *
 * generates the following code:
 * \code
 * {<basename>"<i>", <offset> + <i> * <size>}
 * \endcode
 */
#define REGISTER_ARRAY_ELEMENT(z, i, elem)                                                                             \
    BOOST_PP_COMMA_IF(i) {                                                                                             \
        BOOST_PP_TUPLE_ELEM(0, elem)                                                                                   \
        BOOST_PP_STRINGIZE(i), BOOST_PP_TUPLE_ELEM(1, elem) + i* BOOST_PP_TUPLE_ELEM(2, elem)                          \
    }

/**
 * @brief Generates multiple register definitions.
 * repeats the following pattern \p count times:
 * \code
 * {<basename>"<i>", <offset> + <i> * <size>}
 * \endcode
 *
 * @param basename Prefix of the register name. Resulting name is
 * `<basename><i>`
 * @param offset Offset in bytes of the first register. Only decimal numbers are
 * supported
 * @param size Size in bytes of each register
 * @param count The number of registers to generate
 */
#define REGISTER_ARRAY(basename, offset, size, count)                                                                  \
    BOOST_PP_REPEAT(count, REGISTER_ARRAY_ELEMENT, (basename, offset, size))

#endif // __SCC_TLM_TARGET_BFS_REGISTER_BASE_H__
