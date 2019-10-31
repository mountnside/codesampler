//-----------------------------------------------------------------------------
//           Name: oglu_aa.cpp
//         Author: Simon Flannery (using Kevin Harris' code base).
//  Last Modified: 07/20/06
//    Description: A very simple sample demonstrating how to use the hardware
//                 anti-aliasing capabilities under OpenGL. As you will see,
//                 it is just a simple matter of requesting the correct EXTENDED
//                 pixel format when creating your rendering context device.
//
//                 However, the catch is that (there’s always a catch, right?)
//                 to detect these extended pixel formats that your hardware
//                 might support, you first need to create a standard rendering
//                 context device using a common, supported and known pixel
//                 format – this rendering context device will not support
//                 anti-aliasing and can NOT be edited to support anti-aliasing
//                 (hence the catch). Once you have your standard OpenGL rendering
//                 context device setup you can then query for a pixel format
//                 that supports multiple sampling using the ARB function
//                 "wglChoosePixelFormatARB". Now with this new pixel format,
//                 you need to destroy your first rendering context and window
//                 and create a new window and rendering context with the new
//                 pixel format.
//
//   Control Keys: Space Bar - Toggle AA on and off.
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "wglext.h"
#include "resource.h"

#define GL_MULTISAMPLE_ARB 0x809D /* Ripped from "glext.h" */

struct vertex
{
// GL_T2F_C4UB_V3F
   float u, v;
   unsigned char r, g, b, a;
   float x, y, z;
};

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND  g_hWnd = NULL;
HDC   g_hDC  = NULL;
HGLRC g_hRC  = NULL;

bool g_bAA_support = false, g_bAA = false;

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void init();
void initAA(const int pf); /* New. */
void setup(); /* New. */
void render();
void shutDown();

//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: The application's entry point
// This has been edited to create a basic OpenGL rendering context device,
// query for a pixel format supporting multi-sampling, destroy the first
// rendering context device and create a new OpenGL rendering context device.
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   WNDCLASSEX winClass = {0};
   MSG        uMsg = {0};

   winClass.lpszClassName = "MY_WINDOWS_CLASS";
   winClass.cbSize        = sizeof(WNDCLASSEX);
   winClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
   winClass.lpfnWndProc   = WindowProc;
   winClass.hInstance     = hInstance;
   winClass.hIcon         = LoadIcon(hInstance, (LPCTSTR)IDI_OPENGL_ICON);
   winClass.hIconSm       = LoadIcon(hInstance, (LPCTSTR)IDI_OPENGL_ICON);
   winClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
   winClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
   winClass.lpszMenuName  = NULL;
   winClass.cbClsExtra    = 0;
   winClass.cbWndExtra    = 0;
   
   if (!RegisterClassEx(&winClass))   return E_FAIL;

   g_hWnd = CreateWindowEx(NULL, "MY_WINDOWS_CLASS", 
                          "OpenGL - Dummy",
                          WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                          0, 0, 640, 480, NULL, NULL, hInstance, NULL);

   if (g_hWnd == NULL)   return E_FAIL;

   init();

   PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC) wglGetProcAddress("wglChoosePixelFormatARB"); /* Go get that function pointer. */

   int attributes[] = { /* Basic attributes with multi-sampling support. */
      WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
      WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
      WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
      WGL_COLOR_BITS_ARB, 16,
      WGL_ALPHA_BITS_ARB, 8,
      WGL_DEPTH_BITS_ARB, 16,
      WGL_STENCIL_BITS_ARB, 0,
      WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
      WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
      WGL_SAMPLES_ARB, 4, // Can also be set to 2.
      0,
   };

   int pf = 0; unsigned int n = 0;

   if (wglChoosePixelFormatARB != NULL)
   {
      BOOL bResult = wglChoosePixelFormatARB(g_hDC, attributes, NULL, 1, &pf, &n);

      if (bResult != FALSE && n > 0)
      {
         shutDown(); /* Kill old rendering context and window. */
         g_hWnd = CreateWindowEx(NULL, "MY_WINDOWS_CLASS", /* Create a new window. */
                                 "OpenGL - AA Disabled",
                                 WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                 0, 0, 640, 480, NULL, NULL, hInstance, NULL);

         initAA(pf);
         g_bAA_support = true;
      }
   }

   if (g_bAA_support == false)
   {
      MessageBox(g_hWnd, "Sorry, no suitable pixel format was available supporting hardware AA.", "Error.", MB_OK);
   }

   ShowWindow(g_hWnd, nCmdShow);
   UpdateWindow(g_hWnd);

   setup();
   PostMessage(g_hWnd, WM_SIZE, 0, 29688440); /* Force OpenGL Perspective setup. */

/* And the rest is normal. */
   while (uMsg.message != WM_QUIT)
   {
      if (PeekMessage(&uMsg, NULL, 0, 0, PM_REMOVE))
      {
         TranslateMessage(&uMsg);
         DispatchMessage(&uMsg);
      }
      else render();
   }

   shutDown();

   UnregisterClass("MY_WINDOWS_CLASS", winClass.hInstance);

   return (int) uMsg.wParam;
}

