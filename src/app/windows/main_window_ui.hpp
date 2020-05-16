#pragma once

#include <QLabel>
#include <QMainWindow>
#include <QPushButton>

#include "../widgets/assembly_view.hpp"
#include "../widgets/main_menu.hpp"
#include "../widgets/main_toolbar.hpp"
#include "../widgets/memory_view.hpp"
#include "../widgets/registers_view.hpp"

namespace app
{
class MainWindowUI final {
public:
    explicit MainWindowUI(QMainWindow* window);

    inline auto mainMenu() -> MainMenu* { return m_mainMenu; }
    inline auto toolBar() -> MainToolBar* { return m_toolBar; }

    inline auto assemblyView() -> AssemblyView* { return m_assemblyView; }
    inline auto registersView() -> RegistersView* { return m_registersView; }

    inline auto memoryView() -> MemoryView* { return m_memoryView; }

private:
    void init();

    QMainWindow* m_window;

    MainMenu* m_mainMenu = nullptr;
    MainToolBar* m_toolBar = nullptr;

    AssemblyView* m_assemblyView = nullptr;
    RegistersView* m_registersView = nullptr;

    MemoryView* m_memoryView = nullptr;
};

}  // namespace app
