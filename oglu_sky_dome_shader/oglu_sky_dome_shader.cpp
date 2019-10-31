/* File: oglu_sky_dome_shader.cpp; Mode: C++; Tab-width: 3; Author: Simon Flannery; */

//------------------------------------------------------------------------------
//           Name: oglu_sky_dome_shader.cpp
//         Author: Simon Flannery (simon.flannery@bigpond.com)
//  Last Modified: 04/04/06
//    Description: Demonstrates the use of Shaders to give new life to
//                 the tried old sky dome.
//
//                 Using an offline patch of Perlin noise to represent a cloudy
//                 sky, it is very easy to use vertex shaders to auto generate
//                 texture co-ordinates with an offset to scroll the noise patch
//                 across a sky dome.
//
//                 This allows us to cheaply create an animated sky dome. Also, 
//                 to control the "look and feel" of the sky we can adjust the
//                 tint coloring and scroll speed. For the tint coloring an
//                 expontenial model was used.
//
//                 Full credit for the offline patch of Perlin noise goes to NVIDIA (I think).
//
//   Control Keys: e - decrease the RED tinting component.
//                 r - increase the RED tinting component.
//                 f - decrease the GREEN tinting component.
//                 g - increase the GREEN tinting component.
//                 v - decrease the BLUE tinting component.
//                 b - increase the BLUE tinting component.
//                 - - decrease the scroll speed.
//                 = - increase the scroll speed.
//                 Left mouse button performs looking.
//------------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#define _CRT_SECURE_NO_DEPRECATE /* Stop MS VS 2005 from advertising their new
                                    and more secure CRT library! */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <windows.h>   /* Must Include before GL/gl.h. */
#include "glew.h"
#include <GL/glu.h>    /* OpenGL utilities. */
#include <GL/glaux.h>
#include "Model.h"
#include "Shader.h"

//------------------------------------------------------------------------------
// SETUP A LITTLE MATH INFRASTRUCTURE
//------------------------------------------------------------------------------
struct vector3f
{
   float m[3];

   float& operator[](int i)
   { 
      return m[i];
   }

   float operator[](int i) const
   { 
      return m[i];
   }

   float Length() const
   {
      return sqrt(m[x] * m[x] + m[y] * m[y] + m[z] * m[z]);
   }

   vector3f& Normalize()
   {
      float length = Length();
      m[x] = m[x] / length;
      m[y] = m[y] / length;
      m[z] = m[z] / length;

      return *this;
   }
};

vector3f operator + (const vector3f& a, const vector3f& b)
{
   vector3f v;

   v[x] = a[x] + b[x];
   v[y] = a[y] + b[y];
   v[z] = a[z] + b[z];

   return v;
}

vector3f operator - (const vector3f& a, const vector3f& b)
{
   vector3f v;

   v[x] = a[x] - b[x];
   v[y] = a[y] - b[y];
   v[z] = a[z] - b[z];

   return v;
}

vector3f Cross(const vector3f& a, const vector3f& b)
{
   vector3f v;

   v[x] = (a[y] * b[z]) - (a[z] * b[y]);
   v[y] = (a[z] * b[x]) - (a[x] * b[z]);
   v[z] = (a[x] * b[y]) - (a[y] * b[x]);

   return v;
}

unsigned int g_cloud_texture = 0;
Model_sgl g_sky_model;
Shader g_sky_shader;

float g_tint[4] = {0.9f, 0.7f, 0.7f, 1.0f};
float g_scroll = 1.0f;

bool g_mousing = false;
float g_mouse_x = 0.0f, g_mouse_y = 0.0f;

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
HWND BuildWindow(LPCTSTR, HINSTANCE, int, int);
BOOL HeartBeat(HDC);

vector3f Rotate(float, vector3f&, vector3f&);
void Looking(float, float, vector3f&, vector3f&, vector3f&);

void Initialize();
void ShutDown();
void ReshapeScene(int, int);
void RenderBars();
void IllustrateScene();

