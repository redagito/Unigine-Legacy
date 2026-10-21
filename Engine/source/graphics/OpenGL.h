#pragma once

#ifdef _WIN32
#include <windows.h>
#include <glad/gl.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#endif

// the Khronos registry no longer ships GL_NV_fragment_program
#ifndef GL_PROGRAM_ERROR_POSITION_NV
#define GL_PROGRAM_ERROR_POSITION_NV 0x8E8B
#endif

#ifdef _WIN32
// glad cannot load these either, they come from the hand rolled loader
typedef void (WINAPI * PFNGLGENPROGRAMSNVPROC) (GLsizei, GLuint*);
typedef void (WINAPI * PFNGLBINDPROGRAMNVPROC) (GLenum, GLuint);
typedef void (WINAPI * PFNGLDELETEPROGRAMSNVPROC) (GLsizei, const GLuint*);
typedef void (WINAPI * PFNGLLOADPROGRAMNVPROC) (GLenum, GLuint, GLsizei, const GLubyte*);

extern PFNGLGENPROGRAMSNVPROC glGenProgramsNV;
extern PFNGLBINDPROGRAMNVPROC glBindProgramNV;
extern PFNGLDELETEPROGRAMSNVPROC glDeleteProgramsNV;
extern PFNGLLOADPROGRAMNVPROC glLoadProgramNV;
#endif

inline const char* gl_error_string(GLenum error) {
	switch (error) {
		case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
		case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW";
		case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW";
		case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
		case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
		default: return "unknown";
	}
}