//-----------------------------------------------------------------------------
// Name: WindowProc()
// Desc: Our message handler.
//-----------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   switch (msg)
   {
      case WM_CHAR:
         if (wParam == VK_ESCAPE) /* ESC key. */
         {
            PostQuitMessage(0);
         }

         if (wParam == VK_SPACE) /* The space bar key. */
         {
            g_bAA = !g_bAA;
         }

         break;

      case WM_SIZE:
      { /* Scoping is required here as variables are declared. */
         int nWidth  = LOWORD(lParam); 
         int nHeight = HIWORD(lParam);
         glViewport(0, 0, nWidth, nHeight);

         glMatrixMode(GL_PROJECTION);
         glLoadIdentity();
         gluPerspective(45.0, (GLdouble)nWidth / (GLdouble)nHeight, 0.1, 100.0);
      }
      break;

      case WM_CLOSE:
   // case WM_DESTROY:
         PostQuitMessage(0);
      break;
      
      default:
         return DefWindowProc(hWnd, msg, wParam, lParam);
      break;
   }

   return 0;
}

//-----------------------------------------------------------------------------
// Name: init()
// Desc: This is where we create a "dummy" rendering context.
//-----------------------------------------------------------------------------
void init()
{
   unsigned int pf = 0;

   PIXELFORMATDESCRIPTOR pfd = {0};
// memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

   pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
   pfd.nVersion   = 1;
   pfd.dwFlags    = PFD_DRAW_TO_WINDOW |PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
   pfd.iPixelType = PFD_TYPE_RGBA;
   pfd.cColorBits = 16;
   pfd.cDepthBits = 16;

   g_hDC = GetDC(g_hWnd);
   pf = ChoosePixelFormat(g_hDC, &pfd);
   SetPixelFormat(g_hDC, pf, &pfd);
   g_hRC = wglCreateContext(g_hDC);
   wglMakeCurrent(g_hDC, g_hRC);

   return;
}

//-----------------------------------------------------------------------------
// Name: initAA()
// Desc: This is where we create our AA rendering context.
//-----------------------------------------------------------------------------
void initAA(const int pf)
{
   PIXELFORMATDESCRIPTOR pfd = {0};
// memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

   pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
   pfd.nVersion   = 1;
   pfd.dwFlags    = PFD_DRAW_TO_WINDOW |PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
   pfd.iPixelType = PFD_TYPE_RGBA;
   pfd.cColorBits = 16;
   pfd.cDepthBits = 16;

   g_hDC = GetDC(g_hWnd);
   SetPixelFormat(g_hDC, pf, &pfd);
   g_hRC = wglCreateContext(g_hDC);
   wglMakeCurrent(g_hDC, g_hRC);

   return;
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: Delete and release rendering context devices.
//-----------------------------------------------------------------------------
void shutDown()   
{
   if (g_hRC != NULL)
   {
      wglMakeCurrent(NULL, NULL);
      wglDeleteContext(g_hRC);
      g_hRC = NULL;
   }

   if (g_hDC != NULL)
   {
      ReleaseDC(g_hWnd, g_hDC);
      g_hDC = NULL;
   }

   if (g_hWnd != NULL) /* New. */
   {
      DestroyWindow(g_hWnd);
      g_hWnd = NULL;
   }

   return;
}

//-----------------------------------------------------------------------------
// Name: setup()
// Desc: Setup the OpenGL environment - just the clear color.
//-----------------------------------------------------------------------------
void setup()
{
   glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

   return;
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: Render a basic spinning triangle.
//-----------------------------------------------------------------------------
void render()
{
   static float xRotateTriangle = 0.0f;

   static vertex triangle[] =
   {
      {0.5f, 1.0f, 255,   0,   0, 255,  0.0f,  1.0f, 0.0f},
      {0.0f, 0.0f,   0, 255,   0, 255, -1.0f, -1.0f, 0.0f},
      {1.0f, 0.0f,   0,   0, 255, 255,  1.0f, -1.0f, 0.0f}
   };

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   if (g_bAA_support != false)
   {
      if (g_bAA != false) /* Something new. */
      {
         glEnable(GL_MULTISAMPLE_ARB);
         SetWindowText(g_hWnd, "OpenGL - AA Enabled");
      }
      else
      {
         glDisable(GL_MULTISAMPLE_ARB);
         SetWindowText(g_hWnd, "OpenGL - AA Disabled");
      }
   }

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslatef(0.0f, 0.0f, -4.0f);
   glRotatef(xRotateTriangle, 0.0f, 1.0f, 0.0f);

   glInterleavedArrays(GL_T2F_C4UB_V3F, 0, triangle);
   glDrawArrays(GL_TRIANGLES, 0, 3);

   SwapBuffers(g_hDC);

   xRotateTriangle = xRotateTriangle + 0.2f;

   return;
}
