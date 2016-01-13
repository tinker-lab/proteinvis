//
// Copyright (c) 2009-2013 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
#ifndef GLFONTSTASH_H
#define GLFONTSTASH_H

struct FONScontext* glfonsCreate(int width, int height, int flags);
void glfonsDelete(struct FONScontext* ctx);

unsigned int glfonsRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

#endif

#ifdef GLFONTSTASH_IMPLEMENTATION

struct GLFONScontext {
	GLuint tex;
	int width, height;
};

static int glfons__renderCreate(void* userPtr, int width, int height)
{
	struct GLFONScontext* gl = (struct GLFONScontext*)userPtr;
	// Create may be called multiple times, delete existing texture.
	if (gl->tex != 0) {
		glDeleteTextures(1, &gl->tex);
		gl->tex = 0;
	}
	glGenTextures(1, &gl->tex);
	if (!gl->tex) return 0;
	gl->width = width;
	gl->height = height;
	glBindTexture(GL_TEXTURE_2D, gl->tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, gl->width, gl->height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	return 1;
}

static int glfons__renderResize(void* userPtr, int width, int height)
{
	// Reuse create to resize too.
	return glfons__renderCreate(userPtr, width, height);
}

static void glfons__renderUpdate(void* userPtr, int* rect, const unsigned char* data)
{
	struct GLFONScontext* gl = (struct GLFONScontext*)userPtr;
	int w = rect[2] - rect[0];
	int h = rect[3] - rect[1];

	if (gl->tex == 0) return;
	glBindTexture(GL_TEXTURE_2D, gl->tex);
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, gl->width);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, rect[0]);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, rect[1]);
	glTexSubImage2D(GL_TEXTURE_2D, 0, rect[0], rect[1], w, h, GL_ALPHA,GL_UNSIGNED_BYTE, data);
}

static void glfons__renderDraw(void* userPtr, const float* verts, const float* tcoords, const unsigned int* colors, int nverts)
{
	struct GLFONScontext* gl = (struct GLFONScontext*)userPtr;
	if (gl->tex == 0) return;
	glBindTexture(GL_TEXTURE_2D, gl->tex);
	glEnable(GL_TEXTURE_2D);

	
	// create the vao
	GLuint vaoID, vbo = 0;
	glGenVertexArrays(1, &vaoID);
	glBindVertexArray(vaoID);
				
	// create the vbo
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// initialize size
	int vertSize = sizeof(float)*nverts*2;
	int texCoordSize = sizeof(float)*nverts*2;
	int colorsSize = sizeof(unsigned int)*nverts;
	glBufferData(GL_ARRAY_BUFFER, vertSize+texCoordSize+colorsSize, 0, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertSize, verts);                             // copy vertices starting from 0 offest
	glBufferSubData(GL_ARRAY_BUFFER, vertSize, texCoordSize, tcoords);                // copy normals after vertices
	glBufferSubData(GL_ARRAY_BUFFER, vertSize+texCoordSize, colorsSize, colors);

	// set up vertex attributes
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)vertSize);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_FALSE, 0, (void*)(vertSize+texCoordSize));

	glDrawArrays(GL_TRIANGLES, 0, nverts);

	glBindVertexArray(0);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vaoID);
}

static void glfons__renderDelete(void* userPtr)
{
	struct GLFONScontext* gl = (struct GLFONScontext*)userPtr;
	if (gl->tex != 0)
		glDeleteTextures(1, &gl->tex);
	gl->tex = 0;
	free(gl);
}


struct FONScontext* glfonsCreate(int width, int height, int flags)
{
	struct FONSparams params;
	struct GLFONScontext* gl;

	gl = (struct GLFONScontext*)malloc(sizeof(struct GLFONScontext));
	if (gl == NULL) goto error;
	memset(gl, 0, sizeof(struct GLFONScontext));

	memset(&params, 0, sizeof(params));
	params.width = width;
	params.height = height;
	params.flags = (unsigned char)flags;
	params.renderCreate = glfons__renderCreate;
	params.renderResize = glfons__renderResize;
	params.renderUpdate = glfons__renderUpdate;
	params.renderDraw = glfons__renderDraw; 
	params.renderDelete = glfons__renderDelete;
	params.userPtr = gl;

	return fonsCreateInternal(&params);

error:
	if (gl != NULL) free(gl);
	return NULL;
}

void glfonsDelete(struct FONScontext* ctx)
{
	fonsDeleteInternal(ctx);
}

unsigned int glfonsRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	return (r) | (g << 8) | (b << 16) | (a << 24);
}

#endif
