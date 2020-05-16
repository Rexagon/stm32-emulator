// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "assembly_view.hpp"

#include <QFontDatabase>
#include <QHeaderView>
#include <QVBoxLayout>

#include "../models/assembly_view_model.hpp"

namespace app
{
AssemblyView::AssemblyView(QWidget* parent)
    : QTableView{parent}
{
    init();
}

void AssemblyView::init()
{
    setMinimumWidth(200);
    horizontalHeader()->setStretchLastSection(true);
    verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
    verticalHeader()->setVisible(false);

    QFont font{QFontDatabase::systemFont(QFontDatabase::SystemFont::FixedFont)};
}

void AssemblyView::setModel(QAbstractItemModel* model)
{
    QTableView::setModel(model);

    connect(model, &QAbstractItemModel::dataChanged, this, &AssemblyView::updateRows);
}

void AssemblyView::updateRows()
{
    auto* model = this->model();

    for (int i = 0; i < model->rowCount(); ++i) {
        const auto rowType = model->data(model->index(i, 0), Qt::UserRole);
        if (rowType.value<AssemblyViewModel::RowType>() == AssemblyViewModel::RowType::Label) {
            setSpan(i, 0, 1, 3);
        }
    }
}

}  // namespace app
