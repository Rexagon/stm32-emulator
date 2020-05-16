#pragma once

#include <QObject>
#include <QSettings>

namespace app
{
class Settings final : QObject {
    Q_OBJECT
public:
    explicit Settings();

    auto objdumpPath() -> QString;
    void setObjdumpPath(const QString& path);

    auto objcopyDirectory() -> QString;
    void setObjcopyDirectory(const QString& path);

    auto defaultDirectory() -> QString;
    void setDefaultDirectory(const QString& path);

private:
    QSettings m_settings{};
};

}  // namespace app
