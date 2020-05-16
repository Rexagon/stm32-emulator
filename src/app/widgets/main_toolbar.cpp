// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "main_toolbar.hpp"

namespace app
{
app::MainToolBar::MainToolBar(QWidget* parent)
    : QToolBar{parent}
{
    init();
}

void MainToolBar::init()
{
    loadActions();

    addAction(m_startExecutionAction);
    addAction(m_stopExecutionAction);
    addAction(m_pauseExecutionAction);

    addSeparator();

    addAction(m_nextInstructionAction);
    addAction(m_nextBreakpointAction);
}

void MainToolBar::loadActions()
{
    m_startExecutionAction = new QAction{QIcon{":/icons/play.png"}, tr("&Start"), this};
    m_startExecutionAction->setStatusTip(tr("Start execution"));
    connect(m_startExecutionAction, &QAction::triggered, this, &MainToolBar::startExecution);

    m_stopExecutionAction = new QAction{QIcon{":/icons/stop.png"}, tr("Sto&p"), this};
    m_stopExecutionAction->setStatusTip(tr("Stop execution"));
    connect(m_stopExecutionAction, &QAction::triggered, this, &MainToolBar::stopExecution);

    m_pauseExecutionAction = new QAction{QIcon{":/icons/pause.png"}, tr("&Pause"), this};
    m_pauseExecutionAction->setStatusTip(tr("Pause execution"));
    connect(m_pauseExecutionAction, &QAction::triggered, this, &MainToolBar::pauseExecution);

    m_nextInstructionAction = new QAction{QIcon{":/icons/next.png"}, tr("&Next"), this};
    m_pauseExecutionAction->setStatusTip(tr("Go to next instruction"));
    connect(m_pauseExecutionAction, &QAction::triggered, this, &MainToolBar::nextInstruction);

    m_nextBreakpointAction = new QAction{QIcon{":/icons/last.png"}, tr("&Breakpoint"), this};
    m_nextBreakpointAction->setStatusTip(tr("Go to next breakpoint"));
    connect(m_nextBreakpointAction, &QAction::triggered, this, &MainToolBar::nextBreakpoint);
}

}  // namespace app
