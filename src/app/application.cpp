// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "application.hpp"

#include <QMessageBox>
#include <QProcess>
#include <iostream>  // TODO: remove

namespace app
{
Application::Application(AssemblyViewModel& assemblyViewModel, Settings& settings)
    : m_assemblyViewModel{assemblyViewModel}
    , m_settings{settings}
{
    registerEvents();
}

void Application::loadFile(const QString& path)
{
    QProcess objdump{};
    objdump.start(m_settings.objdumpPath(), QStringList{"-disassemble", "--full-leading-addr", "--triple=thumb", "--demangle", path});
    objdump.waitForFinished();
    if (objdump.exitCode() != 0) {
        QMessageBox::critical(nullptr, tr("Failed to objdump file"), "");
        return;
    }

    m_assemblyViewModel.tryFillFromString(objdump.readAllStandardOutput());

    QProcess objcopy{};
    objcopy.start(m_settings.objcopyDirectory(),
                  QStringList{"-O", "binary", "--only-section=.vector_table", "--only-section=.text", "--only-section=.rodata", path, "-"});
    objcopy.waitForFinished();
    if (objcopy.exitCode() != 0) {
        QMessageBox::critical(nullptr, tr("Failed to objcopy file"), "");
        return;
    }

    auto objcopyOutput = objcopy.readAllStandardOutput();

    initCpu(std::make_unique<std::vector<uint8_t>>(objcopyOutput.begin(), objcopyOutput.end()));
}

void Application::stop()
{
    if (!m_state.has_value()) {
        return;
    }

    initCpu(std::move(m_state->flash));
}

void Application::executeNextInstruction()
{
    step();
}

void Application::executeUntilBreakpoint()
{
}

void Application::addBreakpoint(uint32_t address)
{
    if (!m_state.has_value()) {
        return;
    }

    m_state->breakpoints.insert(address);
}

void Application::removeBreakpoint(uint32_t address)
{
    if (!m_state.has_value()) {
        return;
    }

    m_state->breakpoints.erase(address);
}

void Application::registerEvents()
{
    connect(&m_assemblyViewModel, &AssemblyViewModel::breakpointAdded, this, &Application::addBreakpoint);
    connect(&m_assemblyViewModel, &AssemblyViewModel::breakpointRemoved, this, &Application::removeBreakpoint);
}

void Application::initCpu(std::unique_ptr<std::vector<uint8_t>>&& flash)
{
    emit stateChanged();

    auto flashView = stm32::utils::ArrayView<uint8_t, uint32_t>{flash->data(), static_cast<uint32_t>(flash->size())};

    printf("%ld\n", reinterpret_cast<int64_t>(flashView.begin()));

    m_state.emplace(std::move(flash),
                    stm32::Memory::Config{
                        .flashMemoryStart = 0x08000000u,
                        .flashMemoryEnd = 0x08020000u,

                        .systemMemoryStart = 0x1ffff000u,
                        .systemMemoryEnd = 0x1ffff800u,

                        .optionBytesStart = 0x1ffff800u,
                        .optionBytesEnd = 0x1ffff80Fu,

                        .sramStart = 0x20000000u,
                        .sramEnd = 0x20005000u,

                        .bootMode = stm32::BootMode::FlashMemory,
                        .flash = flashView,
                    });

    m_state->cpu.reset();

    emit memoryLoaded(m_state->cpu.memory());
    emit registersLoaded(m_state->cpu.registers());
}

void Application::step()
{
    if (!m_state.has_value()) {
        return;
    }

    try {
        printf("PC: %08x\t", m_state->cpu.registers().PC());
        printf("IT: %s\t", m_state->cpu.isInItBlock() ? "1" : "_");
        printf("L: %s\t", m_state->cpu.isLastInItBlock() ? "1" : "_");
        printf("\n");

        if (m_state->cpu.isInItBlock()) {
            m_state->cpu.advanceCondition();
        }

        const auto instructionAddress = m_state->cpu.registers().PC() & ~uint32_t{0b1};
        printf("------PC: %08x\n", instructionAddress);
        m_assemblyViewModel.setCurrentAddress(instructionAddress);
        emit instructionSelected(instructionAddress);

        m_state->cpu.step();
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
}

}  // namespace app
