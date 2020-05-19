// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "registers_view.hpp"

#include <QFontDatabase>
#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>

namespace app
{
namespace
{
constexpr const char* GPR_LABELS[13] = {
    "R0",
    "R1",
    "R2",
    "R3",
    "R4",
    "R5",
    "R6",
    "R7",
    "R8",
    "R9",
    "R10",
    "R11",
    "R12",
};
constexpr const char* SPR_LABELS[2] = {
    "SP_main",
    "SP_process",
};
constexpr auto UNINITIALIZED_REG = "........";

template <typename T>
auto toHex(const T& value) -> QString
{
    return QString{"%1"}.arg(value, sizeof(T) * 2, 16, QChar{'0'});
}

template <typename T>
auto toBinary(const T& value) -> QString
{
    return QString{"%1"}.arg(value, sizeof(T) * 8, 2, QChar{'0'});
}
}  // namespace

RegistersView::RegistersView(QWidget* parent)
    : QWidget{parent}
{
    init();
    updateContents();
}

void RegistersView::setRegisters(stm32::rg::CpuRegistersSet& cpuRegisters)
{
    m_cpuRegisters = cpuRegisters;

    updateContents();
}

void RegistersView::updateContents()
{
    for (size_t i = 0; i < m_generalPurposeRegisterFields.size(); ++i) {
        auto* reg = m_generalPurposeRegisterFields[i];

        if (m_cpuRegisters.has_value()) {
            reg->setText(toHex(m_cpuRegisters->get().getRegister(static_cast<uint8_t>(i))));
        }
        else {
            reg->setText(UNINITIALIZED_REG);
        }
    }

    if (m_cpuRegisters.has_value()) {
        uint32_t stackPointers[] = {m_cpuRegisters->get().SP_main(), m_cpuRegisters->get().SP_process()};
        for (size_t i = 0; i < m_stackPointerFields.size(); ++i) {
            m_stackPointerFields[i]->setText(toHex(stackPointers[i]));
        }
    }
    else {
        for (size_t i = 0; i < m_stackPointerFields.size(); ++i) {
            m_stackPointerFields[i]->setText(UNINITIALIZED_REG);
        }
    }

    if (m_cpuRegisters.has_value()) {
        m_linkRegisterField->setText(toHex(m_cpuRegisters->get().LR()));
        m_programCounterRegisterField->setText(toHex(m_cpuRegisters->get().PC()));

        const auto& APSR = m_cpuRegisters->get().APSR();
        m_apsrNCheckbox->setChecked(APSR.N);
        m_apsrZCheckbox->setChecked(APSR.Z);
        m_apsrCCheckbox->setChecked(APSR.C);
        m_apsrVCheckbox->setChecked(APSR.V);
        m_apsrQCheckbox->setChecked(APSR.Q);

        m_ipsrField->setText(toHex(m_cpuRegisters->get().IPSR().exceptionNumber));
        m_basepriField->setText(toHex(m_cpuRegisters->get().BASEPRI().level));

        m_primaskCheckbox->setChecked(m_cpuRegisters->get().PRIMASK().PM);
        m_faultmaskCheckbox->setChecked(m_cpuRegisters->get().FAULTMASK().FM);
        m_controlNPrivCheckbox->setChecked(m_cpuRegisters->get().CONTROL().nPRIV);
        m_controlSpselCheckbox->setChecked(m_cpuRegisters->get().CONTROL().SPSEL);
        m_epsrTCheckbox->setChecked(m_cpuRegisters->get().EPSR().T);
        m_isInItStateCheckbox->setChecked(m_cpuRegisters->get().ITSTATE() & 0b1111u);
        m_isLastInItStateCheckbox->setChecked((m_cpuRegisters->get().ITSTATE() & 0b1111u) == 0b1000u);

        m_itstateField->setText(toBinary(m_cpuRegisters->get().ITSTATE()));
    }
    else {
        m_linkRegisterField->setText(UNINITIALIZED_REG);
        m_programCounterRegisterField->setText(UNINITIALIZED_REG);

        m_apsrNCheckbox->setChecked(false);
        m_apsrZCheckbox->setChecked(false);
        m_apsrCCheckbox->setChecked(false);
        m_apsrVCheckbox->setChecked(false);
        m_apsrQCheckbox->setChecked(false);

        m_ipsrField->setText(UNINITIALIZED_REG);
        m_basepriField->setText(UNINITIALIZED_REG);

        m_primaskCheckbox->setChecked(false);
        m_faultmaskCheckbox->setChecked(false);
        m_controlNPrivCheckbox->setChecked(false);
        m_controlSpselCheckbox->setChecked(false);
        m_epsrTCheckbox->setChecked(false);
        m_isInItStateCheckbox->setChecked(false);
        m_isLastInItStateCheckbox->setChecked(false);

        m_itstateField->setText(UNINITIALIZED_REG);
    }
}

void RegistersView::reset()
{
    m_cpuRegisters.reset();
    updateContents();
}

void RegistersView::init()
{
    auto font = QFontDatabase::systemFont(QFontDatabase::SystemFont::FixedFont);
    setFont(font);

    auto* layout = new QHBoxLayout{this};
    layout->setContentsMargins(0, 0, 0, 0);

    auto* sectionsContainer = new QWidget{this};
    layout->addWidget(sectionsContainer);
    auto* sectionsContainerLayout = new QHBoxLayout{sectionsContainer};
    sectionsContainerLayout->setContentsMargins(0, 0, 0, 0);

    auto* scrollArea = new QScrollArea{this};
    layout->addWidget(scrollArea);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameStyle(QFrame::NoFrame);
    scrollArea->setWidget(sectionsContainer);

    sectionsContainerLayout->addStretch(1);
    sectionsContainerLayout->addWidget(initCpuBaseRegisterFields());
    sectionsContainerLayout->addWidget(initCpuSpecialRegisterFields());
}

auto RegistersView::initCpuBaseRegisterFields() -> QWidget*
{
    auto* container = new QWidget{this};
    auto* layout = new QGridLayout{container};

    const auto characterWidth = fontMetrics().maxWidth();

    int row = 0;
    for (size_t i = 0; i < m_generalPurposeRegisterFields.size(); ++i) {
        layout->addWidget(new QLabel{GPR_LABELS[i], this}, row, 0);
        m_generalPurposeRegisterFields[i] = new QLineEdit{this};
        m_generalPurposeRegisterFields[i]->setReadOnly(true);
        m_generalPurposeRegisterFields[i]->setFixedWidth(characterWidth * 9);
        layout->addWidget(m_generalPurposeRegisterFields[i], row++, 1);
    }

    for (size_t i = 0; i < m_stackPointerFields.size(); ++i) {
        layout->addWidget(new QLabel{SPR_LABELS[i], this}, row, 0);
        m_stackPointerFields[i] = new QLineEdit{this};
        m_stackPointerFields[i]->setReadOnly(true);
        m_stackPointerFields[i]->setFixedWidth(characterWidth * 9);
        layout->addWidget(m_stackPointerFields[i], row++, 1);
    }

    //
    layout->addWidget(new QLabel{"LR", this}, row, 0);
    m_linkRegisterField = new QLineEdit{this};
    m_linkRegisterField->setReadOnly(true);
    m_linkRegisterField->setFixedWidth(characterWidth * 9);
    layout->addWidget(m_linkRegisterField, row++, 1);

    //
    layout->addWidget(new QLabel{"PC", this}, row, 0);
    m_programCounterRegisterField = new QLineEdit{this};
    m_programCounterRegisterField->setReadOnly(true);
    m_programCounterRegisterField->setFixedWidth(characterWidth * 9);
    layout->addWidget(m_programCounterRegisterField, row++, 1);

    //
    layout->setRowStretch(row, 1);

    return container;
}

auto RegistersView::initCpuSpecialRegisterFields() -> QWidget*
{
    auto* container = new QWidget{this};
    auto* layout = new QGridLayout{container};

    const auto characterWidth = fontMetrics().maxWidth();

    int row = 0;

    //
    layout->addWidget(new QLabel{"APSR", this}, row, 0, 1, 1);

    std::tuple<int, QCheckBox*&, const char*> apsrFlags[] = {
        {1, m_apsrNCheckbox, "N"},
        {2, m_apsrZCheckbox, "Z"},
        {3, m_apsrCCheckbox, "C"},
        {4, m_apsrVCheckbox, "V"},
        {5, m_apsrQCheckbox, "Q"},
    };

    for (const auto& [column, checkbox, text] : apsrFlags) {
        checkbox = new ReadOnlyCheckbox{text, this};
        layout->addWidget(checkbox, row, column);
    }
    ++row;

    //
    std::tuple<QLineEdit*&, const char*, int> fields[] = {
        {m_ipsrField, "IPSR", 16},
        {m_basepriField, "BASEPRI", 2},
    };

    for (const auto& [field, text, chars] : fields) {
        layout->addWidget(new QLabel{text, this}, row, 0);
        field = new QLineEdit{this};
        field->setReadOnly(true);
        field->setMaximumWidth(characterWidth * (chars + 1));
        layout->addWidget(field, row++, 1, 1, 5);
    }

    //
    layout->addWidget(new QLabel{"PRIMASK", this}, row, 0, 1, 1);
    m_primaskCheckbox = new ReadOnlyCheckbox{"PM", this};
    layout->addWidget(m_primaskCheckbox, row++, 1);

    //
    layout->addWidget(new QLabel{"FAULTMASK", this}, row, 0, 1, 1);
    m_faultmaskCheckbox = new ReadOnlyCheckbox{"FM", this};
    layout->addWidget(m_faultmaskCheckbox, row++, 1);

    //
    layout->addWidget(new QLabel{"CONTROL", this}, row, 0, 1, 1);
    m_controlNPrivCheckbox = new ReadOnlyCheckbox{"nPRIV", this};
    layout->addWidget(m_controlNPrivCheckbox, row, 1, 1, 3);
    m_controlSpselCheckbox = new ReadOnlyCheckbox{"SPSEL", this};
    layout->addWidget(m_controlSpselCheckbox, row++, 4, 1, 2);

    //
    layout->addWidget(new QLabel{"EPSR", this}, row, 0);
    m_epsrTCheckbox = new ReadOnlyCheckbox{"T", this};
    layout->addWidget(m_epsrTCheckbox, row, 1);
    m_isInItStateCheckbox = new ReadOnlyCheckbox{"IT", this};
    layout->addWidget(m_isInItStateCheckbox, row, 2, 1, 3);
    m_isLastInItStateCheckbox = new ReadOnlyCheckbox{"last", this};
    layout->addWidget(m_isLastInItStateCheckbox, row++, 4, 1, 2);

    //
    layout->addWidget(new QLabel{"ITSTATE", this}, row, 0);
    m_itstateField = new QLineEdit{this};
    m_itstateField->setReadOnly(true);
    m_itstateField->setMaximumWidth(characterWidth * 9);
    layout->addWidget(m_itstateField, row++, 1, 1, 5);

    //
    layout->setRowStretch(row, 1);

    return container;
}

}  // namespace app
