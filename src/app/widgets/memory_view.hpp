#pragma once

#include <stm32/memory.hpp>

#include "hex_view.hpp"

namespace app
{
class MemoryView final : public QWidget {
    Q_OBJECT
public:
    explicit MemoryView(QWidget* parent);
    ~MemoryView() override = default;

    void setMemory(stm32::Memory& memory);
    void updateContents();
    void reset();

public:
    RESTRICT_COPY(MemoryView);

private:
    void init();

    std::optional<std::reference_wrapper<stm32::Memory>> m_memory{};

    HexView* m_flashHexView = nullptr;
    HexView* m_sramHexView = nullptr;
};

}  // namespace app
