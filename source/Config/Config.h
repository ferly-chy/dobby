#pragma once

#include <expected>
#include <string>

namespace dobby {

enum class ConfigError {
    FileNotFound = 0,
    ParseError,
    InvalidValue,
};

struct ApplicationEventMonitorConfig {
    bool posix_file = false;
    bool posix_socket = false;
    bool dynamic_loader = false;
};

struct DobbyConfig {
    ApplicationEventMonitorConfig application_event_monitor;
};

class ConfigLoader {
public:
    static std::expected<DobbyConfig, ConfigError> LoadFromFile(const std::string& path);
};

} // namespace dobby
