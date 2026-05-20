#pragma once

#include <expected>
#include <memory>
#include <string>
#include <vector>

#include "Config/Config.h"

namespace dobby {

enum class PluginError {
    Success = 0,
    InitializationFailed,
    InvalidConfiguration,
    DependencyMissing,
    InternalError
};

class Plugin {
public:
    virtual ~Plugin() = default;

    virtual const char* GetName() const = 0;
    virtual const char* GetVersion() const = 0;

    virtual std::expected<void, PluginError> OnLoad() { return {}; }
    virtual std::expected<void, PluginError> OnEnable() { return {}; }
    virtual void OnDisable() {}
    virtual void OnUnload() {}
};

class PluginManager {
public:
    static PluginManager& Instance();

    void Register(std::unique_ptr<Plugin> plugin);
    [[nodiscard]] std::expected<void, PluginError> LoadAll();
    [[nodiscard]] std::expected<void, PluginError> EnableAll();
    [[nodiscard]] std::expected<void, PluginError> ConfigureFrom(const DobbyConfig& config);
    void DisableAll();
    void UnloadAll();
    void Reset();

    const std::vector<std::unique_ptr<Plugin>>& GetPlugins() const;

private:
    PluginManager() = default;
    std::vector<std::unique_ptr<Plugin>> plugins_;
};

std::expected<void, PluginError> DobbyInitializePluginsFromConfig(const char* config_path);

} // namespace dobby
