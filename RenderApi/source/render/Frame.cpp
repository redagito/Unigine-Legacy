#include "render/Frame.h"

RenderFrame& currentFrame() {
	static RenderFrame current;
	return current;
}

Defines& defines() {
	static Defines current;
	return current;
}
