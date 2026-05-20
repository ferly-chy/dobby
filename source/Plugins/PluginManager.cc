#include "Plugins/Plugin.h"

#include <memory>

#include "Plugins/ApplicationEventMonitor/DynamicLoaderMonitor.h"
#include "Plugins/ApplicationEventMonitor/PosixFileMonitor.h"
#include "Plugins/ApplicationEventMonitor/PosixSocketMonitor.h"

namespace dobby {

PluginManager& PluginManager::Instance() {
    static PluginManager instance;
    return instance;
}

void PluginManager::Register(std::unique_ptr<Plugin> plugin) {
    plugins_.push_back(std::move(plugin));
}

std::expected<void, PluginError> PluginManager::LoadAll() {
    for (const auto& plugin : plugins_) {
        if (auto result = plugin->OnLoad(); !result) {
            return std::unexpected(result.error());
        }
    }
    return {};
}

std::expected<void, PluginError> PluginManager::EnableAll() {
    for (const auto& plugin : plugins_) {
        if (auto result = plugin->OnEnable(); !result) {
            return std::unexpected(result.error());
        }
    }
    return {};
}

std::expected<void, PluginError> PluginManager::ConfigureFrom(const DobbyConfig& config) {
    Reset();

    if (config.application_event_monitor.posix_file) {
        Register(std::make_unique<PosixFileMonitor>());
    }
    if (config.application_event_monitor.posix_socket) {
        Register(std::make_unique<PosixSocketMonitor>());
    }
    if (config.application_event_monitor.dynamic_loader) {
        Register(std::make_unique<DynamicLoaderMonitor>());
    }

    return {};
}

void PluginManager::DisableAll() {
    for (auto it = plugins_.rbegin(); it != plugins_.rend(); ++it) {
        (*it)->OnDisable();
    }
}

void PluginManager::UnloadAll() {
    for (auto it = plugins_.rbegin(); it != plugins_.rend(); ++it) {
        (*it)->OnUnload();
    }
}

void PluginManager::Reset() {
    DisableAll();
    UnloadAll();
    plugins_.clear();
}

const std::vector<std::unique_ptr<Plugin>>& PluginManager::GetPlugins() const {
    return plugins_;
}

std::expected<void, PluginError> DobbyInitializePluginsFromConfig(const char* config_path) {
    if (config_path == nullptr) {
        return std::unexpected(PluginError::InvalidConfiguration);
    }

    auto config = ConfigLoader::LoadFromFile(config_path);
    if (!config) {
        return std::unexpected(PluginError::InvalidConfiguration);
    }

    auto& manager = PluginManager::Instance();
    if (auto result = manager.ConfigureFrom(*config); !result) {
        return result;
    }
    if (auto result = manager.LoadAll(); !result) {
        return result;
    }
    return manager.EnableAll();
}

} // namespace dobby
