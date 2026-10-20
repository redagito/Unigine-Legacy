#include "util/Logger.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <memory>

namespace {

std::shared_ptr<spdlog::logger> engine_logger;

}

namespace logger {

void init(const char* file_name) {
	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(file_name, true);

	engine_logger = std::make_shared<spdlog::logger>("engine", spdlog::sinks_init_list{ console_sink, file_sink });
	spdlog::set_default_logger(engine_logger);

	// the on screen console logs without feeding itself back
	auto console_logger = std::make_shared<spdlog::logger>("console", spdlog::sinks_init_list{ console_sink, file_sink });
	spdlog::register_logger(console_logger);

	spdlog::set_level(spdlog::level::trace);
	spdlog::flush_on(spdlog::level::trace);
}

void add_sink(spdlog::sink_ptr sink) {
	if (engine_logger) engine_logger->sinks().push_back(sink);
}

void shutdown() {
	spdlog::shutdown();
	engine_logger = nullptr;
}

}
