/*******************************************************************************
 * Copyright 2019-2024 MINRES Technologies GmbH
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

#ifndef _TILELINK_TL_TLM_H_
#define _TILELINK_TL_TLM_H_

#include <array>
#include <cstdint>
#include <tlm>

//! TLM2.0 components modeling APB
namespace tilelink {
/**
 * helper function to allow SFINAE
 */
template<typename Enum> struct enable_for_enum {
    static const bool value = false;
};
/**
 * helper function to convert integer into class enums
 * @param t
 * @return
 */
template<typename E> inline E into(typename std::underlying_type<E>::type t);
/**
 * helper function to convert class enums into integer
 * @param t
 * @return
 */
template<typename E, typename ULT = typename std::underlying_type<E>::type, typename X = typename std::enable_if<
        std::is_enum<E>::value && !std::is_convertible<E, ULT>::value, bool>::type>
inline constexpr ULT to_int(E t) {
    return static_cast<typename std::underlying_type<E>::type>(t);
}
/**
 * helper function to convert class enums into char string
 * @param t
 * @return
 */
template<typename E> const char* to_char(E t);
/**
 *
 * @param os
 * @param e
 * @return
 */
template<typename E, typename std::enable_if<enable_for_enum<E>::value, bool>::type>
inline std::ostream& operator<<(std::ostream &os, E e) {
    os << to_char(e);
    return os;
}

std::ostream& operator<<(std::ostream &os, tlm::tlm_generic_payload const & t);

//! opcodes of Tilelink.. The encode the opcode (opcode_e[2:0]) and the applicable channels (opcode_e[8] -> A ... opcode_e[4] -> E)
enum class opcode_e : uint8_t {
    Get = 0x184,
    AccessAckData = 0x061,
    PutFullData = 0x180,
    PutPartialData = 0x181,
    AccessAck = 0x060,
    ArithmeticData = 0x182,
    LogicalData = 0x183,
    Intent = 0x185,
    HintAck = 0x062,
    AcquireBlock = 0x106,
    AcquirePerm = 0x107,
    Grant = 0x024,
    GrantData = 0x025,
    GrantAck = 0x010,
    ProbeBlock = 0x086,
    ProbePerm = 0x087,
    ProbeAck = 0x044,
    ProbeAckData = 0x045,
    Release = 0x046,
    ReleaseData = 0x047,
    ReleaseAck = 0x026,
    ILLEGAL = 0xf
};

struct tilelink_extension : public tlm::tlm_extension<tilelink_extension> {
    opcode_e get_opcode() const;
    void set_opcode(opcode_e);

    uint8_t get_param() const;
    void set_param(uint8_t);

    uint64_t get_source() const;
    void set_source(uint64_t);

    uint64_t get_sink() const;
    void set_sink(uint64_t);

    bool is_corrupt() const;
    void set_corrupt(bool = true);

    bool is_denied() const;
    void set_denied(bool = true);

    tilelink_extension() = default;

    tilelink_extension(const tilelink_extension& o) = default;
    /**
     * @brief the clone function to create deep copies of
     * @return pointer to heap-allocated extension
     */
    tlm::tlm_extension_base* clone() const override;
    /**
     * @brief deep copy all values from ext
     * @param ext
     */
    void copy_from(tlm::tlm_extension_base const& ext) override;
private:
    uint64_t source{0};
    uint64_t sink{0};
    bool corrupt{false};
    bool denied{false};
    uint8_t param{0};
    opcode_e opcode{opcode_e::ILLEGAL};
};
//! aliases for payload and phase types
using tl_payload = tlm::tlm_generic_payload;
using tl_phase = tlm::tlm_phase;
/**
 * @brief The AXI protocol traits class.
 * Since the protocoll defines additional non-ignorable phases a dedicated protocol traits class has to be defined.
 */
struct tl_protocol_types {
    typedef tl_payload tlm_payload_type;
    typedef tl_phase tlm_phase_type;
};
/**
 * definition of the additional protocol phases
 */
DECLARE_EXTENDED_PHASE(ACK);
/**
 * interface definition for the blocking backward interface. This is need to allow snoop accesses in blocking mode
 */
