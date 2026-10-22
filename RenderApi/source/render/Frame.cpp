#include "render/Frame.h"

RenderFrame& currentFrame() {
	static RenderFrame current;
	return current;
}
