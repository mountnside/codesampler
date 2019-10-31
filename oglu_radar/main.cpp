#include "Radar.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
	WNDCLASS wc;
	MSG msg;
	HWND hWindow;
	Radar *radar = new Radar;

	memset(&msg,0,sizeof(msg));
	
	wc.lpszClassName = "Window_Class";
	wc.lpfnWndProc = Radar::WndProc;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.hInstance = hInstance;
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.hCursor = LoadCursor(hInstance,IDC_ARROW);
	wc.hIcon = LoadIcon(hInstance,IDI_APPLICATION);
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.lpszMenuName = "Radar";

	if(!RegisterClass(&wc)){
		MessageBox(NULL,"could not register window class","ERROR",MB_OK);
		return 0;
	}

	hWindow = CreateWindow("Window_Class","Radar",
				WS_OVERLAPPEDWINDOW,0,0,1024,768,NULL,NULL,hInstance,NULL);
	
	if(hWindow == NULL){
		MessageBox(NULL,"Could not create window","ERROR",MB_OK);
		return 0;
	}

	ShowWindow(hWindow,SW_SHOW);
	UpdateWindow(hWindow);

	radar->SetHWND(hWindow);
	radar->InitGL();

	while(msg.message != WM_QUIT){
		if(PeekMessage(&msg,NULL,0,0,PM_REMOVE)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
			radar->Draw();
	}
	radar->KillGL();
	delete radar;
	return msg.wParam;
}