template <typename TRANS = tlm::tlm_generic_payload>
class bw_blocking_transport_if : public virtual sc_core::sc_interface {
public:
    /**
     * @brief snoop access to a snooped master
     * @param trans the payload
     * @param t annotated delay
     */
    virtual void b_snoop(TRANS& trans, sc_core::sc_time& t) = 0;
};
//! alias declaration for the forward interface
template <typename TYPES = tl_protocol_types> using tlu_fw_transport_if = tlm::tlm_fw_transport_if<TYPES>;
//! alias declaration for the backward interface:
template <typename TYPES = tl_protocol_types> using tlu_bw_transport_if = tlm::tlm_bw_transport_if<TYPES>;
//! alias declaration for the ACE forward interface
template <typename TYPES = tl_protocol_types> using tlc_fw_transport_if = tlm::tlm_fw_transport_if<TYPES>;
/**
 *  The ACE backward interface which combines the TLM2.0 backward interface and the @see bw_blocking_transport_if
 */
template <typename TYPES = tl_protocol_types>
class tlc_bw_transport_if : public tlm::tlm_bw_transport_if<TYPES>,
                            public virtual bw_blocking_transport_if<typename TYPES::tlm_payload_type> {};

/**
 * TL-UL/-UH initiator socket class
 */
template <unsigned int BUSWIDTH = 32, typename TYPES = tl_protocol_types, int N = 1,
          sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND>
struct tlu_initiator_socket
: public tlm::tlm_base_initiator_socket<BUSWIDTH, tlu_fw_transport_if<TYPES>, tlu_bw_transport_if<TYPES>, N, POL> {
    //! base type alias
    using base_type =
        tlm::tlm_base_initiator_socket<BUSWIDTH, tlu_fw_transport_if<TYPES>, tlu_bw_transport_if<TYPES>, N, POL>;
    /**
     * @brief default constructor using a generated instance name
     */
    tlu_initiator_socket()
    : base_type() {}
    /**
     * @brief constructor with instance name
     * @param name
     */
    explicit tlu_initiator_socket(const char* name)
    : base_type(name) {}
    /**
     * @brief get the kind of this sc_object
     * @return the kind string
     */
    const char* kind() const override { return "tlu_initiator_socket"; }
    // not the right version but we assume TLM is always bundled with SystemC
#if SYSTEMC_VERSION >= 20181013 // ((TLM_VERSION_MAJOR > 2) || (TLM_VERSION==2 && TLM_VERSION_MINOR>0) ||(TLM_VERSION==2
                                // && TLM_VERSION_MINOR>0 && TLM_VERSION_PATCH>4))
    sc_core::sc_type_index get_protocol_types() const override { return typeid(TYPES); }
#endif
};
/**
 * TL-UL/-UH target socket class
 */
template <unsigned int BUSWIDTH = 32, typename TYPES = tl_protocol_types, int N = 1,
          sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND>
struct tlu_target_socket
: public tlm::tlm_base_target_socket<BUSWIDTH, tlu_fw_transport_if<TYPES>, tlu_bw_transport_if<TYPES>, N, POL> {
    //! base type alias
    using base_type =
        tlm::tlm_base_target_socket<BUSWIDTH, tlu_fw_transport_if<TYPES>, tlu_bw_transport_if<TYPES>, N, POL>;
    /**
     * @brief default constructor using a generated instance name
     */
    tlu_target_socket()
    : base_type() {}
    /**
     * @brief constructor with instance name
     * @param name
     */
    explicit tlu_target_socket(const char* name)
    : base_type(name) {}
    /**
     * @brief get the kind of this sc_object
     * @return the kind string
     */
    const char* kind() const override { return "tlu_target_socket"; }
    // not the right version but we assume TLM is always bundled with SystemC
#if SYSTEMC_VERSION >= 20181013 // ((TLM_VERSION_MAJOR > 2) || (TLM_VERSION==2 && TLM_VERSION_MINOR>0) ||(TLM_VERSION==2
                                // && TLM_VERSION_MINOR>0 && TLM_VERSION_PATCH>4))
    sc_core::sc_type_index get_protocol_types() const override { return typeid(TYPES); }
#endif
};
/**
 * TL-C initiator socket class
 */
template <unsigned int BUSWIDTH = 32, typename TYPES = tl_protocol_types, int N = 1,
          sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND>
