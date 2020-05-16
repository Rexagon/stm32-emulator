#pragma once

#include <QAbstractTableModel>

namespace app
{
class AssemblyViewModel final : public QAbstractTableModel {
    Q_OBJECT

public:
    enum class RowType {
        Label,
        Code,
    };

    struct Row {
        RowType type;
        uint32_t address{};
        QString raw{};
        QString assembly;
    };

    explicit AssemblyViewModel();
    ~AssemblyViewModel() override = default;

    void tryFillFromString(const QString& input);
    void clear();

    int rowCount(const QModelIndex& parent = QModelIndex{}) const override;
    int columnCount(const QModelIndex& parent = QModelIndex{}) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

private:
    std::vector<Row> m_rows;
};

}  // namespace app

Q_DECLARE_METATYPE(app::AssemblyViewModel::RowType)
