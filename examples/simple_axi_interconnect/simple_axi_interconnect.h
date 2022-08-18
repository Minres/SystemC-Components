/*
 * ace_axi_adapt.h
 *
 *  Created on: Aug 13, 2022
 *      Author: tientr
 */
#ifndef __SIMPLE_AXI_INTERCONNECTION__
#define __SIMPLE_AXI_INTERCONNECTION__

#include "generic_extension.h"
#include <axi/axi_initiator.h>
#include <axi/axi_target.h>
#include <axi/axi_tlm.h>
#include <fstream>
#include <iostream>
#include <map>

template <uint64_t SOCKET_WIDTH, uint64_t MASTER_NUM, uint64_t SLAVE_NUM>
class simple_axi_interconnect : public sc_core::sc_module,
                                public axi::axi_fw_transport_if<axi::axi_protocol_types>,
                                public axi::axi_bw_transport_if<axi::axi_protocol_types> {
private:
    std::string file_address_map;
    /* data */
    std::string address_map_path;
    enum e_segment_address {
        max_segmemt_e = 3,
    };
    const int invalid_slave = -1;
    const uint address_line_length = 3;
    // map pointer_target: start_address end_address start_address_end_address
    typedef struct address_map_struct {
        std::string socket_name;
        // pointer of initiator
        axi::axi_initiator_socket<SOCKET_WIDTH>* slave_socket_ptr;
        // address
        uint64_t valid_table_index;
        uint64_t start_address[max_segmemt_e];
        uint64_t end_address[max_segmemt_e];
        address_map_struct() {
            slave_socket_ptr = NULL;
            valid_table_index = 0;
            for(size_t i = 0; i < max_segmemt_e; i++) {
                start_address[i] = 0;
                end_address[i] = 0;
            }
        }
    } address_map_struct;

    address_map_struct slave_address_map[SLAVE_NUM];
    // std::map<tlm::tlm_generic_payload * trans_ptr, axi::axi_target_socket<SOCKET_WIDTH> * master_socket_ptr>
    // master_socket_trans_map; internal func
    bool is_a_command_line(std::string text);
    int get_slave_pos(std::string text);
    axi::axi_initiator_socket<SOCKET_WIDTH>* decode(uint64_t address);

public:
    using transaction_type = axi::axi_protocol_types::tlm_payload_type;
    using phase_type = axi::axi_protocol_types::tlm_phase_type;
    axi::axi_initiator_socket<SOCKET_WIDTH>* intit_socket[SLAVE_NUM];
    axi::axi_target_socket<SOCKET_WIDTH>* target_socket[MASTER_NUM];
    sc_core::sc_in<bool> clk_i{"clk_i"};

    simple_axi_interconnect(sc_core::sc_module_name name, std::string file_address_map);
    ~simple_axi_interconnect();

    void end_of_elaboration(void) override;
    void b_transport(transaction_type& trans, sc_core::sc_time& t) override;
    bool get_direct_mem_ptr(transaction_type& trans, tlm::tlm_dmi& dmi_data) override { return false; }
    unsigned int transport_dbg(transaction_type& trans) override { return 0; }

    tlm::tlm_sync_enum nb_transport_fw(transaction_type& trans, phase_type& phase, sc_core::sc_time& t) override;

    tlm::tlm_sync_enum nb_transport_bw(transaction_type& trans, phase_type& phase, sc_core::sc_time& t) override;

    void invalidate_direct_mem_ptr(sc_dt::uint64 start_range, sc_dt::uint64 end_range) override{};
};

/**
 * @brief Construct a new simple axi interconnect::simple axi interconnect object
 *
 * @tparam SOCKET_WIDTH
 * @tparam MASTER_NUM
 * @tparam SLAVE_NUM
 * @param name
 * @param file_address_map
 */
template <uint64_t SOCKET_WIDTH, uint64_t MASTER_NUM, uint64_t SLAVE_NUM>
simple_axi_interconnect<SOCKET_WIDTH, MASTER_NUM, SLAVE_NUM>::simple_axi_interconnect(sc_core::sc_module_name name,
                                                                                      std::string file_address_map)
