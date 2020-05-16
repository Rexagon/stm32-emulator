// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "main_menu.hpp"

namespace app
{
MainMenu::MainMenu(QWidget* parent)
    : QMenuBar{parent}
{
    init();
}

void MainMenu::init()
{
    loadActions();

    auto* fileMenu = addMenu(tr("&File"));
    fileMenu->addAction(m_openFileAction);
    fileMenu->addAction(m_exitApplicationAction);

    auto* editMenu = addMenu(tr("&Edit"));
    editMenu->addAction(m_openPreferencesAction);
}

void MainMenu::loadActions()
{
    m_openFileAction = new QAction{tr("&Open"), this};
    m_openFileAction->setShortcut(QKeySequence::New);
    m_openFileAction->setStatusTip(tr("Open ELF file"));
    connect(m_openFileAction, &QAction::triggered, this, &MainMenu::openFile);

    m_exitApplicationAction = new QAction{tr("&Exit"), this};
    m_exitApplicationAction->setShortcut(QKeySequence::Quit);
    m_exitApplicationAction->setStatusTip(tr("Exit application"));
    connect(m_exitApplicationAction, &QAction::triggered, this, &MainMenu::exitApplication);

    m_openPreferencesAction = new QAction{tr("&Preferences"), this};
    m_openPreferencesAction->setShortcut(QKeySequence::Preferences);
    m_openPreferencesAction->setStatusTip(tr("Open preferences"));
    connect(m_openPreferencesAction, &QAction::triggered, this, &MainMenu::openPreferences);
}

}  // namespace app
