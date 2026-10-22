#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/sink.h>
#include <vector>

namespace logger {

	// opens the console and file sinks, does nothing if already open
	void init(const char* file_name);

	// adds a sink to the engine logger
	void add_sink(spdlog::sink_ptr sink);

	void shutdown();

}
