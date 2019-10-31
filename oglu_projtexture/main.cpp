#include "CreateGL.h"

/*
This program demonstrates how one would go about doing a projected texture.
The sample here shows how a projected texture technique can be used to produce a light map.
The point is that even though you have very few vertices available for the fixed function
pipeline lighting solution, you can achieve nice per pixel lighting even though the surface 
has only a handful of vertices.
This sample draws a cube, only allowing the inside being visible via culling front facing polys, and then 
projects the light map texture on the second texture stage all through the fixed function pipeline.
The left mouse button will move the cube around and the right mouse button will move the projected 
light map around.
*/

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
	WNDCLASS wc;
	MSG msg;
	HWND hWindow;
	CreateGL *GLInstance = new CreateGL;

	memset(&msg,0,sizeof(msg));
	
	wc.lpszClassName = "Window_Class";
	wc.lpfnWndProc = CreateGL::WndProc;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.hInstance = hInstance;
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.hCursor = LoadCursor(hInstance,IDC_ARROW);
	wc.hIcon = LoadIcon(hInstance,IDI_APPLICATION);
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.lpszMenuName = "Projected Texture";

	if(!RegisterClass(&wc)){
		MessageBox(NULL,"could not register window class","ERROR",MB_OK);
		return 0;
	}

	hWindow = CreateWindow("Window_Class","Projected Texture",
				WS_OVERLAPPEDWINDOW,0,0,1024,768,NULL,NULL,hInstance,NULL);
	
	if(hWindow == NULL){
		MessageBox(NULL,"Could not create window","ERROR",MB_OK);
		return 0;
	}

	ShowWindow(hWindow,SW_SHOW);
	UpdateWindow(hWindow);

	GLInstance->SetHWND(hWindow);
	GLInstance->InitGL();

	while(msg.message != WM_QUIT){
		if(PeekMessage(&msg,NULL,0,0,PM_REMOVE)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
			GLInstance->Draw();
	}
	GLInstance->KillGL();
	delete GLInstance;
	return msg.wParam;
}