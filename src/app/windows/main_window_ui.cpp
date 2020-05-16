// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "main_window_ui.hpp"

#include <QGridLayout>
#include <QStatusBar>
#include <QSyntaxHighlighter>
#include <cassert>

#include "../widgets/custom_splitter.hpp"

namespace app
{
MainWindowUI::MainWindowUI(QMainWindow* window)
    : m_window{window}
{
    assert(window != nullptr);

    init();
}

void MainWindowUI::init()
{
    // Main menu
    m_mainMenu = new MainMenu{m_window};
    m_window->setMenuBar(m_mainMenu);

    // Tool bar
    m_toolBar = new MainToolBar{m_window};
    m_window->addToolBar(m_toolBar);

    // Root container
    auto* rootContainer = new QWidget{m_window};
    auto* rootContainerLayout = new QVBoxLayout{rootContainer};
    rootContainerLayout->setContentsMargins(0, 0, 0, 0);

    auto* verticalSplitter = new CustomSplitter{Qt::Orientation::Vertical, m_window};
    rootContainerLayout->addWidget(verticalSplitter);

    // Top container
    auto* topContainer = new QWidget{m_window};
    verticalSplitter->addWidget(topContainer);
    auto topContainerLayout = new QHBoxLayout{topContainer};
    topContainerLayout->setContentsMargins(0, 0, 0, 0);

    auto* horizontalSplitter = new CustomSplitter{Qt::Orientation::Horizontal, m_window};
    topContainerLayout->addWidget(horizontalSplitter);

    // Top left container
    auto* topLeftContainer = new QWidget{m_window};
    horizontalSplitter->addWidget(topLeftContainer);
    auto* topLeftContainerLayout = new QVBoxLayout{topLeftContainer};
    topLeftContainerLayout->setContentsMargins(0, 0, 0, 0);

    m_assemblyView = new AssemblyView{m_window};
    topLeftContainerLayout->addWidget(m_assemblyView);

    // Top right container
    auto* topRightContainer = new QWidget{m_window};
    horizontalSplitter->addWidget(topRightContainer);
    auto* topRightContainerLayout = new QVBoxLayout{topRightContainer};

    m_registersView = new RegistersView{m_window};
    topRightContainerLayout->addWidget(m_registersView);

    // Bottom container
    auto* bottomContainer = new QWidget{m_window};
    verticalSplitter->addWidget(bottomContainer);
    auto bottomContainerLayout = new QHBoxLayout{bottomContainer};
    bottomContainerLayout->setContentsMargins(0, 0, 0, 0);

    m_memoryView = new MemoryView{m_window};
    bottomContainerLayout->addWidget(m_memoryView);

    // Status tip
    auto statusBar = new QStatusBar{m_window};
    m_window->setStatusBar(statusBar);

    // Finalize
    m_window->setCentralWidget(rootContainer);
}

}  // namespace app
