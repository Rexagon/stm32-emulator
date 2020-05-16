// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "assembly_view.hpp"

#include <QVBoxLayout>

namespace app
{
AssemblyView::AssemblyView(QWidget* parent)
    : QWidget{parent}
{
    init();
}

void AssemblyView::init()
{
    setMinimumWidth(200);
    auto* layout = new QVBoxLayout{this};

    m_textView = new QTextEdit{this};
    layout->addWidget(m_textView);

    m_textView->setReadOnly(true);
}

void AssemblyView::updateView(const QString& assembly)
{
    m_textView->setText(assembly);
}

}  // namespace app
