#ifndef _TLM_AHB_PE_AHB_TARGET_H_

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <array>
#include <functional>
#include <scc/ordered_semaphore.h>
#include <tlm/ahb/ahb_tlm.h>
#include <unordered_set>

namespace ahb {
namespace pe {
/**
 * the target protocol engine base class
 */
class ahb_target_b : public sc_core::sc_module, public tlm::tlm_fw_transport_if<tlm::tlm_base_protocol_types> {
public:
    SC_HAS_PROCESS(ahb_target_b);

    using payload_type = tlm::tlm_base_protocol_types::tlm_payload_type;
    using phase_type = tlm::tlm_base_protocol_types::tlm_phase_type;

    sc_core::sc_in<bool> clk_i{"clk_i"};
    /**
     * @brief the latency between between BEGIN(_PARTIAL)_REQ and END(_PARTIAL)_REQ (AWVALID to AWREADY and WVALID to
     * WREADY)
     */
    sc_core::sc_attribute<unsigned> wr_data_accept_delay{"wr_data_accept_delay", 0};
    /**
     * @brief the latency between between BEGIN_REQ and END_REQ (ARVALID to ARREADY)
     */
    sc_core::sc_attribute<unsigned> rd_addr_accept_delay{"rd_addr_accept_delay", 0};
    /**
     * @brief the latency between between END(_PARTIAL)_RESP and BEGIN(_PARTIAL)_RESP (RREADY to RVALID)
     */
    sc_core::sc_attribute<unsigned> rd_data_beat_delay{"rd_data_beat_delay", 0};
    /**
     * @brief the latency between request and response phase. Will be overwritten by the return of the callback function
     * (if registered)
     */
    sc_core::sc_attribute<unsigned> rd_resp_delay{"rd_resp_delay", 0};
    /**
     * @brief the latency between request and response phase. Will be overwritten by the return of the callback function
     * (if registered)
     */
    sc_core::sc_attribute<unsigned> wr_resp_delay{"wr_resp_delay", 0};

    /** @defgroup fw_if Initiator foreward interface
     *  @{
     */
    void b_transport(payload_type& trans, sc_core::sc_time& t) override;

    tlm::tlm_sync_enum nb_transport_fw(payload_type& trans, phase_type& phase, sc_core::sc_time& t) override;

    bool get_direct_mem_ptr(payload_type& trans, tlm::tlm_dmi& dmi_data) override;

    unsigned int transport_dbg(payload_type& trans) override;
    /** @} */ // end of fw_if
    /** @defgroup config Initiator configuration interface
     *  @{
     */

    /**
     * @brief Set the operation callback function
     *
     * This callback is invoked once a transaction arrives. This function is not allowed to block and returns the
     * latency of the operation i.e. the duration until the reponse phase starts
     * @todo refine API
     *
     * @param cb the callback function
     */

    void set_operation_cb(std::function<unsigned(payload_type& trans)> cb) { operation_cb = cb; }
    /**
     *
     * @param trans
     * @param sync
     */
    void operation_resp(payload_type& trans, bool sync = false);

protected:
    /**
     * the constructor. Protected as it should only be called by derived classes
     *
     * @param nm the module name
     * @param port
     * @param transfer_width
     */
    explicit ahb_target_b(const sc_core::sc_module_name& nm,
                          sc_core::sc_port_b<tlm::tlm_bw_transport_if<tlm::tlm_base_protocol_types>>& port,
                          size_t transfer_width);

    ahb_target_b() = delete;

    ahb_target_b(ahb_target_b const&) = delete;

    ahb_target_b(ahb_target_b&&) = delete;

    ahb_target_b& operator=(ahb_target_b const&) = delete;

    ahb_target_b& operator=(ahb_target_b&&) = delete;

    void send_resp_thread();

    sc_core::sc_port_b<tlm::tlm_bw_transport_if<tlm::tlm_base_protocol_types>>& socket_bw;
    sc_core::sc_semaphore sn_sem{1};
    sc_core::sc_mutex wr, rd, sn;
    bool fast_resp{false};
    bool fast_req{false};
    std::function<unsigned(payload_type& trans)> operation_cb;
    scc::ordered_semaphore rd_resp{1}, wr_resp{1};
    sc_core::sc_clock* clk_if{nullptr};
    void end_of_elaboration() override;
};

/**
 * the target socket protocol engine adapted to a particular target socket configuration
 */
template <unsigned int BUSWIDTH = 32, typename TYPES = tlm::tlm_base_protocol_types, int N = 1,
          sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND>
class ahb3_target : public ahb_target_b {
public:
    using base = ahb_target_b;
    using payload_type = base::payload_type;
    using phase_type = base::phase_type;
    /**
     * @brief the constructor
     * @param socket reference to the initiator socket used to send and receive transactions
     */
    ahb3_target(tlm::tlm_target_socket<BUSWIDTH, TYPES, N, POL>& socket)
    : // @suppress("Class members should be properly initialized")
        ahb3_target(sc_core::sc_gen_unique_name("simple_target"), socket) {}

    ahb3_target(const sc_core::sc_module_name& nm, tlm::tlm_target_socket<BUSWIDTH, TYPES, N, POL>& socket)
    : ahb_target_b(nm, socket.get_base_port(), BUSWIDTH)
    , socket(socket) {
        socket(*this);
    }

    ahb3_target() = delete;

    ahb3_target(ahb3_target const&) = delete;

    ahb3_target(ahb3_target&&) = delete;

    ahb3_target& operator=(ahb3_target const&) = delete;

    ahb3_target& operator=(ahb3_target&&) = delete;

private:
    tlm::tlm_target_socket<BUSWIDTH, TYPES, N, POL>& socket;
};

} // namespace pe
} // namespace ahb

#endif
