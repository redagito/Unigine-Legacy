#pragma once

#include <vector>
#include <string>

// Storage for file paths
class Paths
{
public:
	void addPath(const char* path);
	std::string findFile(const char* name) const;

	const std::vector<std::string>& getPaths() const { return paths; }

private:
	// path
	std::vector<std::string> paths;
};
