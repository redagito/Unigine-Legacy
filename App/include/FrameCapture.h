#pragma once

// the last frame the engine rendered
class FrameCapture {
public:
	// Reads only the app's current framebuffer, before the buffer swap.
	static void capture(int width, int height);

	// writes the last rendered frame as a png, returns the number of pixels
	// that are not black, -1 when the frame could not be written
	static int write(const char* path);
};
