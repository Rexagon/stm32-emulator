// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <QApplication>

#include "application.hpp"
#include "models/assembly_view_model.hpp"
#include "models/settings.hpp"
#include "windows/main_window.hpp"

auto main(int argc, char** argv) -> int
{
    QApplication gui{argc, argv};
    QApplication::setApplicationName("stm32 emulator");

    //
    app::AssemblyViewModel assemblyViewModel;

    app::Settings settings;

    app::Application application{settings};

    app::MainWindow mainWindow{settings};
    mainWindow.assemblyView()->setModel(&assemblyViewModel);

    //
    QWidget::connect(&mainWindow, &app::MainWindow::fileSelected, &application, &app::Application::loadFile);
    QWidget::connect(&mainWindow, &app::MainWindow::exitRequested, &mainWindow, &app::MainWindow::close);

    QWidget::connect(mainWindow.toolBar(), &app::MainToolBar::stopExecution, &application, &app::Application::resetCpu);
    QWidget::connect(mainWindow.toolBar(), &app::MainToolBar::pauseExecution, &application, &app::Application::pauseExecution);
    QWidget::connect(mainWindow.toolBar(), &app::MainToolBar::nextInstruction, &application, &app::Application::executeNextInstruction);
    QWidget::connect(mainWindow.toolBar(), &app::MainToolBar::nextBreakpoint, &application, &app::Application::executeUntilBreakpoint);

    QWidget::connect(&assemblyViewModel, &app::AssemblyViewModel::breakpointAdded, &application, &app::Application::addBreakpoint);
    QWidget::connect(&assemblyViewModel, &app::AssemblyViewModel::breakpointRemoved, &application, &app::Application::removeBreakpoint);

    QWidget::connect(&application, &app::Application::stateChanged, mainWindow.memoryView(), &app::MemoryView::reset);
    QWidget::connect(&application, &app::Application::stateChanged, mainWindow.registersView(), &app::RegistersView::reset);

    QWidget::connect(&application, &app::Application::assemblyLoaded, &assemblyViewModel, &app::AssemblyViewModel::tryFillFromString);
    QWidget::connect(&application, &app::Application::memoryLoaded, mainWindow.memoryView(), &app::MemoryView::setMemory);
    QWidget::connect(&application, &app::Application::registersLoaded, mainWindow.registersView(), &app::RegistersView::setRegisters);

    QWidget::connect(&application, &app::Application::instructionSelected, &assemblyViewModel, &app::AssemblyViewModel::setCurrentAddress);
    QWidget::connect(&application, &app::Application::instructionSelected, mainWindow.memoryView(), &app::MemoryView::updateContents);
    QWidget::connect(&application, &app::Application::instructionSelected, mainWindow.registersView(), &app::RegistersView::updateContents);
    QWidget::connect(&application, &app::Application::instructionSelected, mainWindow.assemblyView(), &app::AssemblyView::scrollToAddress);

    //
    mainWindow.show();

    return QApplication::exec();
}
