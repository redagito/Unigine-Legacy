#pragma once

#ifdef _WIN32
// loads the GL_NV_fragment_program entry points, glad does not ship them
void glext_init();
#endif
