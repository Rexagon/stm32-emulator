#pragma once

#include <QAction>
#include <QMenuBar>

#include "../utils/general.hpp"

namespace app
{
class MainMenu final : public QMenuBar {
    Q_OBJECT
public:
    explicit MainMenu(QWidget* parent);
    ~MainMenu() override = default;

public:
    RESTRICT_COPY(MainMenu);

signals:
    void openFile();
    void exitApplication();
    void openPreferences();

private:
    void init();

    void loadActions();

    QAction* m_openFileAction = nullptr;
    QAction* m_exitApplicationAction = nullptr;
    QAction* m_openPreferencesAction = nullptr;
};

}  // namespace app
