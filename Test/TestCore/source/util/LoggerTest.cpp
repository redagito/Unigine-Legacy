#include <catch2/catch_test_macros.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/sink.h>

#include <algorithm>
#include <cstdio>
#include <memory>

#include "util/Logger.h"

namespace {

class TestSink : public spdlog::sinks::sink {
public:
	void log(const spdlog::details::log_msg&) override {}
	void flush() override {}
	void set_pattern(const std::string&) override {}
	void set_formatter(std::unique_ptr<spdlog::formatter>) override {}
};

bool has_sink(const std::shared_ptr<spdlog::logger>& logger, const spdlog::sink_ptr& sink) {
	const auto& sinks = logger->sinks();
	return std::find(sinks.begin(), sinks.end(), sink) != sinks.end();
}

}

TEST_CASE("logger::remove_sink removes the sink from every logger", "[core][logger]") {
	const char* file_name = "logger_test.log";
	struct Cleanup {
		explicit Cleanup(const char* file_name) : file_name(file_name) { logger::init(file_name); }
		~Cleanup() {
			logger::shutdown();
			std::remove(file_name);
		}
		const char* file_name;
	} cleanup(file_name);

	auto console_logger = spdlog::get("console");
	REQUIRE(console_logger);

	auto sink = std::make_shared<TestSink>();
	console_logger->sinks().push_back(sink);
	logger::add_sink(sink);
	REQUIRE(has_sink(spdlog::default_logger(), sink));
	REQUIRE(has_sink(console_logger, sink));

	logger::remove_sink(sink);

	REQUIRE_FALSE(has_sink(spdlog::default_logger(), sink));
	REQUIRE_FALSE(has_sink(console_logger, sink));
}
