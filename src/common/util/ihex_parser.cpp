
#include "ihex_parser.h"
#include <stdint.h>
#include <stdbool.h>
#include <cstring>

//IHEX file parser state machine
enum {
    START_CODE_STATE = 0,
    BYTE_COUNT_0_STATE = 1,
    BYTE_COUNT_1_STATE = 2,
    ADDR_0_STATE = 3,
    ADDR_1_STATE = 4,
    ADDR_2_STATE = 5,
    ADDR_3_STATE = 6,
    RECORD_TYPE_0_STATE = 7,
    RECORD_TYPE_1_STATE = 8,
    DATA_STATE = 9,
    CHECKSUM_0_STATE = 10,
    CHECKSUM_1_STATE = 11
};
enum record_type_e {
    DATA,
    END_OF_FILE ,
    EXTENDED_SEGMENT_ADDRESS ,
    START_SEGMENT_ADDRESS ,
    EXTENDED_LINEAR_ADDRESS,
    START_LINEAR_ADDRESS
};

#define INVALID_HEX_CHAR        'x'

namespace util {
namespace {
uint8_t hex2dec(uint8_t h) {
    if (h >= '0' && h <= '9')
        return h - '0';
    else if (h >= 'A' && h <= 'F')
        return h - 'A' + 0xA;
    else if (h >= 'a' && h <= 'z')
        return h - 'a' + 0xA;
    else
        return INVALID_HEX_CHAR;
}
}

bool ihex_parser::parse(std::istream &is, std::function<bool(uint64_t, uint64_t, const uint8_t*)> cb) {
    char c;
    uint8_t i;
    unsigned state{0};
    uint16_t address_lo{0};
    uint16_t address_hi{0};
    std::array<uint8_t,IHEX_DATA_SIZE> data;
    unsigned data_size_in_nibble{0};
    uint8_t byte_count{0};
    uint8_t record_type{DATA};
    uint8_t calculated_cs{0};         // calculate checksum
    uint8_t saved_high_nibble{0};
    bool save_high_nibble{true};
    bool ex_segment_addr_mode{false};
    while(!is.eof()) {
        is.get(c);
        if (c == '\0')
            return true;
        if (state){
            if((i = hex2dec(c)) == INVALID_HEX_CHAR)
                return false;
            if (state <= CHECKSUM_1_STATE) {
                if (save_high_nibble)
                    saved_high_nibble = i;
                else
                    calculated_cs += (saved_high_nibble << 4) | i;
                save_high_nibble = !save_high_nibble;
            }
        }
        switch (state) {
        case START_CODE_STATE:
            calculated_cs = 0x00;
            save_high_nibble = true;
            if (c == '\r' || c == '\n') {
                continue;
            } else if (c == ':') {
                byte_count = 0;
                record_type = DATA;
                address_lo = 0x0000;
                memset(data.data(), 0, data.size());
                data_size_in_nibble = 0;
                calculated_cs=0;
                ++state;
            } else
                return false;
            break;
        case BYTE_COUNT_0_STATE:
        case BYTE_COUNT_1_STATE:
            byte_count = (byte_count << 4) | i;
            ++state;
            break;
        case ADDR_0_STATE:
        case ADDR_1_STATE:
        case ADDR_2_STATE:
        case ADDR_3_STATE:
            address_lo = ((address_lo << 4) | i);   // only alter lower 16-bit address
            ++state;
            break;
        case RECORD_TYPE_0_STATE:
            if (i != 0)
                return false;
            ++state;
            break;
        case RECORD_TYPE_1_STATE:
            if (i > START_LINEAR_ADDRESS)
                return false;
            record_type = i;
            if (byte_count == 0) {
                state = CHECKSUM_0_STATE;
            } else if (byte_count > sizeof(data))
                return false;
            else
                ++state;
            break;
        case DATA_STATE: {
            uint8_t b_index = data_size_in_nibble/2;
            data[b_index] = (data[b_index] << 4) | i;

            ++data_size_in_nibble;
            if ((data_size_in_nibble/2) >= byte_count)
                ++state;
            break;
        }
        case CHECKSUM_0_STATE:
            ++state;
            break;
        case CHECKSUM_1_STATE:
            if ((byte_count << 1) != data_size_in_nibble)  // Check whether byte count field match the data size
                return false;
            if (calculated_cs != 0x00)
                return false;
            if (record_type == EXTENDED_SEGMENT_ADDRESS) {          // Set extended segment addresss
                address_hi = ((uint16_t) data[0] << 8) | (data[1]);
                ex_segment_addr_mode = true;
            } else if (record_type == EXTENDED_LINEAR_ADDRESS) {     // Set linear addresss
                address_hi = ((uint16_t) data[0] << 8) | (data[1]);
                ex_segment_addr_mode = false;
            } else if (record_type == DATA && cb) {
                uint64_t address = ex_segment_addr_mode?
                        (static_cast<uint32_t>(address_hi) << 4) + static_cast<uint32_t>(address_lo):
                        (static_cast<uint32_t>(address_hi) << 16) | static_cast<uint32_t>(address_lo);
                if (!cb(address, data_size_in_nibble >> 1, data.data()))
                    return false;
            }
            state = START_CODE_STATE;
            break;
        default:
            return false;
        }
    }
    return true;
}
}
