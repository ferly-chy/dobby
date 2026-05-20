#pragma once

#include "Plugins/Plugin.h"

namespace dobby {

class PosixSocketMonitor : public Plugin {
public:
    const char* GetName() const override { return "PosixSocketMonitor"; }
    const char* GetVersion() const override { return "1.0.0"; }

    std::expected<void, PluginError> OnEnable() override;
    void OnDisable() override;

private:
    bool hooked_ = false;
};

} // namespace dobby
