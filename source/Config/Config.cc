#include "Config/Config.h"

#include <toml++/toml.hpp>

namespace dobby {

namespace {

void ApplyApplicationEventMonitorConfig(const toml::table& table, DobbyConfig& config) {
    if (auto value = table["posix_file"].value<bool>()) {
        config.application_event_monitor.posix_file = *value;
    }
    if (auto value = table["posix_socket"].value<bool>()) {
        config.application_event_monitor.posix_socket = *value;
    }
    if (auto value = table["dynamic_loader"].value<bool>()) {
        config.application_event_monitor.dynamic_loader = *value;
    }
}

} // namespace

std::expected<DobbyConfig, ConfigError> ConfigLoader::LoadFromFile(const std::string& path) {
    DobbyConfig config;

    toml::parse_result result = toml::parse_file(path);
    if (!result) {
        const auto& error = result.error();
        if (error.source().path) {
            return std::unexpected(ConfigError::ParseError);
        }
        return std::unexpected(ConfigError::FileNotFound);
    }

    const toml::table& table = result.table();
    if (const auto* monitor = table["application_event_monitor"].as_table()) {
        ApplyApplicationEventMonitorConfig(*monitor, config);
    }

    return config;
}

} // namespace dobby
