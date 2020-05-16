// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "main_window.hpp"

namespace app
{
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow{parent}
    , m_ui{this}
{
    registerEvents();
}

void MainWindow::registerEvents()
{
    connect(m_ui.mainMenu(), &MainMenu::openFile, this, &MainWindow::openFile);
    connect(m_ui.mainMenu(), &MainMenu::exitApplication, this, &MainWindow::exitApplication);
}

void MainWindow::openFile()
{
    if (m_fileDialog != nullptr) {
        return;
    }

    m_fileDialog = new QFileDialog{this};
    m_fileDialog->setFileMode(QFileDialog::FileMode::ExistingFile);
    m_fileDialog->setAcceptMode(QFileDialog::AcceptMode::AcceptOpen);
    m_fileDialog->setDirectory(QDir::home());
    m_fileDialog->setModal(true);
    m_fileDialog->show();

    connect(m_fileDialog, &QFileDialog::fileSelected, [this](const QString&) {

    });
    connect(m_fileDialog, &QFileDialog::finished, [this](int) {
        m_fileDialog->close();
        m_fileDialog = nullptr;
    });
}

void MainWindow::exitApplication()
{
    close();
}

}  // namespace app
