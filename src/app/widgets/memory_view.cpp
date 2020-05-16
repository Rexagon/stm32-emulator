// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "memory_view.hpp"

#include <QApplication>
#include <QClipboard>
#include <QFontDatabase>
#include <QPaintEvent>
#include <QPainter>
#include <QScrollBar>
#include <QVBoxLayout>

namespace app
{
namespace
{
constexpr auto ADDRESS_LEFT_OFFSET = 5;
constexpr auto HEX_LEFT_OFFSET = 10;
constexpr auto ASCII_LEFT_OFFSET = 16;
}  // namespace

MemoryView::MemoryView(QWidget* parent)
    : QAbstractScrollArea{parent}
{
    init();
}

void MemoryView::init()
{
    setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    m_characterWidth = fontMetrics().maxWidth();
    m_characterHeight = fontMetrics().height();

    m_positionAddress = ADDRESS_LEFT_OFFSET;
    m_positionHex = m_positionAddress + m_addressByteCount * m_characterWidth + HEX_LEFT_OFFSET;
    m_positionAscii = m_positionHex + (m_bytesPerLine * 3 - 1) * m_characterWidth + ASCII_LEFT_OFFSET;

    setMinimumWidth(m_positionAscii + (m_bytesPerLine * m_characterWidth));
}

void MemoryView::setData(QByteArray newData)
{
    verticalScrollBar()->setValue(0);

    m_data = newData;
    m_cursorPosition = 0;
    resetSelection(0);

    viewport()->update();
}

void MemoryView::clearData()
{
    m_data.reset();
}

void MemoryView::updateViewport()
{
    viewport()->update();
}

void MemoryView::paintEvent(QPaintEvent* event)
{
    if (!m_data.has_value()) {
        return;
    }

    const auto dataSize = m_data->size();

    QPainter painter{viewport()};

    auto areaSize = viewport()->size();
    auto widgetSize = fullSize();
    verticalScrollBar()->setPageStep(areaSize.height() / m_characterHeight);
    verticalScrollBar()->setRange(0, (widgetSize.height() - areaSize.height()) / m_characterHeight + 1);

    auto firstLineNumber = verticalScrollBar()->value();

    int lastLineNumber = firstLineNumber + areaSize.height() / m_characterHeight;
    if (lastLineNumber > dataSize / m_bytesPerLine) {
        lastLineNumber = (dataSize + m_bytesPerLine - 1) / m_bytesPerLine;
    }

    painter.fillRect(event->rect(), palette().color(QPalette::Base));

    auto addressAreaColor = QColor{0xdd, 0xdd, 0xdd, 0xff};
    painter.fillRect(QRect{0, event->rect().top(), m_positionHex - HEX_LEFT_OFFSET / 2, height()}, addressAreaColor);

    auto linePos = m_positionAscii - (ASCII_LEFT_OFFSET / 2);
    painter.setPen(Qt::gray);
    painter.drawLine(linePos, event->rect().top(), linePos, height());

    painter.setPen(Qt::black);

    auto yPosStart = m_characterHeight;

    QBrush def = painter.brush();
    QBrush selected{QColor{0x6d, 0x9e, 0xff, 0xff}};
    auto dataView = m_data->mid(firstLineNumber * m_bytesPerLine, (lastLineNumber - firstLineNumber) * m_bytesPerLine);

    for (int lineIndex = firstLineNumber, yPos = yPosStart; lineIndex < lastLineNumber; lineIndex += 1, yPos += m_characterHeight) {
        QString address = QString("%1").arg(lineIndex * 16, 10, 16, QChar('0'));
        painter.drawText(m_positionAddress, yPos, address);

        for (int xPos = m_positionHex, i = 0; i < m_bytesPerLine && ((lineIndex - firstLineNumber) * m_bytesPerLine + i) < dataView.size();
             i++, xPos += 3 * m_characterWidth) {
            auto pos = (lineIndex * m_bytesPerLine + i) * 2;
            if (pos >= m_selectionBegin && pos < m_selectionEnd) {
                painter.setBackground(selected);
                painter.setBackgroundMode(Qt::OpaqueMode);
            }

            QString val = QString::number((dataView[(lineIndex - firstLineNumber) * m_bytesPerLine + i] & 0xF0) >> 4, 16);
            painter.drawText(xPos, yPos, val);

            if ((pos + 1) >= m_selectionBegin && (pos + 1) < m_selectionEnd) {
                painter.setBackground(selected);
                painter.setBackgroundMode(Qt::OpaqueMode);
            }
            else {
                painter.setBackground(def);
                painter.setBackgroundMode(Qt::OpaqueMode);
            }

            val = QString::number((dataView[(lineIndex - firstLineNumber) * m_bytesPerLine + i] & 0xF), 16);
            painter.drawText(xPos + m_characterWidth, yPos, val);

            painter.setBackground(def);
            painter.setBackgroundMode(Qt::OpaqueMode);
        }

        for (int xPosAscii = m_positionAscii, i = 0;
             ((lineIndex - firstLineNumber) * m_bytesPerLine + i) < dataView.size() && (i < m_bytesPerLine);
             i++, xPosAscii += m_characterWidth) {
            auto ch = static_cast<char>(dataView[(lineIndex - firstLineNumber) * m_bytesPerLine + i]);
            if ((ch < 0x20) || (ch > 0x7e)) {
                ch = '.';
            }

            painter.drawText(xPosAscii, yPos, QString(ch));
        }
    }

    if (hasFocus()) {
        int x = (m_cursorPosition % (2 * m_bytesPerLine));
        int y = m_cursorPosition / (2 * m_bytesPerLine);
        y -= firstLineNumber;
        int cursorX = (((x / 2) * 3) + (x % 2)) * m_characterWidth + m_positionHex;
        int cursorY = y * m_characterHeight + 4;
        painter.fillRect(cursorX, cursorY, 2, m_characterHeight, palette().color(QPalette::WindowText));
    }
}

void MemoryView::keyPressEvent(QKeyEvent* event)
{
    auto setVisible = false;

    // Cursor movements
    if (event->matches(QKeySequence::MoveToNextChar)) {
        setCursorPosition(m_cursorPosition + 1);
        resetSelection(m_cursorPosition);
        setVisible = true;
    }
    if (event->matches(QKeySequence::MoveToPreviousChar)) {
        setCursorPosition(m_cursorPosition - 1);
        resetSelection(m_cursorPosition);
        setVisible = true;
    }
    if (event->matches(QKeySequence::MoveToEndOfLine)) {
        setCursorPosition(m_cursorPosition | ((m_bytesPerLine * 2) - 1));
        resetSelection(m_cursorPosition);
        setVisible = true;
    }
    if (event->matches(QKeySequence::MoveToStartOfLine)) {
        setCursorPosition(m_cursorPosition | (m_cursorPosition % (m_bytesPerLine * 2)));
        resetSelection(m_cursorPosition);
        setVisible = true;
    }
    if (event->matches(QKeySequence::MoveToPreviousLine)) {
        setCursorPosition(m_cursorPosition - m_bytesPerLine * 2);
        resetSelection(m_cursorPosition);
        setVisible = true;
    }
    if (event->matches(QKeySequence::MoveToNextLine)) {
        setCursorPosition(m_cursorPosition + m_bytesPerLine * 2);
        resetSelection(m_cursorPosition);
        setVisible = true;
    }

    if (event->matches(QKeySequence::MoveToNextPage)) {
        setCursorPosition(m_cursorPosition + (viewport()->height() / m_characterHeight - 1) * 2 * m_bytesPerLine);
        resetSelection(m_cursorPosition);
        setVisible = true;
    }
    if (event->matches(QKeySequence::MoveToPreviousPage)) {
        setCursorPosition(m_cursorPosition - (viewport()->height() / m_characterHeight - 1) * 2 * m_bytesPerLine);
        resetSelection(m_cursorPosition);
        setVisible = true;
    }
    if (event->matches(QKeySequence::MoveToEndOfDocument)) {
        if (m_data.has_value()) {
            setCursorPosition(m_data->size() * 2);
        }
        resetSelection(m_cursorPosition);
        setVisible = true;
    }
    if (event->matches(QKeySequence::MoveToStartOfDocument)) {
        setCursorPosition(0);
        resetSelection(m_cursorPosition);
        setVisible = true;
    }

    // Select commands
    if (event->matches(QKeySequence::SelectAll)) {
        resetSelection(0);
        if (m_data.has_value()) {
            setSelection(m_data->size() * 2 + 1);
        }
        setVisible = true;
    }
    if (event->matches(QKeySequence::SelectNextChar)) {
        int pos = m_cursorPosition + 1;
        setCursorPosition(pos);
        setSelection(pos);
        setVisible = true;
    }
    if (event->matches(QKeySequence::SelectPreviousChar)) {
        int pos = m_cursorPosition - 1;
        setSelection(pos);
        setCursorPosition(pos);
        setVisible = true;
    }
    if (event->matches(QKeySequence::SelectEndOfLine)) {
        int pos = m_cursorPosition - (m_cursorPosition % (2 * m_bytesPerLine)) + (2 * m_bytesPerLine);
        setCursorPosition(pos);
        setSelection(pos);
        setVisible = true;
    }
    if (event->matches(QKeySequence::SelectStartOfLine)) {
        int pos = m_cursorPosition - (m_cursorPosition % (2 * m_bytesPerLine));
        setCursorPosition(pos);
        setSelection(pos);
        setVisible = true;
    }
    if (event->matches(QKeySequence::SelectPreviousLine)) {
        int pos = m_cursorPosition - (2 * m_bytesPerLine);
        setCursorPosition(pos);
        setSelection(pos);
        setVisible = true;
    }
    if (event->matches(QKeySequence::SelectNextLine)) {
        int pos = m_cursorPosition + (2 * m_bytesPerLine);
        setCursorPosition(pos);
        setSelection(pos);
        setVisible = true;
    }

    if (event->matches(QKeySequence::SelectNextPage)) {
        int pos = m_cursorPosition + (((viewport()->height() / m_characterHeight) - 1) * 2 * m_bytesPerLine);
        setCursorPosition(pos);
        setSelection(pos);
        setVisible = true;
    }
    if (event->matches(QKeySequence::SelectPreviousPage)) {
        int pos = m_cursorPosition - (((viewport()->height() / m_characterHeight) - 1) * 2 * m_bytesPerLine);
        setCursorPosition(pos);
        setSelection(pos);
        setVisible = true;
    }
    if (event->matches(QKeySequence::SelectEndOfDocument)) {
        int pos = 0;
        if (m_data.has_value()) {
            pos = m_data->size() * 2;
        }
        setCursorPosition(pos);
        setSelection(pos);
        setVisible = true;
    }
    if (event->matches(QKeySequence::SelectStartOfDocument)) {
        int pos = 0;
        setCursorPosition(pos);
        setSelection(pos);
        setVisible = true;
    }

    if (event->matches(QKeySequence::Copy)) {
        if (m_data.has_value()) {
            QString res;
            int idx = 0;
            int copyOffset = 0;

            auto selectedData = m_data->mid(m_selectionBegin / 2, (m_selectionEnd - m_selectionBegin) / 2 + 1);
            if (m_selectionBegin % 2) {
                res += QString::number((selectedData[(idx + 1) / 2] & 0xF), 16);
                res += " ";
                idx++;
                copyOffset = 1;
            }

            int selectedSize = m_selectionEnd - m_selectionBegin;
            for (; idx < selectedSize; idx += 2) {
                QString val = QString::number((selectedData[(copyOffset + idx) / 2] & 0xF0) >> 4, 16);
                if (idx + 1 < selectedSize) {
                    val += QString::number((selectedData[(copyOffset + idx) / 2] & 0xF), 16);
                    val += " ";
                }
                res += val;

                if ((idx / 2) % m_bytesPerLine == (m_bytesPerLine - 1))
                    res += "\n";
            }
            QClipboard* clipboard = QApplication::clipboard();
            clipboard->setText(res);
        }
    }

    if (setVisible) {
        ensureVisible();
    }
    viewport()->update();
}

void MemoryView::mouseMoveEvent(QMouseEvent* event)
{
    auto actualPosition = cursorPosition(event->pos());
    setCursorPosition(actualPosition);
    setSelection(actualPosition);

    viewport()->update();
}

void MemoryView::mousePressEvent(QMouseEvent* event)
{
    auto position = cursorPosition(event->pos());

    if ((QApplication::keyboardModifiers() & Qt::ShiftModifier) && event->button() == Qt::LeftButton) {
        setSelection(position);
    }
    else {
        resetSelection(position);
    }

    setCursorPosition(position);

    viewport()->update();
}

auto MemoryView::fullSize() const -> QSize
{
    if (!m_data.has_value()) {
        return QSize{0, 0};
    }

    const auto width = m_positionAscii + (m_bytesPerLine * m_characterWidth);
    auto height = m_data->size() / m_bytesPerLine;
    if (m_data->size() % m_bytesPerLine) {
        height++;
    }
    height *= m_characterHeight;

    return QSize{width, height};
}

void MemoryView::resetSelection(int position)
{
    position = std::max(0, position);

    m_selectionInit = position;
    m_selectionBegin = position;
    m_selectionEnd = position;
}

void MemoryView::setSelection(int position)
{
    position = std::max(0, position);

    if (position >= m_selectionInit) {
        m_selectionEnd = position;
        m_selectionBegin = m_selectionInit;
    }
    else {
        m_selectionEnd = m_selectionInit;
        m_selectionBegin = position;
    }
}

void MemoryView::ensureVisible()
{
    const auto areaSize = viewport()->size();

    auto firstLineIndex = verticalScrollBar()->value();
    auto lastLineIndex = firstLineIndex + areaSize.height() / m_characterHeight;

    auto cursorY = m_cursorPosition / (2 * m_bytesPerLine);

    if (cursorY < firstLineIndex) {
        verticalScrollBar()->setValue(cursorY);
    }
    else if (cursorY >= lastLineIndex) {
        verticalScrollBar()->setValue(cursorY - areaSize.height() / m_characterHeight + 1);
    }
}

void MemoryView::setCursorPosition(int position)
{
    position = std::max(0, position);

    auto maxPosition = 0;
    if (m_data.has_value()) {
        maxPosition = m_data->size() * 2;
        if (m_data->size() % m_bytesPerLine) {
            maxPosition++;
        }
    }

    m_cursorPosition = std::min(maxPosition, position);
}

auto MemoryView::cursorPosition(const QPoint& position) -> int
{
    auto result = -1;

    if ((position.x() >= m_positionHex) && (position.x() < (m_positionHex + (m_bytesPerLine * 3 - 1) * m_characterWidth))) {
        int x = (position.x() - m_positionHex) / m_characterWidth;
        auto y = (position.y() / m_characterHeight * 2 * m_bytesPerLine);

        x = ((x / 3) * 2) + ((x % 3) > 0);

        result = x + y + verticalScrollBar()->value() * m_bytesPerLine * 2;
    }

    return result;
}


}  // namespace app
