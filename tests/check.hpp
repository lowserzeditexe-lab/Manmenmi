#pragma once
#include <iostream>

namespace checks {
inline int failures = 0;
inline int count = 0;
inline void check(bool passed, const char* expression, int line) {
    ++count;
    if (!passed) {
        ++failures;
        std::cerr << "FAIL line=" << line << " expression=" << expression << '\n';
    }
}
inline int finish() {
    std::cout << "checks=" << count << " failures=" << failures << '\n';
    return failures == 0 ? 0 : 1;
}
} // namespace checks
// Unlike assert(), checks execute with NDEBUG in Release.
#define CHECK(expression) checks::check(static_cast<bool>(expression), #expression, __LINE__)