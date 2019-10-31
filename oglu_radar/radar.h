#ifndef RADAR_H
#define RADAR_H

#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glaux.h>
//#include <gl/wglext.h>
#include "wglext.h"

struct VertexData
{
	float u,v;
	float x,y,z;
};


typedef struct
{
	HPBUFFERARB hPbuffer;
	HDC hDC;
	HGLRC hRC;
	int width;
	int height;
}PBUFFER;



class Radar
{
	public:
		Radar();
		~Radar();
		void InitGL();
		void KillGL();
		void Draw();
		void DrawBackground();
		void DrawOverlay(double );
		DWORD FrameTimeDifference(DWORD time);
		void SetHWND(HWND hw){hWnd = hw;}
		HWND GetHWND(){return hWnd;}
		void RenderBlur();
		void makeTexture(int dimension, int channels);
		void InitializePBuffer();
		static LRESULT CALLBACK WndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);

	private:
		DWORD lasttime;
		PBUFFER pbuffer;
		static int size;
		GLuint texId,barktex,floortex;
		HDC hDC;
		HGLRC hRC;
		HWND hWnd;
		GLUquadricObj *line;
		char *extensions;
};
#endif