/* File: oglu_cel_shading.cpp; Mode: C++; Tab-width: 3; Author: Simon Flannery; */

//------------------------------------------------------------------------------
//           Name: oglu_cel_shading.cpp
//         Author: Simon Flannery (simon.flannery@bigpond.com)
//  Last Modified: 03/18/06
//    Description: An example of cel shading with OpenGL using a vertex program.
//
//                 The notorious Teapot is used to demonstrate the simple 
//                 technique of Cel-shading (or toon rendering). This involves
//                 creating a lookup table and calculating the inverse model
//                 matrix to convert the light position into model space.
//
//   Control Keys: Left Mouse Button - Spin the Teapot.
//                 Arrow keys translate the (point) light source.
//------------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>   /* Must Include before GL/gl.h. */
#include <stdio.h>
#include <stdlib.h>
#include "glew.h"      /* OpenGL. */
#include "Teapot.h"
#include "Shader.h"

enum {x, y, z, w};

//------------------------------------------------------------------------------
// SETUP A LITTLE MATH INFRASTRUCTURE
//------------------------------------------------------------------------------
struct vector4f
{
   float m[4];

   float& operator[](int i)
   { 
      return m[i];
   }

   float operator[](int i) const
   { 
      return m[i];
   }
};

struct matrix16f
{
   float m[16];

   operator float* () const
   {
      return (float*) this;
   }

   operator const float* () const
   {
      return (const float*) this;
   }
};

vector4f operator*(const matrix16f& mm, const vector4f& rhs)
{
   vector4f v;

   v[x] = mm.m[0] * rhs[x] + mm.m[4] * rhs[y] + mm.m[ 8] * rhs[z] + mm.m[12] * rhs[w];
   v[y] = mm.m[1] * rhs[x] + mm.m[5] * rhs[y] + mm.m[ 9] * rhs[z] + mm.m[13] * rhs[w];
   v[z] = mm.m[2] * rhs[x] + mm.m[6] * rhs[y] + mm.m[10] * rhs[z] + mm.m[14] * rhs[w];
   v[w] = mm.m[3] * rhs[x] + mm.m[7] * rhs[y] + mm.m[11] * rhs[z] + mm.m[15] * rhs[w];

   return v;
}

//------------------------------------------------------------------------------
// GLOBALS
//------------------------------------------------------------------------------
float g_mouse_x = 45.0f, g_mouse_y = -45.0f;

unsigned int g_id_cel_brightness = 0;

vector4f g_light_position = {5.0f, 5.0f, 5.0f, 0.0f};

Shader my_shader;

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
HWND BuildWindow(LPCTSTR, HINSTANCE, int, int);
BOOL HeartBeat(HDC);

void Initialize();
void ShutDown();
void ReshapeScene(int, int);
void IllustrateScene();

//------------------------------------------------------------------------------
// The application's entry point.
//------------------------------------------------------------------------------
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   HWND hwnd = NULL;
   if ((hwnd = BuildWindow("OpenGL32 Cel-Shaded Teapot Using a vertex program.", hInstance, 640, 480)) != NULL)
   {
      HDC   hdc = GetDC(hwnd);
      HGLRC hrc = wglCreateContext(hdc);
      wglMakeCurrent(hdc, hrc);

      glewInit(); /* Very little error checking is done here. */

      if (GLEW_ARB_vertex_program != false)
      {
         Initialize();

         ShowWindow(hwnd, nCmdShow);
         HeartBeat(hdc);

         ShutDown();
      }
      else
      {
         MessageBox(NULL, "Sorry. One or more GL_ARB_vertex_program functions were not detected.", "Error", MB_OK | MB_ICONEXCLAMATION);
      }

      wglMakeCurrent(hdc, NULL);
      wglDeleteContext(hrc);
      ReleaseDC(hwnd, hdc);

      DestroyWindow(hwnd);
   }

   return 0;
}

