// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "main_window.hpp"

#include <QGridLayout>
#include <QStatusBar>

#include "../widgets/custom_splitter.hpp"

namespace app
{
MainWindow::MainWindow(Settings& settings)
    : QMainWindow{nullptr}
    , m_settings{settings}
{
    init();
    registerEvents();
}

void MainWindow::init()
{
    // Main menu
    m_mainMenu = new MainMenu{this};
    setMenuBar(m_mainMenu);

    // Tool bar
    m_toolBar = new MainToolBar{this};
    addToolBar(m_toolBar);

    // Root container
    auto* rootContainer = new QWidget{this};
    auto* rootContainerLayout = new QVBoxLayout{rootContainer};
    rootContainerLayout->setContentsMargins(0, 0, 0, 0);

    auto* verticalSplitter = new CustomSplitter{Qt::Orientation::Vertical, this};
    rootContainerLayout->addWidget(verticalSplitter);

    // Top container
    auto* topContainer = new QWidget{this};
    verticalSplitter->addWidget(topContainer);
    auto topContainerLayout = new QHBoxLayout{topContainer};
    topContainerLayout->setContentsMargins(0, 0, 0, 0);

    auto* horizontalSplitter = new CustomSplitter{Qt::Orientation::Horizontal, this};
    topContainerLayout->addWidget(horizontalSplitter);

    // Top left container
    auto* topLeftContainer = new QWidget{this};
    horizontalSplitter->addWidget(topLeftContainer);
    horizontalSplitter->setStretchFactor(0, 1);
    auto* topLeftContainerLayout = new QVBoxLayout{topLeftContainer};
    topLeftContainerLayout->setContentsMargins(0, 0, 0, 0);

    m_assemblyView = new AssemblyView{this};
    topLeftContainerLayout->addWidget(m_assemblyView);

    // Top right container
    auto* topRightContainer = new QWidget{this};
    horizontalSplitter->addWidget(topRightContainer);
    auto* topRightContainerLayout = new QVBoxLayout{topRightContainer};
    topRightContainerLayout->setContentsMargins(0, 0, 0, 0);

    m_registersView = new RegistersView{this};
    topRightContainerLayout->addWidget(m_registersView);

    // Bottom container
    auto* bottomContainer = new QWidget{this};
    verticalSplitter->addWidget(bottomContainer);
    auto bottomContainerLayout = new QHBoxLayout{bottomContainer};
    bottomContainerLayout->setContentsMargins(0, 0, 0, 0);

    m_memoryView = new MemoryView{this};
    bottomContainerLayout->addWidget(m_memoryView);

    // Status tip
    auto statusBar = new QStatusBar{this};
    setStatusBar(statusBar);

    // Finalize
    setCentralWidget(rootContainer);
}

void MainWindow::registerEvents()
{
    connect(m_mainMenu, &MainMenu::openFile, this, &MainWindow::openFileDialog);
    connect(m_mainMenu, &MainMenu::exitApplication, this, &MainWindow::exitRequested);
}

void MainWindow::openFileDialog()
{
    if (m_fileDialog != nullptr) {
        return;
    }

    m_fileDialog = new QFileDialog{this};
    m_fileDialog->setFileMode(QFileDialog::FileMode::ExistingFile);
    m_fileDialog->setAcceptMode(QFileDialog::AcceptMode::AcceptOpen);
    m_fileDialog->setDirectory(m_settings.defaultDirectory());
    m_fileDialog->setModal(true);
    m_fileDialog->show();

    connect(m_fileDialog, &QFileDialog::fileSelected, [this](const QString& filePath) {
        const auto directory = QFileInfo{filePath}.absoluteDir();
        m_settings.setDefaultDirectory(directory.path());

        emit MainWindow::fileSelected(filePath);
    });
    connect(m_fileDialog, &QFileDialog::finished, [this](int) {
        m_fileDialog->close();
        m_fileDialog = nullptr;
    });
}

}  // namespace app
