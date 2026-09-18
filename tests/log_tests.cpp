#include "check.hpp"
#include <manmenmi/core/log.hpp>

#include <sstream>
#include <string>
#include <thread>
#include <vector>

int main() {
    using namespace manmenmi;
    std::ostringstream sink;
    Logger logger{sink, LogLevel::warning};
    logger.write(LogLevel::debug, LogCategory::core, "hidden");
    logger.write(LogLevel::info, LogCategory::config, "hidden");
    CHECK(sink.str().empty());
    logger.write(LogLevel::warning, LogCategory::window, "visible");
    logger.write(LogLevel::error, LogCategory::graphics, "bad\nline\r\t\x1b");
    CHECK(sink.str() == "[warning][window] visible\n[error][graphics] bad\\nline\\r\\t?\n");
    for (const auto level : {LogLevel::debug, LogLevel::info, LogLevel::warning, LogLevel::error}) {
        CHECK(parse_log_level(log_level_name(level)).value() == level);
    }
    CHECK(!parse_log_level("ERROR"));
    std::ostringstream concurrent_sink;
    Logger concurrent{concurrent_sink};
    std::vector<std::thread> workers;
    for (int i = 0; i < 4; ++i) {
        workers.emplace_back([&concurrent] {
            for (int j = 0; j < 100; ++j) { concurrent.write(LogLevel::info, LogCategory::test, "whole-record"); }
        });
    }
    for (auto& worker : workers) { worker.join(); }
    std::istringstream records{concurrent_sink.str()};
    int count = 0;
    for (std::string line; std::getline(records, line); ++count) {
        CHECK(line == "[info][test] whole-record");
    }
    CHECK(count == 400);
    return checks::finish();
}