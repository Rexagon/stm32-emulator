#pragma once

#include <QLabel>

namespace app
{
class RegistersView final : public QWidget {
    Q_OBJECT
public:
    explicit RegistersView(QWidget* parent);
    ~RegistersView() override = default;

private:
    void init();
};

}  // namespace app
