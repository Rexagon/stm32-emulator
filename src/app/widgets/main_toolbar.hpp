#pragma once

#include <QAction>
#include <QToolBar>

#include "../utils/general.hpp"

namespace app
{
class MainToolBar final : public QToolBar {
    Q_OBJECT
public:
    explicit MainToolBar(QWidget* parent);
    ~MainToolBar() override = default;

public:
    RESTRICT_COPY(MainToolBar);

signals:
    void stopExecution();
    void pauseExecution();
    void nextInstruction();
    void nextBreakpoint();

private:
    void init();

    void loadActions();

    QAction* m_pauseExecutionAction = nullptr;
    QAction* m_stopExecutionAction = nullptr;
    QAction* m_nextInstructionAction = nullptr;
    QAction* m_nextBreakpointAction = nullptr;
};

}  // namespace app
