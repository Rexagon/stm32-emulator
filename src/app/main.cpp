// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <fstream>
#include <iostream>
#include <optional>
#include <stm32/stm32.hpp>
#include <streambuf>
#include <vector>

auto loadFlash(const char* path) -> std::optional<std::vector<uint8_t>>
{
    std::ifstream file{path, std::ios::binary};
    if (!file.is_open()) {
        return std::nullopt;
    }

    std::vector<uint8_t> result;

    file.seekg(0, std::ios::end);
    result.reserve(static_cast<size_t>(file.tellg()));
    file.seekg(0, std::ios::beg);

    result.assign(std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{});

    return result;
}

int main(int argc, const char** argv)
{
    if (argc < 2) {
        return 1;
    }

    stm32::Cpu cpu{stm32::Memory::Config{
        .flashMemoryStart = 0x08000000u,
        .flashMemoryEnd = 0x0801ffffu,

        .systemMemoryStart = 0x1ffff000u,
        .systemMemoryEnd = 0x1ffff800u,

        .optionBytesStart = 0x1ffff800u,
        .optionBytesEnd = 0x1ffff80Fu,

        .sramStart = 0x20000000u,
        .sramEnd = 0x20001fffu,

        .bootMode = stm32::BootMode::FlashMemory,
    }};

    if (auto loaded = loadFlash(argv[1]); loaded.has_value()) {
        assert(cpu.memory().FLASH().size() >= loaded->size());
        cpu.memory().FLASH() = std::move(loaded.value());
    }
    else {
        std::cerr << "failed to open file '" << argv[1] << "'" << std::endl;
        return 1;
    }

    try {
        cpu.reset();

        while (true) {
            std::cout << "PC: " << cpu.registers().PC() << std::endl;
            std::cout << "IT: " << cpu.isInItBlock() << std::endl;
            std::cout << "ITlast: " << cpu.isLastInItBlock() << std::endl;

            cpu.step();

            std::cout << std::endl;
        }
    }
    catch (const stm32::utils::CpuException& e) {
        std::cerr << e.what() << std::endl;
    }
    catch (const stm32::utils::UnpredictableException& e) {
        std::cerr << e.what() << std::endl;
    }
    catch (...) {
        std::cerr << "unknown exception" << std::endl;
    }

    return 0;
}
