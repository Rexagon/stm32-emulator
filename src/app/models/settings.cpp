// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "settings.hpp"

#include <QDir>
#include <QSettings>

namespace app
{
namespace
{
constexpr auto OBJDUMP_DEFAULT_PATH = "llvm-objdump";
constexpr auto OBJCOPY_DEFAULT_PATH = "llvm-objcopy";

constexpr auto SETTINGS_OBJDUMP_PATH = "internal/objdump-path";
constexpr auto SETTINGS_OBJCOPY_PATH = "internal/objcopy-path";
constexpr auto SETTINGS_DEFAULT_DIRECTORY = "internal/default-directory";

template <typename T>
void set(QSettings& settings, const QString& key, const std::optional<T>& value)
{
    if (value) {
        settings.setValue(key, *value);
    }
    else {
        settings.remove(key);
    }
}

template <typename T>
std::optional<T> get(const QSettings& settings, const QString& key)
{
    const auto variant = settings.value(key);

    if (variant.isNull()) {
        return std::nullopt;
    }

    return variant.value<T>();
}

}  // namespace

Settings::Settings()
{
}

auto Settings::objdumpPath() -> QString
{
    return get<QString>(m_settings, SETTINGS_OBJDUMP_PATH).value_or(OBJDUMP_DEFAULT_PATH);
}

void Settings::setObjdumpPath(const QString& path)
{
    set(m_settings, SETTINGS_OBJDUMP_PATH, std::optional{path});
}

auto Settings::objcopyDirectory() -> QString
{
    return get<QString>(m_settings, SETTINGS_OBJCOPY_PATH).value_or(OBJCOPY_DEFAULT_PATH);
}

void Settings::setObjcopyDirectory(const QString& path)
{
    set(m_settings, SETTINGS_OBJCOPY_PATH, std::optional{path});
}

auto Settings::defaultDirectory() -> QString
{
    return get<QString>(m_settings, SETTINGS_DEFAULT_DIRECTORY).value_or(QDir::homePath());
}

void Settings::setDefaultDirectory(const QString& path)
{
    set(m_settings, SETTINGS_DEFAULT_DIRECTORY, std::optional{path});
}

}  // namespace app
