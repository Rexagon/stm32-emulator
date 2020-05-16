// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "custom_splitter.hpp"

#include <QPaintEvent>
#include <QPainter>

namespace app
{
CustomSplitter::CustomSplitter(Qt::Orientation orientation, QWidget* parent)
    : QSplitter{orientation, parent}
{
    setHandleWidth(1);
}

QSplitterHandle* CustomSplitter::createHandle()
{
    return new CustomSplitterHandle{orientation(), this};
}

CustomSplitterHandle::CustomSplitterHandle(Qt::Orientation orientation, QSplitter* parent)
    : QSplitterHandle{orientation, parent}
{
}

void CustomSplitterHandle::paintEvent(QPaintEvent* event)
{
    QPainter painter{this};
    painter.fillRect(event->rect(), Qt::gray);
}

}  // namespace app
