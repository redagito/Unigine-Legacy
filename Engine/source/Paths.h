#pragma once

#include <vector>
#include <string>

// Storage for file paths
class Paths
{
public:
	void addPath(const char* path);
	const char* findFile(const char* name) const;

private:
	// path
	std::vector<std::string> paths;
};