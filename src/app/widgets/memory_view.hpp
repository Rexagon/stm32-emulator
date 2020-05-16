#pragma once

#include <QTextEdit>

#include "../utils/general.hpp"

namespace app
{
class MemoryView final : public QWidget {
    Q_OBJECT
public:
    explicit MemoryView(QWidget* parent);
    ~MemoryView() override = default;

public:
    RESTRICT_COPY(MemoryView);

private:
    void init();

    QTextEdit* m_textView = nullptr;
};

}  // namespace app
