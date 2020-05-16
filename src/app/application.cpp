// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "application.hpp"

#include <QMessageBox>
#include <QProcess>

namespace app
{
Application::Application(Settings& settings)
    : m_settings{settings}
    , m_assemblyViewModel{}
{
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
    objcopy.start(m_settings.objcopyDirectory(), QStringList{"-O", "binary", "--only-section=.text", path, "-"});
    objcopy.waitForFinished();
    if (objcopy.exitCode() != 0) {
        QMessageBox::critical(nullptr, tr("Failed to objcopy file"), "");
        return;
    }

    initCpu(objcopy.readAllStandardOutput());
}

void Application::initCpu(QByteArray flash)
{
    m_state.emplace(ApplicationState{.flash = flash,
                                     .cpu = stm32::Cpu{stm32::Memory::Config{
                                         .flashMemoryStart = 0x08000000u,
                                         .flashMemoryEnd = 0x0801ffffu,

                                         .systemMemoryStart = 0x1ffff000u,
                                         .systemMemoryEnd = 0x1ffff800u,

                                         .optionBytesStart = 0x1ffff800u,
                                         .optionBytesEnd = 0x1ffff80Fu,

                                         .sramStart = 0x20000000u,
                                         .sramEnd = 0x20001fffu,

                                         .bootMode = stm32::BootMode::FlashMemory,
                                         .flash = stm32::utils::ArrayView<uint8_t, uint32_t>{reinterpret_cast<uint8_t*>(flash.data()),
                                                                                             static_cast<uint32_t>(flash.size())},
                                     }}});

    auto& SRAM = m_state->cpu.memory().SRAM();

    emit memoryLoaded(QByteArray::fromRawData(reinterpret_cast<char*>(SRAM.data()), static_cast<int>(SRAM.size())));
}

}  // namespace app
