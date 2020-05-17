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
            reg->setText(QString{"%1"}.arg(m_cpuRegisters->get().getRegister(static_cast<uint8_t>(i)), 8, 16, QChar{'0'}));
        }
        else {
            reg->setText(UNINITIALIZED_REG);
        }
    }

    if (m_cpuRegisters.has_value()) {
        uint32_t stackPointers[] = {m_cpuRegisters->get().SP_main(), m_cpuRegisters->get().SP_process()};
        for (size_t i = 0; i < m_stackPointerFields.size(); ++i) {
            m_stackPointerFields[i]->setText(QString{"%1"}.arg(stackPointers[i], 8, 16, QChar{'0'}));
        }
    }
    else {
        for (size_t i = 0; i < m_stackPointerFields.size(); ++i) {
            m_stackPointerFields[i]->setText(UNINITIALIZED_REG);
        }
    }

    if (m_cpuRegisters.has_value()) {
        m_linkRegisterField->setText(QString{"%1"}.arg(m_cpuRegisters->get().LR(), 8, 16, QChar{'0'}));
        m_programCounterRegisterField->setText(QString{"%1"}.arg(m_cpuRegisters->get().PC(), 8, 16, QChar{'0'}));
    }
    else {
        m_linkRegisterField->setText(UNINITIALIZED_REG);
        m_programCounterRegisterField->setText(UNINITIALIZED_REG);
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

    auto* layout = new QHBoxLayout{this};
    layout->setContentsMargins(0, 0, 0, 0);

    auto* baseRegistersContainer = new QWidget{this};
    layout->addWidget(baseRegistersContainer);
    auto* baseRegistersContainerLayout = new QGridLayout{baseRegistersContainer};

    auto* scrollArea = new QScrollArea{this};
    layout->addWidget(scrollArea);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameStyle(QFrame::NoFrame);
    scrollArea->setWidget(baseRegistersContainer);

    int row = 0;
    for (size_t i = 0; i < m_generalPurposeRegisterFields.size(); ++i) {
        auto* label = new QLabel{GPR_LABELS[i], this};
        baseRegistersContainerLayout->addWidget(label, row, 0);

        m_generalPurposeRegisterFields[i] = new QLineEdit{this};
        auto* textEdit = m_generalPurposeRegisterFields[i];
        textEdit->setReadOnly(true);
        textEdit->setFont(font);
        baseRegistersContainerLayout->addWidget(textEdit, row++, 1);
    }

    for (size_t i = 0; i < m_stackPointerFields.size(); ++i) {
        auto* label = new QLabel{SPR_LABELS[i], this};
        baseRegistersContainerLayout->addWidget(label, row, 0);

        m_stackPointerFields[i] = new QLineEdit{this};
        auto* textEdit = m_stackPointerFields[i];
        textEdit->setReadOnly(true);
        textEdit->setFont(font);
        baseRegistersContainerLayout->addWidget(textEdit, row++, 1);
    }

    //
    {
        auto* label = new QLabel{"LR", this};
        baseRegistersContainerLayout->addWidget(label, row, 0);

        m_linkRegisterField = new QLineEdit{this};
        m_linkRegisterField->setReadOnly(true);
        m_linkRegisterField->setFont(font);
        baseRegistersContainerLayout->addWidget(m_linkRegisterField, row++, 1);
    }

    //
    {
        auto* label = new QLabel{"PC", this};
        baseRegistersContainerLayout->addWidget(label, row, 0);

        m_programCounterRegisterField = new QLineEdit{this};
        m_programCounterRegisterField->setReadOnly(true);
        m_programCounterRegisterField->setFont(font);
        baseRegistersContainerLayout->addWidget(m_programCounterRegisterField, row++, 1);
    }

    //
    baseRegistersContainerLayout->setRowStretch(row, 1);
}

}  // namespace app
