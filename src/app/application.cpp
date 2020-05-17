// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "application.hpp"

#include <QMessageBox>
#include <QProcess>
#include <QTimer>

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
    objdump.start(m_settings.objdumpPath(), QStringList{"-disassemble", "--full-leading-addr", "--triple=thumb", path});
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

void Application::resetCpu()
{
    if (!m_state.has_value()) {
        return;
    }

    m_assemblyViewModel.setCurrentAddress(0);
    m_state->cpu.reset();
    m_state->shouldPause = true;

    emit stateChanged();
    emit memoryLoaded(m_state->cpu.memory());
    emit registersLoaded(m_state->cpu.registers());
    updateNextInstructionAddress();
}

void Application::pauseExecution()
{
    if (m_state.has_value()) {
        m_state->shouldPause = true;
    }
}

void Application::executeNextInstruction()
{
    if (m_state.has_value()) {
        m_state->shouldPause = true;
    }
    step();
}

void Application::executeUntilBreakpoint()
{
    if (m_state.has_value()) {
        m_state->shouldPause = false;
    }
    step();
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

    resetCpu();
}

void Application::step()
{
    if (!m_state.has_value()) {
        return;
    }

    try {
        if (m_state->cpu.isInItBlock()) {
            m_state->cpu.advanceCondition();
        }

        m_state->cpu.step();
    }
    catch (const stm32::utils::CpuException& e) {
        printf("ERROR: %s\n", e.what());
    }
    catch (const stm32::utils::UnpredictableException& e) {
        printf("ERROR: %s\n", e.what());
    }
    catch (...) {
        printf("UNKNOWN ERROR\n");
    }

    updateNextInstructionAddress();

    if (!m_state->shouldPause && !m_state->breakpoints.contains(m_state->nextInstructionAddress)) {
        QTimer::singleShot(0, this, &Application::step);
    }
}

void Application::updateNextInstructionAddress()
{
    if (!m_state.has_value()) {
        return;
    }

    m_state->nextInstructionAddress = m_state->cpu.registers().PC() & ~uint32_t{0b1};
    m_assemblyViewModel.setCurrentAddress(m_state->nextInstructionAddress);
    emit instructionSelected(m_state->nextInstructionAddress);
}

}  // namespace app
