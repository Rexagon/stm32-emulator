#pragma once

#include <QTextEdit>

#include "../utils/general.hpp"

namespace app
{
class AssemblyView final : public QWidget {
    Q_OBJECT
public:
    explicit AssemblyView(QWidget* parent);
    ~AssemblyView() override = default;

public:
    RESTRICT_COPY(AssemblyView);

private:
    void init();

    QTextEdit* m_textView = nullptr;
};

}  // namespace app