//------------------------------------------------------------------------------
// The application entry point.
//------------------------------------------------------------------------------
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   HWND hwnd = NULL;   /* Window Handle. */

   if ((hwnd = BuildWindow("OpenGLSL Sky Dome.", hInstance, 640, 480)) != NULL)
   {
      HDC   hdc = GetDC(hwnd);
      HGLRC hrc = wglCreateContext(hdc);
      wglMakeCurrent(hdc, hrc);

      glewInit();

      if (GLEW_ARB_shader_objects != false)
      {
         Initialize();

         ShowWindow(hwnd, nCmdShow);
         HeartBeat(hdc);

         ShutDown();
      }
      else
      {
         MessageBox(NULL, "Sorry. One or more GL_ARB_shader_objects functions were not detected.", "Sky Dome Error", MB_OK | MB_ICONEXCLAMATION);
      }

      wglMakeCurrent(hdc, NULL);
      wglDeleteContext(hrc);
      ReleaseDC(hwnd, hdc);

      DestroyWindow(hwnd);
   }

   return 0;
}

//------------------------------------------------------------------------------
// Message handler for the window.
//------------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   static POINT last_mouse = {0, 0};
   static POINT current_mouse = {0, 0};

   switch (msg)
   {
      case WM_SIZE:
         ReshapeScene(LOWORD(lParam), HIWORD(lParam));
         return 0;
         break;

      case WM_CHAR:
         if (wParam == VK_ESCAPE) /* ESC key. */
         {
            PostQuitMessage(0);
         }

         switch (wParam) /* Control the tinting and scroll speed. */
         {
         case 'r':
            if (g_tint[0] < 1.0f) {   g_tint[0] = g_tint[0] + 0.1f;   }
            break;
         case 'e':
            if (g_tint[0] > 0.0f) {   g_tint[0] = g_tint[0] - 0.1f;   }
            break;
         case 'g':
            if (g_tint[1] < 1.0f) {   g_tint[1] = g_tint[1] + 0.1f;   }
            break;
         case 'f':
            if (g_tint[1] > 0.0f) {   g_tint[1] = g_tint[1] - 0.1f;   }
            break;
         case 'b':
            if (g_tint[2] < 1.0f) {   g_tint[2] = g_tint[2] + 0.1f;   }
            break;
         case 'v':
            if (g_tint[2] > 0.0f) {   g_tint[2] = g_tint[2] - 0.1f;   }
            break;
         case '-':
            if (g_scroll > -0.2f) {   g_scroll = g_scroll - 0.1f;   }
            break;
         case '=':
            if (g_scroll < 1.0f) {   g_scroll = g_scroll + 0.1f;   }
            break;
         }

         return 0;
         break;

      case WM_MOUSEMOVE:
         current_mouse.x = LOWORD(lParam);
         current_mouse.y = HIWORD(lParam);

         if (g_mousing != false)
         {
            g_mouse_x -= ((float) (current_mouse.x - last_mouse.x)) / 5000.0f;
            g_mouse_y -= ((float) (current_mouse.y - last_mouse.y)) / 5000.0f;
         }

         last_mouse.x = current_mouse.x;
         last_mouse.y = current_mouse.y;
         break;

      case WM_LBUTTONDOWN:
         g_mouse_x = g_mouse_y = 0.0f;
         last_mouse.x = current_mouse.x = LOWORD(lParam);
         last_mouse.y = current_mouse.y = HIWORD(lParam);
         g_mousing = true;
      break;

      case WM_LBUTTONUP:
         g_mousing = false;
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
   MSG msg = {0};    /* Message. */
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
// Rotate around an axis given a looking direction.
//------------------------------------------------------------------------------
vector3f Rotate(float angle, vector3f& axis, vector3f& direction)
{
   vector3f view;

   float cos_theta = cosf(angle);
   float sin_theta = sinf(angle);

   view[x]  = (cos_theta + (1.0f - cos_theta) * axis[x] * axis[x])           * direction[x];
   view[x] += ((1.0f - cos_theta) * axis[x] * axis[y] - axis[z] * sin_theta) * direction[y];
   view[x] += ((1.0f - cos_theta) * axis[x] * axis[z] + axis[y] * sin_theta) * direction[z];
   view[y]  = ((1.0f - cos_theta) * axis[x] * axis[y] + axis[z] * sin_theta) * direction[x];
   view[y] += (cos_theta + (1.0f - cos_theta) * axis[y] * axis[y])           * direction[y];
   view[y] += ((1.0f - cos_theta) * axis[y] * axis[z] - axis[x] * sin_theta) * direction[z];
   view[z]  = ((1.0f - cos_theta) * axis[x] * axis[z] - axis[y] * sin_theta) * direction[x];
   view[z] += ((1.0f - cos_theta) * axis[y] * axis[z] + axis[x] * sin_theta) * direction[y];
   view[z] += (cos_theta + (1.0f - cos_theta) * axis[z] * axis[z])           * direction[z];

   return view;
}

//------------------------------------------------------------------------------
// Perform Simple Looking.
//------------------------------------------------------------------------------
void Looking(float rotate_x, float rotate_y, vector3f& eye, vector3f& look, vector3f& up)
{
   static float current_x_rotation  = 0.0f;
   static float previous_x_rotation = 0.0f; 

   previous_x_rotation = current_x_rotation;
   current_x_rotation  = current_x_rotation + rotate_y;

   vector3f view = {0};
   vector3f direction = look - eye;

   vector3f axis = Cross(direction, up);
            axis.Normalize();

   if (current_x_rotation > 1.0f)     
   {
      current_x_rotation = 1.0f;

      if (previous_x_rotation != 1.0f)
      {
         look = eye + Rotate(1.0f - previous_x_rotation, axis, direction);
      }
   }
   else if (current_x_rotation < -1.0f)
   {
      current_x_rotation = -1.0f;

      if (previous_x_rotation != -1.0f)
      {
         look = eye + Rotate(-1.0f - previous_x_rotation, axis, direction);
      }
   }
   else 
   {    
      look = eye + Rotate(rotate_y, axis, direction);
   }

   direction = look - eye;
   look = eye + Rotate(rotate_x, up, direction);

   return;
}

//------------------------------------------------------------------------------
// Sets up the openGL environment including loading the shader, 
// sky dome model and the Perlin noise patch (the texture).
//------------------------------------------------------------------------------
void Initialize()
{
   glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
   glShadeModel(GL_SMOOTH);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_CULL_FACE);
   glCullFace(GL_BACK);
   glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

   glLineWidth(3.0f);

   float diffuse[]  = {1.0f, 1.0f, 1.0f, 1.0f};
   float position[] = {0.0f, 2.0f, 0.0f, 1.0f};
   float specular[] = {1.0f, 1.0f, 1.0f, 1.0f};

   glEnable(GL_LIGHTING);
   glEnable( GL_LIGHT0);
   glLightfv(GL_LIGHT0, GL_DIFFUSE,  diffuse);
   glLightfv(GL_LIGHT0, GL_POSITION, position);
   glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

   float shininess = 5.0f;
   glEnable(GL_COLOR_MATERIAL);
   glMaterialf( GL_FRONT, GL_SHININESS, shininess);
   glMaterialfv(GL_FRONT, GL_SPECULAR,  specular);

/* Load the cloud texture. */
   AUX_RGBImageRec* pTextureImage = auxDIBImageLoad(".\\cloud.bmp");

   if (pTextureImage != NULL)
   {
      glGenTextures(1, &g_cloud_texture);
      glBindTexture(GL_TEXTURE_2D, g_cloud_texture);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D ,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D, 0, 3, pTextureImage->sizeX, pTextureImage->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, pTextureImage->data);
   }

   if (pTextureImage != NULL)
   {
      if (pTextureImage->data != NULL)
      {
         free(pTextureImage->data);
      }

      free(pTextureImage);
   }

   g_sky_model.Load("sky.sgl");
   g_sky_model.Unitize();
