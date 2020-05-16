#pragma once

#include <QObject>
#include <memory>
#include <stm32/stm32.hpp>

#include "models/assembly_view_model.hpp"
#include "models/settings.hpp"

namespace app
{
class Application final : public QObject {
    Q_OBJECT

    struct ApplicationState {
        QByteArray flash;
        stm32::Cpu cpu;
    };

public:
    explicit Application(Settings& settings);

    void loadFile(const QString& path);

    inline auto assemblyViewModel() -> AssemblyViewModel* { return &m_assemblyViewModel; }

signals:
    void memoryLoaded(QByteArray data);

private:
    void initCpu(QByteArray flash);

    Settings& m_settings;

    std::optional<ApplicationState> m_state{};

    AssemblyViewModel m_assemblyViewModel;
};

}  // namespace app
