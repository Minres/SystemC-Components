#ifndef UTIL_ELF_IO_H_
#define UTIL_ELF_IO_H_
#include <cstdint>
#include <elfio/elf_types.hpp>
#include <functional>
#include <string>

namespace util {
uint64_t load_elf_file(std::string const& name, std::function<bool(uint64_t, uint64_t, const uint8_t*)>,
                       uint8_t expected_elf_class = ELFIO::ELFCLASS32, uint16_t expected_elf_machine = ELFIO::EM_RISCV);
std::unordered_map<std::string, uint64_t> read_elf_symbols(std::string const& name, uint8_t expected_elf_class = ELFIO::ELFCLASS32,
                                                           uint16_t expected_elf_machine = ELFIO::EM_RISCV);
} // namespace util
#endif