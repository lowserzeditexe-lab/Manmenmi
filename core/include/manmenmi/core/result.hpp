#pragma once

#include <string>
#include <utility>
#include <variant>

namespace manmenmi {
enum class ErrorCode { invalid_argument, io_error, unimplemented, unavailable, invalid_state, internal_error };

struct Error {
    ErrorCode code;
    std::string message;
};

[[nodiscard]] const char* error_name(ErrorCode code) noexcept;

// No implicit success or default construction. Incorrect access is a programming error.
template <typename T> class [[nodiscard]] Result {
public:
    explicit Result(T value) : data_(std::move(value)) {}
    explicit Result(Error error) : data_(std::move(error)) {}
    [[nodiscard]] bool has_value() const noexcept { return std::holds_alternative<T>(data_); }
    explicit operator bool() const noexcept { return has_value(); }
    T& value() & { return std::get<T>(data_); }
    const T& value() const& { return std::get<T>(data_); }
    T&& value() && { return std::get<T>(std::move(data_)); }
    const Error& error() const& { return std::get<Error>(data_); }
private:
    std::variant<T, Error> data_;
};

using Status = Result<std::monostate>;
[[nodiscard]] Status success();
} // namespace manmenmi