//------------------------------------------------------------------------------
// Message handler for our window.
//------------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   static POINT last_mouse = {0, 0};
   static POINT current_mouse = {0, 0};
   static bool  bXYMousing = false;

   switch (msg)
   {
      case WM_SIZE:
         ReshapeScene(LOWORD(lParam), HIWORD(lParam));
         return 0;
         break;

      case WM_MOUSEMOVE:
         current_mouse.x = LOWORD (lParam);
         current_mouse.y = HIWORD (lParam);

         if (bXYMousing != false)
         {
            g_mouse_x -= (current_mouse.x - last_mouse.x);
            g_mouse_y -= (current_mouse.y - last_mouse.y);
         }

         last_mouse.x = current_mouse.x;
         last_mouse.y = current_mouse.y;
      break;

      case WM_LBUTTONDOWN:
         last_mouse.x = current_mouse.x = LOWORD (lParam);
         last_mouse.y = current_mouse.y = HIWORD (lParam);
         bXYMousing = true;
      break;

      case WM_LBUTTONUP:
         bXYMousing = false;
      break;

      case WM_KEYDOWN:
         switch (wParam)
         {
            case 38: /* Up Arrow Key. */
               g_light_position[y] += 0.1f;
               break;

            case 40: /* Down Arrow Key. */
               g_light_position[y] -= 0.1f;
               break;

            case 37: /* Left Arrow Key. */
               g_light_position[x] -= 0.1f;
               break;

            case 39: /* Right Arrow Key. */
               g_light_position[x] += 0.1f;
               break;
         }
      break;

      case WM_CHAR:
         if (wParam == VK_ESCAPE) /* ESC key. */
         {
            PostQuitMessage(0);
         }

         return 0;
         break;

      case WM_CLOSE:
          PostQuitMessage(0);
          return 0;
          break;
   }

   return DefWindowProc(hwnd, msg, wParam, lParam);
}

//------------------------------------------------------------------------------
// Registers the type of window and then creates the window. The pixel format
// is also described.
//------------------------------------------------------------------------------
HWND BuildWindow(LPCTSTR szTitle, HINSTANCE hInstance, int nWidth, int nHeight)
{
   int pf = 0;
   HDC hdc   = NULL;
   HWND hwnd = NULL;

   WNDCLASS wndClass = { CS_HREDRAW | CS_VREDRAW | CS_OWNDC, WindowProc, 0, 0, hInstance,
                         LoadIcon(NULL, IDI_APPLICATION),
                         LoadCursor(NULL, IDC_ARROW),
                         NULL, NULL, "OpenGL" };

   PIXELFORMATDESCRIPTOR pfd = {0};

   RegisterClass(&wndClass);

   hwnd = CreateWindow("OpenGL", szTitle,
                       WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                       CW_USEDEFAULT, CW_USEDEFAULT,
                       nWidth, nHeight,
                       NULL, NULL,
                       hInstance, NULL);

   hdc = GetDC(hwnd);

   memset(&pfd, 0, sizeof(pfd));
   pfd.nSize      = sizeof(pfd);
   pfd.nVersion   = 1;
   pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
   pfd.iPixelType = PFD_TYPE_RGBA;
   pfd.cDepthBits = 32;
   pfd.cColorBits = 32;

   pf = ChoosePixelFormat(hdc, &pfd);
   SetPixelFormat(hdc, pf, &pfd);
// DescribePixelFormat(hdc, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

   ReleaseDC(hwnd, hdc);

   return hwnd;
}

//------------------------------------------------------------------------------
// Responsible for the message loop. Renders scene and swaps buffer each iteration.
//------------------------------------------------------------------------------
BOOL HeartBeat(HDC hdc)
{
   MSG msg   = {0};    /* Message. */
   BOOL bActive = TRUE, bMsg = FALSE;

   while (msg.message != WM_QUIT)
   {
      if (bActive != FALSE)
      {
         bMsg = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
      }
      else
      {
         bMsg = GetMessage(&msg, NULL, 0, 0);
      }
      
      if (bMsg != FALSE)
      {
         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }

      IllustrateScene();
      SwapBuffers(hdc);
   }

   return TRUE;
}

