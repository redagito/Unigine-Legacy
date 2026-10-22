#pragma once

// the last frame the engine rendered
class FrameCapture {
public:

	// writes the last rendered frame as a png, returns the number of pixels
	// that are not black, -1 when the frame could not be written
	static int write(const char* path);
};
