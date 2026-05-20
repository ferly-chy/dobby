#pragma once

#include <string>
#include <expected>
#include <memory>
#include <vector>

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
    static PluginManager& Instance() {
        static PluginManager instance;
        return instance;
    }

    void Register(std::unique_ptr<Plugin> plugin) {
        plugins_.push_back(std::move(plugin));
    }

    const std::vector<std::unique_ptr<Plugin>>& GetPlugins() const {
        return plugins_;
    }

private:
    PluginManager() = default;
    std::vector<std::unique_ptr<Plugin>> plugins_;
};

} // namespace dobby
