// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "application.hpp"

#include <QMessageBox>
#include <QProcess>

namespace app
{
Application::Application(Settings& settings)
    : m_settings{settings}
{
}

void Application::loadFile(const QString& path)
{
    QProcess objdump{};
    objdump.start(m_settings.objdumpPath(), QStringList{"-disassemble", "--full-leading-addr", "--triple=thumb", path});
    objdump.waitForFinished();
    if (objdump.exitCode() != 0) {
        QMessageBox::critical(nullptr, tr("Failed to objdump file"), "");
        return;
    }

    QString assemblyOutput{objdump.readAllStandardOutput()};

    QProcess objcopy{};
    objcopy.start(m_settings.objcopyDirectory(), QStringList{"-O", "binary", "--only-section=.text", path, "-"});
    objcopy.waitForFinished();
    if (objcopy.exitCode() != 0) {
        QMessageBox::critical(nullptr, tr("Failed to objcopy file"), "");
        return;
    }

    auto data = std::make_shared<QByteArray>(objcopy.readAllStandardOutput());

    emit assemblyLoaded(assemblyOutput);
    emit binaryLoaded(data);
}

}  // namespace app
