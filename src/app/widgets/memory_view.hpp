#pragma once

#include <QTextEdit>

#include "../utils/general.hpp"

namespace app
{
class MemoryView final : public QAbstractScrollArea {
    Q_OBJECT
public:
    explicit MemoryView(QWidget* parent);
    ~MemoryView() override = default;

    void setData(QByteArray newData);
    void clearData();

    void updateViewport();

public:
    RESTRICT_COPY(MemoryView);

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    void init();

    auto fullSize() const -> QSize;
    void resetSelection(int position);
    void setSelection(int position);
    void ensureVisible();
    void setCursorPosition(int position);
    auto cursorPosition(const QPoint& position) -> int;

    std::optional<QByteArray> m_data{};

    int m_bytesPerLine = 16;
    int m_addressByteCount = 10;

    int m_positionAddress = 0;
    int m_positionHex = 0;
    int m_positionAscii = 0;
    int m_characterWidth = 0;
    int m_characterHeight = 0;

    int m_selectionBegin = 0;
    int m_selectionEnd = 0;
    int m_selectionInit = 0;
    int m_cursorPosition = 0;
};

}  // namespace app
