//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================


// stub OpenGL header, all pddi gl code uses this instead of '#include <GL/gl.h>
#ifdef SRR2_GL_FRAMEWORK

// OpenGL.framework exports the full GL 2.1 entry point set, so no loader is
// needed. gl.h only reaches GL 1.1 unless GL_GLEXT_LEGACY is off, so glext.h
// supplies the 1.2+ prototypes and the compressed texture enums.
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>

#else

#include <glad/glad.h>

#endif
