#pragma once

#include <QObject>
#include <memory>
#include <set>
#include <stm32/stm32.hpp>

#include "models/assembly_view_model.hpp"
#include "models/settings.hpp"

namespace app
{
class Application final : public QObject {
    Q_OBJECT

    struct ApplicationState {
        explicit ApplicationState(std::unique_ptr<std::vector<uint8_t>>&& data, stm32::Memory::Config config)
            : flash{std::move(data)}
            , cpu{config}
        {
        }

        std::unique_ptr<std::vector<uint8_t>> flash;
        stm32::Cpu cpu;
        std::set<uint32_t> breakpoints{};
        uint32_t nextInstructionAddress{};
        bool shouldPause = true;
    };

public:
    explicit Application(Settings& settings);

    void loadFile(const QString& path);
    void resetCpu();
    void pauseExecution();
    void executeNextInstruction();
    void executeUntilBreakpoint();

    void addBreakpoint(uint32_t address);
    void removeBreakpoint(uint32_t address);

signals:
    void stateChanged();
    void assemblyLoaded(const QString& assembly);
    void memoryLoaded(stm32::Memory& memory);
    void registersLoaded(stm32::rg::CpuRegistersSet& cpuRegisters);
    void instructionSelected(uint32_t address);

private:
    void initCpu(std::unique_ptr<std::vector<uint8_t>>&& flash);

    void step();

    void updateNextInstructionAddress();

    Settings& m_settings;

    std::optional<ApplicationState> m_state{};
};

}  // namespace app
