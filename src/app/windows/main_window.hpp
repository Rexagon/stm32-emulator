#pragma once

#include <QFileDialog>
#include <memory>

#include "main_window_ui.hpp"

namespace app
{
class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

public:
    RESTRICT_COPY(MainWindow);

private:
    void registerEvents();

    void openFile();
    void exitApplication();

    MainWindowUI m_ui;

    QFileDialog* m_fileDialog = nullptr;
};

}  // namespace app
