#pragma once

#include <memory>
#include <manmenmi/core/config.hpp>

namespace manmenmi {
class Runtime {
public:
    virtual ~Runtime() = default;
    virtual Status tick() = 0;
    virtual void request_stop() noexcept = 0;
};

// M0 always returns UNIMPLEMENTED and logs every candidate. No window/device exists.
// M1 will establish concrete ownership and platform adapters behind this boundary.
[[nodiscard]] Result<std::unique_ptr<Runtime>> create_runtime(const Config& config, Logger& logger);
} // namespace manmenmi