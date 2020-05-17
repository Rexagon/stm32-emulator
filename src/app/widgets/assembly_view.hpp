#pragma once

#include <QStyledItemDelegate>
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

    void setModel(QAbstractItemModel* model) override;

    void scrollToAddress(uint32_t address);

public:
    RESTRICT_COPY(AssemblyView);

private:
    void init();
    void registerEvents();

    void updateRows();
};

class SelectionControlDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit SelectionControlDelegate(QObject* parent = nullptr)
        : QStyledItemDelegate{parent}
    {
    }

    ~SelectionControlDelegate() override = default;

    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override
    {
        QStyledItemDelegate::initStyleOption(option, index);
        const bool selected = option->state & QStyle::State_Selected;
        option->font.setBold(selected);
        if (selected) {
            option->state = option->state & ~QStyle::State_Selected & ~QStyle::State_HasFocus;
        }
    }
};

}  // namespace app
