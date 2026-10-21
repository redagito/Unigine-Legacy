#pragma once

// routes the driver debug messages to the log, the first error stops the app
void installDebugCallback();

// true when the driver reports its errors through the callback
bool debugCallbackInstalled();
