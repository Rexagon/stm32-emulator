#pragma once

#include <QFileDialog>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>

#include "../models/settings.hpp"
#include "../widgets/assembly_view.hpp"
#include "../widgets/main_menu.hpp"
#include "../widgets/main_toolbar.hpp"
#include "../widgets/memory_view.hpp"
#include "../widgets/registers_view.hpp"

namespace app
{
class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(Settings& settings);

    inline auto mainMenu() -> MainMenu* { return m_mainMenu; }
    inline auto toolBar() -> MainToolBar* { return m_toolBar; }

    inline auto assemblyView() -> AssemblyView* { return m_assemblyView; }
    inline auto registersView() -> RegistersView* { return m_registersView; }

    inline auto memoryView() -> MemoryView* { return m_memoryView; }

public:
    RESTRICT_COPY(MainWindow);

signals:
    void fileSelected(const QString& path);
    void exitRequested();

private:
    void init();
    void registerEvents();

    void openFileDialog();

    MainMenu* m_mainMenu = nullptr;
    MainToolBar* m_toolBar = nullptr;

    AssemblyView* m_assemblyView = nullptr;
    RegistersView* m_registersView = nullptr;

    MemoryView* m_memoryView = nullptr;

    QFileDialog* m_fileDialog = nullptr;

    Settings& m_settings;
};

}  // namespace app
