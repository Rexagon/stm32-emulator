#pragma once

#include <QObject>
#include <memory>

#include "models/settings.hpp"

namespace app
{
class Application final : public QObject {
    Q_OBJECT
public:
    explicit Application(Settings& settings);

    void loadFile(const QString& path);

signals:
    void assemblyLoaded(const QString& assembly);
    void binaryLoaded(std::shared_ptr<QByteArray> data);

private:
    Settings& m_settings;
};

}  // namespace app
