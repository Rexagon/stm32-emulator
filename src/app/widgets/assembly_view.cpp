// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "assembly_view.hpp"

#include <QFontDatabase>
#include <QHeaderView>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>

#include "../models/assembly_view_model.hpp"

namespace app
{
AssemblyView::AssemblyView(QWidget* parent)
    : QTableView{parent}
{
    init();
    registerEvents();
}

void AssemblyView::scrollToAddress(uint32_t address)
{
    auto* model = dynamic_cast<AssemblyViewModel*>(this->model());
    if (model == nullptr) {
        return;
    }

    const auto row = model->getRowByAddress(address).first;

    scrollTo(model->index(row, 0));

    viewport()->update();
}

void AssemblyView::init()
{
    QFont font{QFontDatabase::systemFont(QFontDatabase::SystemFont::FixedFont)};
    setFont(font);

    setMinimumWidth(200);
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
    horizontalHeader()->setVisible(false);
    verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
    verticalHeader()->setVisible(false);

    setSelectionBehavior(QAbstractItemView::SelectRows);
    setFrameStyle(QFrame::NoFrame);
    setShowGrid(false);

    setItemDelegate(new SelectionControlDelegate{this});
}

void AssemblyView::registerEvents()
{
    connect(this, &AssemblyView::doubleClicked, [this](const QModelIndex& index) {
        if (auto model = this->model(); model != nullptr) {
            model->setData(index, {}, Qt::CheckStateRole);
        }
    });
}

void AssemblyView::setModel(QAbstractItemModel* model)
{
    QTableView::setModel(model);

    setColumnWidth(0, 20);
    setColumnWidth(1, fontMetrics().maxWidth() * 10);

    updateRows();
    connect(model, &QAbstractItemModel::dataChanged, [this]() { QTimer::singleShot(0, this, &AssemblyView::updateRows); });
}

void AssemblyView::updateRows()
{
    auto* model = this->model();
    if (model == nullptr) {
        return;
    }

    const auto rowHeight = fontMetrics().height();

    for (int i = 0; i < model->rowCount(); ++i) {
        const auto rowType = model->data(model->index(i, 0), Qt::UserRole);
        if (rowType.value<AssemblyViewModel::RowType>() == AssemblyViewModel::RowType::Label) {
            setSpan(i, 2, 1, 2);
            setRowHeight(i, rowHeight * 2);
        }
        else {
            setRowHeight(i, rowHeight);
        }
    }

    viewport()->update();
}

}  // namespace app
