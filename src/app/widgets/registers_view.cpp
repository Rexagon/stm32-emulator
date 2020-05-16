// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "registers_view.hpp"

#include <QGridLayout>

namespace app
{
RegistersView::RegistersView(QWidget* parent)
    : QWidget{parent}
{
    init();
}

void RegistersView::init()
{
    auto* layout = new QGridLayout{this};

    auto* label = new QLabel{"Registers view", this};
    layout->addWidget(label);
}

}  // namespace app
