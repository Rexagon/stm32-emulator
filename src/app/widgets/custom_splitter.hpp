#pragma once

#include <QSplitter>
#include <QSplitterHandle>

namespace app
{
class CustomSplitter final : public QSplitter {
    Q_OBJECT
public:
    explicit CustomSplitter(Qt::Orientation orientation, QWidget* parent);
    ~CustomSplitter() override = default;

protected:
    QSplitterHandle* createHandle() override;
};

class CustomSplitterHandle final : public QSplitterHandle {
    Q_OBJECT
public:
    explicit CustomSplitterHandle(Qt::Orientation orientation, QSplitter* parent);

protected:
    void paintEvent(QPaintEvent* event) override;
};

}  // namespace app