/* Get the bounding box of the sky dome. */
   float bbox[3] = {0};
   g_sky_model.GetDimensions(bbox[0], bbox[1], bbox[2]);

   g_sky_shader.Load("sky.vertex", "sky.fragment");
/* Tell the shader the bounding box of the sky dome. */
   g_sky_shader.Bind("bbox", bbox, 4);

   return;
}

//------------------------------------------------------------------------------
// OpenGL Cleanup of the texture, model and shader.
//------------------------------------------------------------------------------
void ShutDown()
{
   glDeleteTextures(1, &g_cloud_texture);
   g_sky_model.Delete();
   g_sky_shader.Delete();

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
   gluPerspective(45.0f, (float) nWidth / (float) nHeight, 0.1f, 100.0f);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   return;
}

//------------------------------------------------------------------------------
// Render feedback bars for each for the R, G and B color channels and the 
// scroll speed (black).
//------------------------------------------------------------------------------
void RenderBars()
{
/* Push back the current matrices and go orthographic mode. */
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   glOrtho(0, 256.0, 256.0, 0, -1, 1);

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();

   glDisable(GL_LIGHTING);
/* Render the feedback bars. */
   glBegin(GL_LINES);
   glColor3f(1.0f, 0.0f, 0.0f);
   glVertex2f(10.0f, 10.0f);
   glVertex2f(10.0f + g_tint[0] * 40.0f, 10.0f);
   glColor3f(0.0f, 1.0f, 0.0f);
   glVertex2f(10.0f, 20.0f);
   glVertex2f(10.0f + g_tint[1] * 40.0f, 20.0f);
   glColor3f(0.0f, 0.0f, 1.0f);
   glVertex2f(10.0f, 30.0f);
   glVertex2f(10.0f + g_tint[2] * 40.0f, 30.0f);
   glColor3f(0.0f, 0.0f, 0.0f);
   glVertex2f(10.0f, 40.0f);
   glVertex2f(10.0f + g_scroll * 40.0f, 40.0f);
   glEnd();
   glEnable(GL_LIGHTING);

/* Pop the matrices back. */
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();

   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();

   return;
}

