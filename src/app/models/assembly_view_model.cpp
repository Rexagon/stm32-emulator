// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "assembly_view_model.hpp"

#include <QStringRef>
#include <QTextStream>

namespace app
{
namespace
{
constexpr auto LINES_TO_SKIP = 6;
}  // namespace

AssemblyViewModel::AssemblyViewModel()
    : m_rows{}
{
}

void AssemblyViewModel::tryFillFromString(const QString& input)
{
    clear();

    const auto lines = input.split('\n').mid(LINES_TO_SKIP);

    m_rows.reserve(static_cast<size_t>(lines.count()));

    for (const auto& line : lines) {
        if (line.isEmpty()) {
            continue;
        }

        if (line.startsWith(' ')) {
            const auto address = line.midRef(0, 8).trimmed().toUInt(nullptr, 16);
            const auto raw = line.midRef(10, 30).trimmed();
            const auto assembly = line.midRef(40).trimmed();

            m_rows.emplace_back(Row{.type = RowType::Code, .address = address, .raw = raw.toString(), .assembly = assembly.toString()});
        }
        else {
            const auto address = line.midRef(0, 8).toUInt(nullptr, 16);
            const auto assembly = line.midRef(9);

            m_rows.emplace_back(Row{.type = RowType::Label, .address = address, .assembly = assembly.toString()});
        }
    }

    emit dataChanged(index(0, 0), index(rowCount(), columnCount()));
    emit layoutChanged();
}

void AssemblyViewModel::clear()
{
    m_rows.clear();
}

int AssemblyViewModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(m_rows.size());
}

int AssemblyViewModel::columnCount(const QModelIndex&) const
{
    return 3;
}

QVariant AssemblyViewModel::data(const QModelIndex& index, int role) const
{
    auto row = static_cast<size_t>(index.row());
    auto column = index.column();

    const auto& item = m_rows[row];

    switch (role) {
        case Qt::DisplayRole: {
            switch (item.type) {
                case RowType::Label:
                    switch (column) {
                        case 0:
                            return item.assembly;
                    }
                    break;
                case RowType::Code:
                    switch (column) {
                        case 0:
                            return item.address;
                        case 1:
                            return item.raw;
                        case 2:
                            return item.assembly;
                    }
                    break;
            }
            break;
        }
        case Qt::UserRole:
            return QVariant::fromValue(item.type);
        case Qt::CheckStateRole:
            if (item.type == RowType::Code && column == 0) {
                return Qt::Checked;
            }
            break;
    }

    return QVariant{};
}
}  // namespace app