//------------------------------------------------------------------------------
// Sets up the required openGL environment and shader.
//------------------------------------------------------------------------------
void Initialize()
{
   glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
   glShadeModel(GL_SMOOTH);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_CULL_FACE);
   glCullFace(GL_FRONT);
   glPointSize(8.0f);
   glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

/* Load in our shader. */
   my_shader.Load("cel.txt");

/* Create our lookup table. */
	unsigned char BrightnessData[16] = { 127, 127, 127, 191, 
                                        191, 191, 191, 191,
                                        255, 255, 255, 255,
                                        255, 255, 255, 255 };
   glGenTextures(1, &g_id_cel_brightness);
   glBindTexture(GL_TEXTURE_1D, g_id_cel_brightness);
   glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
/* Commit the lookup table to memory. */
   glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 16, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, BrightnessData);

   return;
}

//------------------------------------------------------------------------------
// Clean up the texture and shader.
//------------------------------------------------------------------------------
void ShutDown()
{
   glDeleteTextures(1, &g_id_cel_brightness);

   my_shader.Delete();

   return;
}

//------------------------------------------------------------------------------
// Resize and Initialize the openGL environment.
//------------------------------------------------------------------------------
void ReshapeScene(int nWidth, int nHeight)
{
   if (nHeight == 0)
   {
      nHeight = 1;
   }

   glViewport(0, 0, nWidth, nHeight);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(45.0, (double) nWidth / (double) nHeight, 0.1, 100.0);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   return;
}

//------------------------------------------------------------------------------
// Renders the scene of our Teapot.
//------------------------------------------------------------------------------
void IllustrateScene()
{
   matrix16f inverse_model_matrix;

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glLoadIdentity();
   glTranslatef(0.0f, 0.0f, -40.0f);

/* Calculate the inverse matrix - the lazy way. */
/* We do the view model setup operations in reverse order! */
   glPushMatrix();
   glLoadIdentity();
   glRotatef(g_mouse_x, 0.0f, 1.0f, 0.0f);
   glRotatef(g_mouse_y, 1.0f, 0.0f, 0.0f);
   glGetFloatv(GL_MODELVIEW_MATRIX, inverse_model_matrix);
   glPopMatrix();

/* Use the inverse model matrix to convert light position into model space. */
   vector4f model_space_light_position = inverse_model_matrix * g_light_position;

   glPushMatrix();
   glRotatef(-g_mouse_y, 1.0f, 0.0f, 0.0f);
   glRotatef(-g_mouse_x, 0.0f, 1.0f, 0.0f);

   glColor4f(0.75f, 0.95f, 1.0f, 1.0f);

/* Shade our beautiful Teapot. */
   glBindTexture(GL_TEXTURE_1D, g_id_cel_brightness);
   glEnable(GL_TEXTURE_1D); /* Enable our lookup table. */
   my_shader.Enable();
   my_shader.Bind();
/* Tell the shader where our light is in model space.*/
   my_shader.EnvParameter(0, model_space_light_position[x], model_space_light_position[y], model_space_light_position[z], 1.0f);
   RenderSolidTeapot(8.0);
   my_shader.Disable();
   glDisable(GL_TEXTURE_1D);

/* Outline our Teapot. */
   glLineWidth(3.0f);
   glCullFace(GL_BACK);
   glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
   glPolygonMode(GL_FRONT, GL_LINE);
   RenderSolidTeapot(8.0);
   glPolygonMode(GL_FRONT, GL_FILL);
   glCullFace(GL_FRONT);

   glPopMatrix();

/* Render the light source position. */
   glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
   glPushMatrix();
   glTranslatef(g_light_position[x], g_light_position[y], g_light_position[z]);
   glBegin(GL_POINTS);
      glVertex3f(0.0f, 0.0f, 0.0f);
   glEnd();
   glPopMatrix();

   return;
}