struct tlc_initiator_socket
: public tlm::tlm_base_initiator_socket<BUSWIDTH, tlc_fw_transport_if<TYPES>, tlc_bw_transport_if<TYPES>, N, POL> {
    //! base type alias
    using base_type =
        tlm::tlm_base_initiator_socket<BUSWIDTH, tlc_fw_transport_if<TYPES>, tlc_bw_transport_if<TYPES>, N, POL>;
    /**
     * @brief default constructor using a generated instance name
     */
    tlc_initiator_socket()
    : base_type() {}
    /**
     * @brief constructor with instance name
     * @param name
     */
    explicit tlc_initiator_socket(const char* name)
    : base_type(name) {}
    /**
     * @brief get the kind of this sc_object
     * @return the kind string
     */
    const char* kind() const override { return "tlc_initiator_socket"; }
#if SYSTEMC_VERSION >= 20181013 // not the right version but we assume TLM is always bundled with SystemC
    /**
     * @brief get the type of protocol
     * @return the kind typeid
     */
    sc_core::sc_type_index get_protocol_types() const override { return typeid(TYPES); }
#endif
};
/**
 * TL-C target socket class
 */
template <unsigned int BUSWIDTH = 32, typename TYPES = tl_protocol_types, int N = 1,
          sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND>
struct tlc_target_socket
: public tlm::tlm_base_target_socket<BUSWIDTH, tlc_fw_transport_if<TYPES>, tlc_bw_transport_if<TYPES>, N, POL> {
    //! base type alias
    using base_type =
        tlm::tlm_base_target_socket<BUSWIDTH, tlc_fw_transport_if<TYPES>, tlc_bw_transport_if<TYPES>, N, POL>;
    /**
     * @brief default constructor using a generated instance name
     */
    tlc_target_socket()
    : base_type() {}
    /**
     * @brief constructor with instance name
     * @param name
     */
    explicit tlc_target_socket(const char* name)
    : base_type(name) {}
    /**
     * @brief get the kind of this sc_object
     * @return the kind string
     */
    const char* kind() const override { return "tlc_target_socket"; }
    // not the right version but we assume TLM is always bundled with SystemC
#if SYSTEMC_VERSION >= 20181013 // not the right version but we assume TLM is always bundled with SystemC
    /**
     * @brief get the type of protocol
     * @return the kind typeid
     */
    sc_core::sc_type_index get_protocol_types() const override { return typeid(TYPES); }
#endif
};
/*****************************************************************************
 * free function easing handling of transactions and extensions
 *****************************************************************************/
/*****************************************************************************
 * Implementation details
 *****************************************************************************/
template <> struct enable_for_enum<opcode_e> { static const bool enable = true; };

inline opcode_e tilelink_extension::get_opcode() const {
    return opcode;
}

inline void tilelink_extension::set_opcode(opcode_e opcode) {
    this->opcode=opcode;
}

inline uint8_t tilelink_extension::get_param() const {
    return param;
}

inline void tilelink_extension::set_param(uint8_t param) {
    this->param=param;
}

inline uint64_t tilelink_extension::get_source() const {
    return source;
}

inline void tilelink_extension::set_source(uint64_t source) {
    this->source=source;
}

inline uint64_t tilelink_extension::get_sink() const {
    return sink;
}

inline void tilelink_extension::set_sink(uint64_t sink) {
    this->sink=sink;
}

inline bool tilelink_extension::is_corrupt() const {
    return corrupt;
}

inline void tilelink_extension::set_corrupt(bool corrupt) {
    this->corrupt=corrupt;
}

inline bool tilelink_extension::is_denied() const {
    return denied;
}

inline void tilelink_extension::set_denied(bool denied) {
    this->denied=denied;
}

inline tlm::tlm_extension_base* tilelink_extension::clone() const {
    return new tilelink_extension(this);
}

inline void tilelink_extension::copy_from(const tlm::tlm_extension_base &ext) {
    auto const* tl_ext = dynamic_cast<const tilelink_extension*>(&ext);
    assert(tl_ext);
    (*this) = *tl_ext;
}
} // namespace } // namespace apb


#endif /* _TILELINK_TL_TLM_H_ */
