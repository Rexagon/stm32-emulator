// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "memory_view.hpp"

#include <QVBoxLayout>

namespace app
{
MemoryView::MemoryView(QWidget* parent)
    : QWidget{parent}
{
    init();
}

void MemoryView::init()
{
    auto* layout = new QVBoxLayout{this};

    m_textView = new QTextEdit{this};
    layout->addWidget(m_textView);

    m_textView->setReadOnly(true);
}

}  // namespace app
