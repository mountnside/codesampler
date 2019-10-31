#ifndef CREATEGL_H
#define CREATEGL_H

#include <new>
#include "projtex.h"

// For convenience, this project ships with its own "glext.h" extension header 
// file. If you have trouble running this sample, it may be that this "glext.h" 
// file is defining something that your hardware doesn’t actually support. 
// Try recompiling the sample using your own local, vendor-specific "glext.h" 
// header file.

#include "glext.h"      // Sample's header file
//#include <GL/glext.h> // Your local header file

struct VertexData
{
	float u,v;
	float r,g,b,a;
	float nx,ny,nz;
	float x,y,z;
};


class CreateGL
{
	public:
		CreateGL();
		~CreateGL();
		void InitGL();
		void KillGL();
		void Draw();
		void SetHWND(HWND hw){hWnd = hw;}
		HWND GetHWND(){return hWnd;}
		static LRESULT CALLBACK WndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);

	private:
		HDC hDC;
		HGLRC hRC;
		HWND hWnd;
		TexProj *TexObj;
		Matrix4x4f ObjectTransform, TextureTransform;
		GLuint displayList;
		float MVmatrix[16];

		static float rotateObjectX,rotateObjectY;
		static float rotateTextureX,rotateTextureY;
		static float moveEyePoint;

		static bool objectMove,textureMove;
};
#endif