: sc_core::sc_module(name) {
    this->file_address_map = file_address_map;
    for(size_t socket_index = 0; socket_index < SLAVE_NUM; socket_index++) {
        // initiator connect to slave
        std::string name_of_socket = "init_socket_" + std::to_string(socket_index);
        std::string full_name_initiator = std::string(name) + "." + name_of_socket;
        intit_socket[socket_index] = new axi::axi_initiator_socket<SOCKET_WIDTH>(name_of_socket.c_str());
        intit_socket[socket_index]->bind(*this);
        slave_address_map[socket_index].socket_name = full_name_initiator;
        slave_address_map[socket_index].slave_socket_ptr = intit_socket[socket_index];
    }

    for(size_t socket_index = 0; socket_index < MASTER_NUM; socket_index++) {
        // target connect to master
        std::string name_of_socket = "target_socket_" + std::to_string(socket_index);
        std::string full_name_initiator = std::string(name) + "." + name_of_socket;
        target_socket[socket_index] = new axi::axi_target_socket<SOCKET_WIDTH>(name_of_socket.c_str());
        target_socket[socket_index]->bind(*this);
    }
}

/**
 * @brief Destroy the simple axi interconnect::simple axi interconnect object
 *
 * @tparam SOCKET_WIDTH
 * @tparam MASTER_NUM
 * @tparam SLAVE_NUM
 */
template <uint64_t SOCKET_WIDTH, uint64_t MASTER_NUM, uint64_t SLAVE_NUM>
simple_axi_interconnect<SOCKET_WIDTH, MASTER_NUM, SLAVE_NUM>::~simple_axi_interconnect() {
    for(size_t socket_index = 0; socket_index < SLAVE_NUM; socket_index++) {
        delete intit_socket[socket_index];
    }

    for(size_t socket_index = 0; socket_index < MASTER_NUM; socket_index++) {
        delete target_socket[socket_index];
    }
}

/**
 * @brief
 *
 */
template <uint64_t SOCKET_WIDTH, uint64_t MASTER_NUM, uint64_t SLAVE_NUM>
void simple_axi_interconnect<SOCKET_WIDTH, MASTER_NUM, SLAVE_NUM>::end_of_elaboration(void) {
    // Create a text string, which is used to output the text file
    std::string map_text;

    // Read from the text file
    std::ifstream map_file(file_address_map.c_str());
    std::string space_delimiter = " ";
    // Use a while loop together with the getline() function to read the file line by line
    while(getline(map_file, map_text)) {
        // Output the text from the file
        std::vector<std::string> words{};
        size_t pos = 0;
        if(false == is_a_command_line(map_text)) {
            while((pos = map_text.find(space_delimiter)) != std::string::npos) {
                words.push_back(map_text.substr(0, pos));
                map_text.erase(0, pos + space_delimiter.length());
            }
            if(!map_text.empty()) {
                words.push_back(map_text);
            }
            // instert address
            int slave_pos = get_slave_pos(words[0]);
            if((words.size() >= 3) && (invalid_slave != slave_pos)) {
                // hex support
                uint64_t start_address = strtoll(words[1].c_str(), NULL, 16);
                uint64_t end_address = strtoll(words[2].c_str(), NULL, 16);
                // need to check duplication addaress ::later
                if(slave_address_map[slave_pos].valid_table_index > max_segmemt_e) {
                    SCCWARN(SCMOD) << "Overload address slave: " << slave_pos;
                    continue;
                }
                slave_address_map[slave_pos].start_address[slave_address_map[slave_pos].valid_table_index] =
                    start_address;
                slave_address_map[slave_pos].end_address[slave_address_map[slave_pos].valid_table_index] = end_address;
                slave_address_map[slave_pos].valid_table_index++;
            }
        }
    }

    // Close the file
    map_file.close();
}

/**
 * @brief
 *
 * @param text
 * @return true
 * @return false
 */
template <uint64_t SOCKET_WIDTH, uint64_t MASTER_NUM, uint64_t SLAVE_NUM>
bool simple_axi_interconnect<SOCKET_WIDTH, MASTER_NUM, SLAVE_NUM>::is_a_command_line(std::string text) {
    if((text[0] == '/') || (text[0] == '#')) {
        return true;
    } else {
        return false;
    }
}

/**
 * @brief
 *
 * @param text
 * @return int
 */
