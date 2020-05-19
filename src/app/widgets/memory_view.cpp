// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "memory_view.hpp"

#include <QGridLayout>

namespace app
{
MemoryView::MemoryView(QWidget* parent)
    : QWidget{parent}
{
    init();
}

void MemoryView::setMemory(stm32::Memory& memory)
{
    m_flashHexView->setData(
        0u,
        QByteArray::fromRawData(reinterpret_cast<char*>(memory.config().flash.begin()), static_cast<int>(memory.config().flash.size())));
    m_sramHexView->setData(memory.config().sramStart,
                           QByteArray::fromRawData(reinterpret_cast<char*>(memory.SRAM().data()), static_cast<int>(memory.SRAM().size())));

    m_memory.emplace(memory);
}

void MemoryView::updateContents()
{
    if (!m_memory.has_value()) {
        return;
    }

    m_sramHexView->updateViewport();
}

void MemoryView::reset()
{
    m_sramHexView->reset();

    m_memory.reset();
}

void MemoryView::init()
{
    auto* layout = new QHBoxLayout{this};

    m_flashHexView = new HexView{this};
    layout->addWidget(m_flashHexView);

    m_sramHexView = new HexView{this};
    layout->addWidget(m_sramHexView);
}

}  // namespace app
