#include "check.hpp"
#include <manmenmi/core/runtime.hpp>

#include <memory>
#include <sstream>
#include <type_traits>

int main() {
    using namespace manmenmi;
    static_assert(!std::is_default_constructible_v<Result<int>>);
    Result<int> value{42};
    CHECK(value);
    CHECK(value.value() == 42);
    Result<int> error{Error{ErrorCode::invalid_argument, "specific reason"}};
    CHECK(!error);
    CHECK(error.error().code == ErrorCode::invalid_argument);
    CHECK(error.error().message == "specific reason");
    Result<std::unique_ptr<int>> owned{std::make_unique<int>(7)};
    const auto moved = std::move(owned).value();
    CHECK(*moved == 7);
    CHECK(success());
    for (const auto code : {ErrorCode::invalid_argument, ErrorCode::io_error,
            ErrorCode::unimplemented, ErrorCode::unavailable, ErrorCode::invalid_state, ErrorCode::internal_error}) {
        CHECK(std::string_view{error_name(code)} != "UNKNOWN_ERROR");
    }
    CHECK(std::string_view{error_name(static_cast<ErrorCode>(999))} == "UNKNOWN_ERROR");
    CHECK(backend_inventory().size() == 3);
    for (const auto& entry : backend_inventory()) {
        CHECK(entry.state == ImplementationState::unimplemented);
        CHECK(parse_backend(backend_name(entry.backend)).value() == entry.backend);
    }
    CHECK(!parse_backend("Vulkan"));
    CHECK(!parse_backend("metal"));
    for (auto requested : {Backend::automatic, Backend::vulkan, Backend::dx12, Backend::opengl}) {
        const auto expected = requested == Backend::automatic ? Backend::vulkan : requested;
        const auto strict = backend_candidates(requested, false);
        CHECK(strict.size() == 1);
        CHECK(strict.front() == expected);
        const auto fallback = backend_candidates(requested, true);
        CHECK(fallback.size() == 3);
        CHECK(fallback.front() == expected);
        CHECK(fallback[0] != fallback[1] && fallback[1] != fallback[2] && fallback[0] != fallback[2]);
        for (const bool enabled : {false, true}) {
            std::ostringstream sink;
            Logger logger{sink, LogLevel::error};
            const auto runtime = create_runtime(Config{requested, enabled, LogLevel::error}, logger);
            CHECK(!runtime);
            CHECK(runtime.error().code == ErrorCode::unimplemented);
            CHECK(sink.str().find("UNIMPLEMENTED") != std::string::npos);
            CHECK((sink.str().find("explicit fallback") != std::string::npos) == enabled);
        }
    }
    return checks::finish();
}