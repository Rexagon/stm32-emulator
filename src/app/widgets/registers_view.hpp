#pragma once

#include <QCheckBox>
#include <QLineEdit>
#include <QMouseEvent>
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
    auto initCpuBaseRegisterFields() -> QWidget*;
    auto initCpuSpecialRegisterFields() -> QWidget*;

    std::optional<std::reference_wrapper<stm32::rg::CpuRegistersSet>> m_cpuRegisters{};

    std::array<QLineEdit*, 13> m_generalPurposeRegisterFields{};
    std::array<QLineEdit*, 2> m_stackPointerFields{};
    QLineEdit* m_linkRegisterField = nullptr;
    QLineEdit* m_programCounterRegisterField = nullptr;

    QCheckBox* m_apsrQCheckbox = nullptr;
    QCheckBox* m_apsrVCheckbox = nullptr;
    QCheckBox* m_apsrCCheckbox = nullptr;
    QCheckBox* m_apsrZCheckbox = nullptr;
    QCheckBox* m_apsrNCheckbox = nullptr;

    QLineEdit* m_ipsrField = nullptr;
    QLineEdit* m_basepriField = nullptr;

    QCheckBox* m_primaskCheckbox = nullptr;

    QCheckBox* m_faultmaskCheckbox = nullptr;

    QCheckBox* m_controlNPrivCheckbox = nullptr;
    QCheckBox* m_controlSpselCheckbox = nullptr;

    QCheckBox* m_epsrTCheckbox = nullptr;
    QCheckBox* m_isInItStateCheckbox = nullptr;
    QCheckBox* m_isLastInItStateCheckbox = nullptr;
};

class ReadOnlyCheckbox final : public QCheckBox {
    Q_OBJECT
public:
    explicit ReadOnlyCheckbox(const QString& text, QWidget* parent)
        : QCheckBox{text, parent}
    {
    }
    ~ReadOnlyCheckbox() override = default;

protected:
    void mousePressEvent(QMouseEvent* event) override { event->ignore(); }
};

}  // namespace app
