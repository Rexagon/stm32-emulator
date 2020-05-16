// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "assembly_view_model.hpp"

#include <QBrush>
#include <QStringRef>
#include <QTextStream>

namespace app
{
namespace
{
constexpr auto LINES_TO_SKIP = 6;

constexpr QColor PALETTE[4][3] = {
    {QColor{0x99, 0x99, 0x99, 0xff}, QColor{0xdd, 0xdd, 0xdd, 0xff}, QColor{0xff, 0xff, 0xff, 0xff}},
    {QColor{0xa1, 0x74, 0x03, 0xff}, QColor{0xe0, 0xb1, 0x2f, 0xff}, QColor{0xff, 0xea, 0xd4, 0xff}},
    {QColor{0x59, 0x75, 0x42, 0xff}, QColor{0x6e, 0x9e, 0x56, 0xff}, QColor{0xe4, 0xff, 0xcf, 0xff}},
    {QColor{0x85, 0x34, 0x2d, 0xff}, QColor{0xd1, 0x50, 0x49, 0xff}, QColor{0xff, 0xbb, 0xb3, 0xff}},
};
}  // namespace

AssemblyViewModel::AssemblyViewModel()
    : m_rows{}
    , m_rowsMap{}
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

        Row* row;

        if (line.startsWith(' ')) {
            const auto address = line.midRef(0, 8).trimmed().toUInt(nullptr, 16);
            const auto raw = line.midRef(10, 30).trimmed();
            const auto assembly = line.midRef(40).trimmed();

            row = &m_rows.emplace_back(
                Row{.type = RowType::Code, .address = address, .raw = raw.toString(), .assembly = assembly.toString()});
        }
        else {
            const auto address = line.midRef(0, 8).toUInt(nullptr, 16);
            const auto assembly = line.midRef(9);

            row = &m_rows.emplace_back(Row{.type = RowType::Label, .address = address, .assembly = assembly.toString()});
        }

        m_rowsMap.insert({row->address, row});
    }

    emit dataChanged(index(0, 0), index(rowCount(), columnCount()));
    emit layoutChanged();
}

void AssemblyViewModel::clear()
{
    m_currentAddress = 0;
    m_rowsMap.clear();
    m_rows.clear();
}

auto AssemblyViewModel::selectCurrentAddress(uint32_t address) -> AssemblyViewModel::Row*
{
    m_currentAddress = address;
    auto it = m_rowsMap.find(address);
    if (it == m_rowsMap.end()) {
        return nullptr;
    }
    return it->second;
}

int AssemblyViewModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(m_rows.size());
}

int AssemblyViewModel::columnCount(const QModelIndex&) const
{
    return 4;
}

QVariant AssemblyViewModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return false;
    }

    const auto row = static_cast<size_t>(index.row());
    const auto column = index.column();

    const auto& item = m_rows[row];

    switch (role) {
        case Qt::DisplayRole: {
            switch (item.type) {
                case RowType::Label:
                    switch (column) {
                        case 2:
                            return item.assembly;
                    }
                    break;
                case RowType::Code:
                    switch (column) {
                        case 1:
                            return QString{"%1"}.arg(item.address, 8, 16, QChar{'0'});
                        case 2:
                            return item.raw;
                        case 3:
                            return item.assembly;
                    }
                    break;
            }
            break;
        }
        case Qt::UserRole:
            return QVariant::fromValue(item.type);
        case Qt::CheckStateRole:
            if (column == 0 && item.type == RowType::Code && item.withBreakpoint) {
                return Qt::Checked;
            }
            break;
        case Qt::TextAlignmentRole:
            switch (item.type) {
                case RowType::Label:
                    return Qt::AlignmentFlag::AlignBottom;
                case RowType::Code:
                    if (column < 2) {
                        return Qt::AlignmentFlag::AlignCenter;
                    }
                    break;
            }
            break;
        case Qt::BackgroundRole: {
            auto color = 0;
            if (item.address == m_currentAddress) {
                if (item.withBreakpoint) {
                    color = 3;
                }
                else {
                    color = 2;
                }
            }
            if (item.withBreakpoint) {
                color = 1;
            }
            return PALETTE[color][std::min(2, column)];
        }
    }

    return QVariant{};
}

bool AssemblyViewModel::setData(const QModelIndex& index, const QVariant&, int role)
{
    if (!index.isValid()) {
        return false;
    }

    const auto row = static_cast<size_t>(index.row());

    auto& item = m_rows[row];

    switch (role) {
        case Qt::CheckStateRole:
            if (item.type == RowType::Code) {
                item.withBreakpoint = !item.withBreakpoint;
                emit dataChanged(index, index);
                if (item.withBreakpoint) {
                    emit breakpointAdded(item.address);
                }
                else {
                    emit breakpointRemoved(item.address);
                }
                return true;
            }
            break;
    }

    return false;
}

}  // namespace app
