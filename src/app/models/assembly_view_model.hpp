#pragma once

#include <QAbstractTableModel>
#include <unordered_map>

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
        uint32_t address;
        QString raw{};
        QString assembly;
        bool withBreakpoint = false;
    };

    explicit AssemblyViewModel();
    ~AssemblyViewModel() override = default;

    void tryFillFromString(const QString& input);
    void clear();

    void setCurrentAddress(uint32_t address);
    auto getRowByAddress(uint32_t address) -> std::pair<int, Row*>;

    int rowCount(const QModelIndex& parent = QModelIndex{}) const override;
    int columnCount(const QModelIndex& parent = QModelIndex{}) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;

signals:
    void breakpointAdded(uint32_t address);
    void breakpointRemoved(uint32_t address);

private:
    uint32_t m_currentAddress = 0;

    std::vector<Row> m_rows;
    std::unordered_map<uint32_t, std::pair<int, Row*>> m_rowsMap;
};

}  // namespace app

Q_DECLARE_METATYPE(app::AssemblyViewModel::RowType)
