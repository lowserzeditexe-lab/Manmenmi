#include "check.hpp"

#include <ostream>
#include <sstream>

#include <manmenmi/core/log.hpp>
#include <manmenmi/window/window.hpp>
#include "../window/src/sdl_window.hpp"

namespace {
void set_sdl_driver(const char* value) {
#if defined(_WIN32)
    _putenv_s("SDL_VIDEODRIVER", value);
#else
    setenv("SDL_VIDEODRIVER", value, 1);
#endif
}
}

int main() {
    std::ostringstream sink;
    manmenmi::Logger logger(sink, manmenmi::LogLevel::debug);

    const manmenmi::window::WindowConfig config{"M1 native window", 1280, 720, true, true};
    {
        const auto window_result = manmenmi::window::detail::create_window(config, logger);
        if (window_result.has_value()) {
            auto& window = *window_result.value();
            CHECK(window.size().width == 1280);
            CHECK(window.size().height == 720);
            CHECK(window.is_visible() == true);
            CHECK(window.native_surface_token() != 0);
            CHECK(window.show().has_value());
            CHECK(window.hide().has_value());
            CHECK(window.resize(1920, 1080).has_value());
            CHECK(window.poll_events().has_value());
            CHECK(window.size().width == 1920);
            CHECK(window.size().height == 1080);
        } else {
            CHECK(window_result.error().code == manmenmi::ErrorCode::platform_error ||
                  window_result.error().code == manmenmi::ErrorCode::not_initialized ||
                  window_result.error().code == manmenmi::ErrorCode::unavailable);
        }
    }

    set_sdl_driver("invalid_driver_for_m1_window_test");
    const auto failing_result = manmenmi::window::detail::create_window(config, logger);
    CHECK(!failing_result.has_value());
    CHECK(failing_result.error().code == manmenmi::ErrorCode::platform_error ||
          failing_result.error().code == manmenmi::ErrorCode::not_initialized ||
          failing_result.error().code == manmenmi::ErrorCode::unavailable);

    set_sdl_driver("dummy");
    const auto dummy_result = manmenmi::window::detail::create_window(config, logger);
    if (dummy_result.has_value()) {
        CHECK(dummy_result.value()->native_surface_token() != 0);
    } else {
        CHECK(dummy_result.error().code == manmenmi::ErrorCode::platform_error ||
              dummy_result.error().code == manmenmi::ErrorCode::not_initialized ||
              dummy_result.error().code == manmenmi::ErrorCode::unavailable);
    }

    return checks::finish();
}