template <uint64_t SOCKET_WIDTH, uint64_t MASTER_NUM, uint64_t SLAVE_NUM>
int simple_axi_interconnect<SOCKET_WIDTH, MASTER_NUM, SLAVE_NUM>::get_slave_pos(std::string text) {
    for(size_t i = 0; i < SLAVE_NUM; i++) {
        if(slave_address_map[i].socket_name == text) {
            return i;
        }
    }
    return invalid_slave;
}

/**
 * @brief
 *
 * @param address
 * @return axi::axi_initiator_socket<SOCKET_WIDTH>*
 */
template <uint64_t SOCKET_WIDTH, uint64_t MASTER_NUM, uint64_t SLAVE_NUM>
axi::axi_initiator_socket<SOCKET_WIDTH>*
simple_axi_interconnect<SOCKET_WIDTH, MASTER_NUM, SLAVE_NUM>::decode(uint64_t address) {
    for(size_t slave_index = 0; slave_index < SLAVE_NUM; slave_index++) {
        for(size_t address_index = 0; address_index < slave_address_map[slave_index].valid_table_index; slave_index++) {
            if((slave_address_map[slave_index].start_address[address_index] <= address) &&
               (address < slave_address_map[slave_index].end_address[address_index])) {
                return slave_address_map[slave_index].slave_socket_ptr;
            }
        }
    }
    return NULL;
}

/**
 * @brief
 *
 * @param trans
 * @param t
 */
template <uint64_t SOCKET_WIDTH, uint64_t MASTER_NUM, uint64_t SLAVE_NUM>
void simple_axi_interconnect<SOCKET_WIDTH, MASTER_NUM, SLAVE_NUM>::b_transport(transaction_type& trans,
                                                                               sc_core::sc_time& t) {
    uint64_t trans_address = trans.get_address();
    axi::axi_initiator_socket<SOCKET_WIDTH>* slave_socket_ptr = decode(trans_address);
    if(slave_socket_ptr != NULL) {
        (*slave_socket_ptr)->b_transport(trans, t); // not support
    } else {
        SCCERR(SCMOD) << "Cannot decode address " << trans_address;
    }
}

/**
 * @brief
 *
 * @param trans
 * @param phase
 * @param t
 * @return tlm::tlm_sync_enum
 */
template <uint64_t SOCKET_WIDTH, uint64_t MASTER_NUM, uint64_t SLAVE_NUM>
tlm::tlm_sync_enum
simple_axi_interconnect<SOCKET_WIDTH, MASTER_NUM, SLAVE_NUM>::nb_transport_fw(transaction_type& trans,
                                                                              phase_type& phase, sc_core::sc_time& t) {
    uint64_t trans_address = trans.get_address();
    axi::axi_initiator_socket<SOCKET_WIDTH>* slave_socket_ptr = decode(trans_address);
    if(slave_socket_ptr != NULL) {
        return (*slave_socket_ptr)->nb_transport_fw(trans, phase, t);
    } else {
        printf("Error: Cannot decode address 0x%lx\n", trans_address);
        return tlm::TLM_COMPLETED;
    }
}

/**
 * @brief
 *
 * @param trans
 * @param phase
 * @param t
 * @return tlm::tlm_sync_enum
 */
template <uint64_t SOCKET_WIDTH, uint64_t MASTER_NUM, uint64_t SLAVE_NUM>
tlm::tlm_sync_enum
simple_axi_interconnect<SOCKET_WIDTH, MASTER_NUM, SLAVE_NUM>::nb_transport_bw(transaction_type& trans,
                                                                              phase_type& phase, sc_core::sc_time& t) {
    generic_extension<axi::axi_target_socket<SOCKET_WIDTH>>* generic_ext =
        trans.get_extension<generic_extension<axi::axi_target_socket<SOCKET_WIDTH>>>();
    axi::axi_target_socket<SOCKET_WIDTH>* master_ptr =
        generic_ext->get_bw_if_ptr(); // assign pointer master1
    if(master_ptr != NULL) {
        return (*master_ptr)->nb_transport_bw(trans, phase, t);
    } else {
        SCCERR(SCMOD) << "bw_if_ptr is null ";
        return tlm::TLM_COMPLETED;
    }
}

#endif