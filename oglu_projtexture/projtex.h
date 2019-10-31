#ifndef PROJTEX_H
#define PROJTEX_H

#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glaux.h>
#include "matrix4x4.h"

class TexProj
{
	public:
		TexProj();
		~TexProj();

		void loadLightMapTexture(const char *name);
		void loadTexture(const char *name);
		void textureProjection(Matrix4x4f &mv);
		void BindProjectiveTexture(bool exp);

	private:
		GLuint texObject, projTexture;
		int texWidth, texHeight;
};
#endif