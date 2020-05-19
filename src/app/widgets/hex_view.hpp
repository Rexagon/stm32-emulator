#pragma once

#include <QTextEdit>

#include "../utils/general.hpp"

namespace app
{
class HexView final : public QAbstractScrollArea {
    Q_OBJECT
public:
    explicit HexView(QWidget* parent);
    ~HexView() override = default;

    void setData(uint32_t addressOffset, QByteArray newData);
    void reset();

    void updateViewport();

public:
    RESTRICT_COPY(HexView);

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

    uint32_t m_addressOffset = 0;
    std::optional<QByteArray> m_data{};

    int m_bytesPerLine = 16;
    int m_addressCharacterCount = 8;

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
