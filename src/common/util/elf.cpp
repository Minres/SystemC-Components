#include "elf.h"
#include <elfio/elfio.hpp>
#include <sstream>
#include <stdexcept>

namespace util {
uint64_t load_elf_file(std::string const& name, std::function<bool(uint64_t, uint64_t, const uint8_t*)> cb, uint8_t expected_elf_class,
                       uint16_t expected_elf_machine) {
    // Create elfio reader
    ELFIO::elfio reader;
    // Load ELF data
    if(!reader.load(name))
        throw std::runtime_error("Could not load file");
    // check elf properties
    if(reader.get_class() != expected_elf_class)
        throw std::runtime_error("ELF Class missmatch");
    if(reader.get_type() != ELFIO::ET_EXEC && reader.get_type() != ELFIO::ET_DYN)
        throw std::runtime_error("Input is neither an executable nor a pie executable (dyn)");
    if(reader.get_machine() != expected_elf_machine)
        throw std::runtime_error("ELF Machine type missmatch");
    auto entry_address = reader.get_entry();
    for(const auto& pseg : reader.segments) {
        const auto fsize = pseg->get_file_size(); // 0x42c/0x0
        const auto seg_data = pseg->get_data();
        const auto type = pseg->get_type();
        if(type == ELFIO::PT_LOAD && fsize > 0) {
            if(cb(pseg->get_physical_address(), fsize, reinterpret_cast<const uint8_t* const>(seg_data))) {
                std::ostringstream oss;
                oss << "Problem writing " << fsize << " bytes to 0x" << std::hex << pseg->get_physical_address();
                throw std::runtime_error(oss.str());
            }
        }
    }
    return entry_address;
};

std::unordered_map<std::string, uint64_t> read_elf_symbols(std::string const& name, uint8_t expected_elf_class,
                                                           uint16_t expected_elf_machine) {
    // Create elfio reader
    ELFIO::elfio reader;
    // Load ELF data
    if(!reader.load(name))
        throw std::runtime_error("Could not load file");
    // check elf properties
    if(reader.get_class() != expected_elf_class)
        throw std::runtime_error("ELF Class missmatch");
    if(reader.get_type() != ELFIO::ET_EXEC && reader.get_type() != ELFIO::ET_DYN)
        throw std::runtime_error("Input is neither an executable nor a pie executable (dyn)");
    if(reader.get_machine() != expected_elf_machine)
        throw std::runtime_error("ELF Machine type missmatch");
    std::unordered_map<std::string, uint64_t> symbol_table;
    const auto sym_sec = reader.sections[".symtab"];
    if(ELFIO::SHT_SYMTAB == sym_sec->get_type() || ELFIO::SHT_DYNSYM == sym_sec->get_type()) {
        ELFIO::symbol_section_accessor symbols(reader, sym_sec);
        auto sym_no = symbols.get_symbols_num();
        std::string name;
        ELFIO::Elf64_Addr value = 0;
        ELFIO::Elf_Xword size = 0;
        unsigned char bind = 0;
        unsigned char type = 0;
        ELFIO::Elf_Half section = 0;
        unsigned char other = 0;
        for(auto i = 0U; i < sym_no; ++i) {
            symbols.get_symbol(i, name, value, size, bind, type, section, other);
            if(name != "") {
                symbol_table[name] = value;
            }
        }
    }
    return std::move(symbol_table);
};

} // namespace util
