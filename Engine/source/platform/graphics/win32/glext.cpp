#include "graphics/win32/glext.h"

#ifdef _WIN32

#include "graphics/OpenGL.h"

PFNGLGENPROGRAMSNVPROC glGenProgramsNV;
PFNGLBINDPROGRAMNVPROC glBindProgramNV;
PFNGLDELETEPROGRAMSNVPROC glDeleteProgramsNV;
PFNGLLOADPROGRAMNVPROC glLoadProgramNV;

void glext_init() {

	glGenProgramsNV = (PFNGLGENPROGRAMSNVPROC)wglGetProcAddress("glGenProgramsNV");
	glBindProgramNV = (PFNGLBINDPROGRAMNVPROC)wglGetProcAddress("glBindProgramNV");
	glDeleteProgramsNV = (PFNGLDELETEPROGRAMSNVPROC)wglGetProcAddress("glDeleteProgramsNV");
	glLoadProgramNV = (PFNGLLOADPROGRAMNVPROC)wglGetProcAddress("glLoadProgramNV");
}

#endif
