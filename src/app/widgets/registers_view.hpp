#pragma once

#include <QLineEdit>
#include <stm32/registers/cpu_registers_set.hpp>

#include "../utils/general.hpp"

namespace app
{
class RegistersView final : public QWidget {
    Q_OBJECT
public:
    explicit RegistersView(QWidget* parent);
    ~RegistersView() override = default;

    void setRegisters(stm32::rg::CpuRegistersSet& cpuRegisters);
    void updateContents();
    void reset();

public:
    RESTRICT_COPY(RegistersView);

private:
    void init();

    std::optional<std::reference_wrapper<stm32::rg::CpuRegistersSet>> m_cpuRegisters{};

    std::array<QLineEdit*, 13> m_generalPurposeRegisterFields{};
    std::array<QLineEdit*, 2> m_stackPointerFields{};
    QLineEdit* m_linkRegisterField = nullptr;
    QLineEdit* m_programCounterRegisterField = nullptr;
};

}  // namespace app