//------------------------------------------------------------------------------
// Renders the scene - The sky dome is rendered first, then the foreground.
//------------------------------------------------------------------------------
void IllustrateScene()
{
   static float time = 0.0f;
   static vector3f eye = {0.0f, 0.0f, 0.0f}, look = {0.0f, 0.0f, 1.0f}, up = {0.0f, 1.0f, 0.0f};

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glLoadIdentity();

   if (g_mousing != false)
   {
      Looking(g_mouse_x, g_mouse_y, eye, look, up);
   }

   gluLookAt(eye[x], eye[y], eye[z], look[x], look[y], look[z], up[x], up[y], up[z]);

/* Turn off the depth buffer when rendering the background. */
   glDisable(GL_DEPTH_TEST);

/* Update the timer for scrolling the clouds. */
   time = time + ((1.0f / 2000.0f) * g_scroll);

   g_sky_shader.Enable();
/* Select the shader program and update parameters. */
/* The "time" parameter controls the cloud scrolling. */
   g_sky_shader.Bind("time", &time, 1);
/* The "horizon" parameter controls the sky tinting. */
   g_sky_shader.Bind("horizon", g_tint, 4);

   glActiveTexture(GL_TEXTURE0 + 0);
   glActiveTexture(GL_TEXTURE0 + 1);
/* Load the cloud texture. */
   glBindTexture(GL_TEXTURE_2D, g_cloud_texture);

/* Render the sky dome. */
   g_sky_model.Render();

/* Unselect the shader. */
   g_sky_shader.Disable();

/* Turn on the depth buffer when rendering the foreground. */
   glEnable(GL_DEPTH_TEST);

/* Render the forground, for example, a teapot or bunny. */
   glEnable(GL_LIGHTING);
   glColor3f(0.0f, 1.0f, 0.0f);
   glBegin(GL_QUADS);
   glNormal3f( 0.0f,  1.0f,  0.0f);
   glVertex3f( 20.0f, -1.0f,  20.0f);
   glVertex3f( 20.0f, -1.0f, -20.0f);
   glVertex3f(-20.0f, -1.0f, -20.0f);
   glVertex3f(-20.0f, -1.0f,  20.0f);
   glEnd();
   glDisable(GL_LIGHTING);

   RenderBars();

   return;
}
