#pragma once

#include <QTableView>
#include <QTextEdit>

#include "../utils/general.hpp"

namespace app
{
class AssemblyView final : public QTableView {
    Q_OBJECT
public:
    explicit AssemblyView(QWidget* parent);
    ~AssemblyView() override = default;

    void setModel(QAbstractItemModel *model) override;

public:
    RESTRICT_COPY(AssemblyView);

private:
    void init();

    void updateRows();
};

}  // namespace app
