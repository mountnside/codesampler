/* File: oglu_hdr.cpp; Mode: C++; Tab-width: 3; Author: Simon Flannery;       */

//------------------------------------------------------------------------------
//          Name: oglu_hdr.cpp
//        Author: Simon Flannery (simon.flannery.nospam@gmail.com)
// Last Modified: 11/13/2007
//   Description: High Dynamic Range rendering, or HDR for short, is
//                a lighting procedure designed to emulate, and capture, the 
//                increased lighting levels and contrast ratios experienced in 
//                the real world. Rendering HDR imaging involves the use
//                of a wider dynamic range than usual. More simply, each pixel
//                is not limited to the range [0, 1]. However, since traditional
//                display devices are still bounded by [0, 1] we use the concept
//                of Tone Mapping to allow a HDR color to be mapped to Low
//                Dynamic Range in order to be displayed.
//
//                To implement HDR imaging, the following technology is
//                required:
//                1. The ability to render to an off-screen surface, for example
//                   render to a frame buffer object (FBO)
//                2. Shading language support, for example OpenGL 2.0
//                3. Non-power of 2, NPOT, texture support is really useful 
//                   (not all cards have support for this)
//
//                The following procedure is used to implement this simple
//                sample:
//                1. Render the scene to a master texture / frame buffer 
//                   object (FBO)
//                2. High pass filter the master FBO
//                3. Down sample the scene by rendering the High pass filter
//                   FBO to a set of successively smaller FBO's
//                4. Perform simple blurring on the downsampled FBO's
//                5. Render the final frame by combining all the downsampled
//                   FBO's
//
// The theory from the following website was useful:
// http://www.prideout.net/bloom/index.php
//------------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <math.h>
#include <windows.h>   // Must Include before GL/gl.h.
#include "glew.h"      // OpenGL.
#include "Teapot.h"

#ifndef GL_SHADING_LANGUAGE_VERSION
#define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#endif

#define WIDTH  800
#define HEIGHT 600

#define MAX_COLORS 5
#define EPSILON 0.001f

enum {x, y, z, w};
enum direction {HORIZONTAL, VERTICAL};

typedef float color[3];

//------------------------------------------------------------------------------
// GLOBALS
//------------------------------------------------------------------------------
float g_mouse_x = 140.0f, g_mouse_y = -20.0f, g_mouse_z = 30.0f;

unsigned int my_main_texture = 0, my_main_depth = 0, my_main_fbo = 0;
unsigned int my_pass_texture[4] = {0}, my_pass_fbo[4] = {0};
unsigned int my_swap_texture[4] = {0}, my_swap_fbo[4] = {0};

unsigned int my_shader = 0, my_combine = 0, my_high_pass = 0, my_blur = 0;

//------------------------------------------------------------------------------
// PROTOTYPES
//------------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
HWND BuildWindow(LPCTSTR, HINSTANCE, int, int);
BOOL HeartBeat(HWND, HDC);
unsigned char* ReadShaderFile(const char* szFileName);

void Initialize();
void ShutDown();
void ReshapeScene(int, int);
void Blur(unsigned int, unsigned int, direction dirn);
void Render();

//------------------------------------------------------------------------------
// The application's entry point.
//------------------------------------------------------------------------------
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   HWND hWnd = NULL;
   if ((hWnd = BuildWindow("OpenGLSL HDR", hInstance, WIDTH, HEIGHT)) != NULL)
   {
      HDC   hdc = GetDC(hWnd);
      HGLRC hrc = wglCreateContext(hdc);
      wglMakeCurrent(hdc, hrc);

      glewInit(); // Very little error checking is done here.

      if (GLEW_ARB_shader_objects != false)
      {
         Initialize();

         ShowWindow(hWnd, nCmdShow);
         HeartBeat(hWnd, hdc);

         ShutDown();
      }
      else
      {
         MessageBox(NULL, "Sorry. One or more GL_ARB_shader_objects functions were not detected.", "Error", MB_OK | MB_ICONEXCLAMATION);
      }

      wglMakeCurrent(hdc, NULL);
      wglDeleteContext(hrc);
      ReleaseDC(hWnd, hdc);

      DestroyWindow(hWnd);
   }

   return 0;
}

//------------------------------------------------------------------------------
// Message handler for our window.
//------------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   static POINT last_mouse = {0, 0};
   static POINT current_mouse = {0, 0};
   static bool  bXYMousing = false;
   static bool  bZMousing  = false;

   switch (msg)
   {
      case WM_SIZE:
         ReshapeScene(LOWORD(lParam), HIWORD(lParam));
         return 0;
      break;

      case WM_MOUSEMOVE:
         current_mouse.x = LOWORD(lParam);
         current_mouse.y = HIWORD(lParam);

         if (bXYMousing != false)
         {
            g_mouse_x -= (current_mouse.x - last_mouse.x);
            g_mouse_y -= (current_mouse.y - last_mouse.y);
         }

         if (bZMousing != false)
         {
            g_mouse_z -= (current_mouse.y - last_mouse.y);
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

      case WM_RBUTTONDOWN:
         last_mouse.x = current_mouse.x = LOWORD(lParam);
         last_mouse.y = current_mouse.y = HIWORD(lParam);
         bZMousing = true;
      break;

      case WM_RBUTTONUP:
         bZMousing = false;
      break;

      case WM_CHAR:
         if (wParam == VK_ESCAPE) // ESC key.
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

   return DefWindowProc(hWnd, msg, wParam, lParam);
}

//------------------------------------------------------------------------------
// Registers the type of window and then creates the window. The pixel format
// is also described.
//------------------------------------------------------------------------------
HWND BuildWindow(LPCTSTR szTitle, HINSTANCE hInstance, int nWidth, int nHeight)
{
   int pf = 0;
   HDC hdc   = NULL;
   HWND hWnd = NULL;

   WNDCLASS wndClass = { CS_HREDRAW | CS_VREDRAW | CS_OWNDC, WindowProc, 0, 0, hInstance,
                         LoadIcon(NULL, IDI_APPLICATION),
                         LoadCursor(NULL, IDC_ARROW),
                         NULL, NULL, "OpenGL" };

   PIXELFORMATDESCRIPTOR pfd = {0};

   RegisterClass(&wndClass);

   hWnd = CreateWindow("OpenGL", szTitle,
                       WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZEBOX | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                       CW_USEDEFAULT, CW_USEDEFAULT,
                       nWidth, nHeight,
                       NULL, NULL,
                       hInstance, NULL);

   hdc = GetDC(hWnd);

   memset(&pfd, 0, sizeof(pfd));
   pfd.nSize      = sizeof(pfd);
   pfd.nVersion   = 1;
   pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
   pfd.iPixelType = PFD_TYPE_RGBA;
   pfd.iLayerType = PFD_MAIN_PLANE;
   pfd.cColorBits = 24;

   pf = ChoosePixelFormat(hdc, &pfd);
   SetPixelFormat(hdc, pf, &pfd);
// DescribePixelFormat(hdc, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

   ReleaseDC(hWnd, hdc);

   return hWnd;
}

//------------------------------------------------------------------------------
// Responsible for the message loop. Renders scene and swaps buffer each
// iteration.
//------------------------------------------------------------------------------
BOOL HeartBeat(HWND hWnd, HDC hdc)
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

      bActive = !IsIconic(hWnd);

      if (bMsg != FALSE)
      {
         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }

      Render();
      SwapBuffers(hdc);
   }

   return TRUE;
}

//------------------------------------------------------------------------------
// Load a Shader from a file.
//------------------------------------------------------------------------------
unsigned char* ReadShaderFile(const char* szFileName)
{
   FILE *file = fopen(szFileName, "r");

   if (file == NULL)
   {
      MessageBox(NULL, "Cannot open shader file!", "ERROR", MB_OK | MB_ICONEXCLAMATION);
      
      return 0;
   }

   struct _stat fileStats;

   if (_stat(szFileName, &fileStats) != 0)
   {
      MessageBox(NULL, "Cannot get file stats for shader file!", "ERROR", MB_OK | MB_ICONEXCLAMATION);
   
      return 0;
   }

   unsigned char* buffer = new unsigned char[fileStats.st_size];

   size_t bytes = fread(buffer, 1, fileStats.st_size, file);

   buffer[bytes] = 0;

   fclose(file);

   return buffer;
}

//------------------------------------------------------------------------------
// Sets up the required OpenGL environment and shaders.
//------------------------------------------------------------------------------
void Initialize()
{
   glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
   glEnable(GL_CULL_FACE);
   glCullFace(GL_FRONT);
   glShadeModel(GL_SMOOTH);
   glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

   srand((unsigned int) time(NULL));

   glGenTextures(1, &my_main_texture);
   glBindTexture(GL_TEXTURE_2D, my_main_texture);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, WIDTH, HEIGHT, 0, GL_RGBA, GL_HALF_FLOAT_ARB, 0);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
   glBindTexture(GL_TEXTURE_2D, 0);

   glGenRenderbuffersEXT(1, &my_main_depth);
   glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, my_main_depth);
   glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, WIDTH, HEIGHT);

   glGenFramebuffersEXT(1, &my_main_fbo);
   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, my_main_fbo);
   glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, my_main_texture, 0);
   glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, my_main_depth);
   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

   for (int i = 0; i < 4; ++i)
   {
      glGenTextures(1, &my_pass_texture[i]);
      glBindTexture(GL_TEXTURE_2D, my_pass_texture[i]);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH >> i, HEIGHT >> i, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      glBindTexture(GL_TEXTURE_2D, 0);

      glGenFramebuffersEXT(1, &my_pass_fbo[i]);
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, my_pass_fbo[i]);
      glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, my_pass_texture[i], 0);
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

      glGenTextures(1, &my_swap_texture[i]);
      glBindTexture(GL_TEXTURE_2D, my_swap_texture[i]);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH >> i, HEIGHT >> i, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      glBindTexture(GL_TEXTURE_2D, 0);

      glGenFramebuffersEXT(1, &my_swap_fbo[i]);
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, my_swap_fbo[i]);
      glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, my_swap_texture[i], 0);
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
   }

// Load in our shaders.
   float kernel[3] = { 5.0f, 6.0f, 5.0f };
// Normalize kernel coefficients.
   float sum = 0.0f;
   for (int i = 0; i < 3; ++i) {   sum += kernel[i];   }
   for (int i = 0; i < 3; ++i) {   kernel[i] /= sum;   }

   char error[4096] = {0};

// This will be the blur shader.
   my_blur = glCreateProgramObjectARB();

   unsigned char* vertex_data = ReadShaderFile("pass.vertex");
   unsigned int vertex_object = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
   glShaderSourceARB(vertex_object, 1, (const char**) &vertex_data, NULL);
   glCompileShaderARB(vertex_object);
   glAttachObjectARB(my_blur, vertex_object);
// glDeleteObjectARB(vertex_object);
   delete [] vertex_data;

   int compiled = 0;
   glGetObjectParameterivARB(vertex_object, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

   if (compiled == 0)
   {
      glGetInfoLogARB(vertex_object, sizeof(error), NULL, error);
      MessageBox(NULL, error, "Vertex Shader Compile Error", MB_OK | MB_ICONEXCLAMATION);
   }

   unsigned char* fragment_data = ReadShaderFile("blur.fragment");
   unsigned int fragment_object = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
   glShaderSourceARB(fragment_object, 1, (const char**) &fragment_data, NULL);
   glCompileShaderARB(fragment_object);
   glAttachObjectARB(my_blur, fragment_object);
   glDeleteObjectARB(fragment_object);

   delete [] fragment_data;

   compiled = 0;
   glGetObjectParameterivARB(fragment_object, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

   if (compiled == 0)
   {
      glGetInfoLogARB(fragment_object, sizeof(error), NULL, error);
      MessageBox(NULL, error, "Fragment Shader Compile Error", MB_OK | MB_ICONEXCLAMATION);
   }

   glLinkProgramARB(my_blur);
   int linked = 0;
   glGetObjectParameterivARB(my_blur, GL_OBJECT_LINK_STATUS_ARB, &linked);

   if (linked == 0)
   {
      glGetInfoLogARB(my_blur, sizeof(error), NULL, error);
      MessageBox(NULL, error, "Shader Linking Error", MB_OK | MB_ICONEXCLAMATION);
   }

   glUseProgramObjectARB(my_blur);
   unsigned int location = glGetUniformLocationARB(my_blur, "coefficients");
   glUniform3fvARB(location, 1, kernel);

   glUseProgramObjectARB(0);

// This is the shader that combines all our down sampled textures, or should I say FBOs.
   my_combine = glCreateProgramObjectARB();
   glAttachObjectARB(my_combine, vertex_object); // We don't load and compile the same vertex shader as used
                                                 // in the previous vertex and fragment shader pair.
                                                 // We'll just reuse it.

   compiled = 0;
   glGetObjectParameterivARB(vertex_object, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

   if (compiled == 0)
   {
      glGetInfoLogARB(vertex_object, sizeof(error), NULL, error);
      MessageBox(NULL, error, "Vertex Shader Compile Error", MB_OK | MB_ICONEXCLAMATION);
   }

   fragment_data = ReadShaderFile("combine.fragment");
   fragment_object = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
   glShaderSourceARB(fragment_object, 1, (const char**) &fragment_data, NULL);
   glCompileShaderARB(fragment_object);
   glAttachObjectARB(my_combine, fragment_object);
   glDeleteObjectARB(fragment_object);

   delete [] fragment_data;

   compiled = 0;
   glGetObjectParameterivARB(fragment_object, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

   if (compiled == 0)
   {
      glGetInfoLogARB(fragment_object, sizeof(error), NULL, error);
      MessageBox(NULL, error, "Fragment Shader Compile Error", MB_OK | MB_ICONEXCLAMATION);
   }

   glLinkProgramARB(my_combine);
   linked = 0;
   glGetObjectParameterivARB(my_combine, GL_OBJECT_LINK_STATUS_ARB, &linked);

   if (linked == 0)
   {
      glGetInfoLogARB(my_combine, sizeof(error), NULL, error);
      MessageBox(NULL, error, "Shader Linking Error", MB_OK | MB_ICONEXCLAMATION);
   }

   glUseProgramObjectARB(0);

// This will be the high pass shader to capture the over brights of our scene.
   my_high_pass = glCreateProgramObjectARB();
   glAttachObjectARB(my_high_pass, vertex_object);
   glDeleteObjectARB(vertex_object); // Now we must delete it.

   compiled = 0;
   glGetObjectParameterivARB(vertex_object, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

   if (compiled == 0)
   {
      glGetInfoLogARB(vertex_object, sizeof(error), NULL, error);
      MessageBox(NULL, error, "Vertex Shader Compile Error", MB_OK | MB_ICONEXCLAMATION);
   }

   fragment_data = ReadShaderFile("hipass.fragment");
   fragment_object = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
   glShaderSourceARB(fragment_object, 1, (const char**) &fragment_data, NULL);
   glCompileShaderARB(fragment_object);
   glAttachObjectARB(my_high_pass, fragment_object);
   glDeleteObjectARB(fragment_object);

   delete [] fragment_data;

   compiled = 0;
   glGetObjectParameterivARB(fragment_object, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

   if (compiled == 0)
   {
      glGetInfoLogARB(fragment_object, sizeof(error), NULL, error);
      MessageBox(NULL, error, "Fragment Shader Compile Error", MB_OK | MB_ICONEXCLAMATION);
   }

   glLinkProgramARB(my_high_pass);
   linked = 0;
   glGetObjectParameterivARB(my_high_pass, GL_OBJECT_LINK_STATUS_ARB, &linked);

   if (linked == 0)
   {
      glGetInfoLogARB(my_high_pass, sizeof(error), NULL, error);
      MessageBox(NULL, error, "Shader Linking Error", MB_OK | MB_ICONEXCLAMATION);
   }

   glUseProgramObjectARB(0);

// This will be our simple lighting shader model.
   float exponent = 16.0f;
   float light_position[3] = {-5.0f, 6.0f, 5.0f};
   float model_color[3] = {0.8f, 0.3f, 0.5f};

   my_shader = glCreateProgramObjectARB();
   vertex_data = ReadShaderFile("shader.vertex");
   vertex_object = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
   glShaderSourceARB(vertex_object, 1, (const char**) &vertex_data, NULL);
   glCompileShaderARB(vertex_object);
   glAttachObjectARB(my_shader, vertex_object);
   glDeleteObjectARB(vertex_object);
   delete [] vertex_data;

   compiled = 0;
   glGetObjectParameterivARB(vertex_object, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

   if (compiled == 0)
   {
      glGetInfoLogARB(vertex_object, sizeof(error), NULL, error);
      MessageBox(NULL, error, "Vertex Shader Compile Error", MB_OK | MB_ICONEXCLAMATION);
   }

   fragment_data = ReadShaderFile("shader.fragment");
   fragment_object = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
   glShaderSourceARB(fragment_object, 1, (const char**) &fragment_data, NULL);
   glCompileShaderARB(fragment_object);
   glAttachObjectARB(my_shader, fragment_object);
   glDeleteObjectARB(fragment_object);

   delete [] fragment_data;

   compiled = 0;
   glGetObjectParameterivARB(fragment_object, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

   if (compiled == 0)
   {
      glGetInfoLogARB(fragment_object, sizeof(error), NULL, error);
      MessageBox(NULL, error, "Fragment Shader Compile Error", MB_OK | MB_ICONEXCLAMATION);
   }

   glLinkProgramARB(my_shader);
   linked = 0;
   glGetObjectParameterivARB(my_shader, GL_OBJECT_LINK_STATUS_ARB, &linked);

   if (linked == 0)
   {
      glGetInfoLogARB(my_shader, sizeof(error), NULL, error);
      MessageBox(NULL, error, "Shader Linking Error", MB_OK | MB_ICONEXCLAMATION);
   }

   glUseProgramObjectARB(my_shader);
   location = glGetUniformLocationARB(my_shader, "exponent");
   glUniform1fvARB(location, 1, &exponent);
   location = glGetUniformLocationARB(my_shader, "light_position");
   glUniform3fvARB(location, 1, light_position);
   location = glGetUniformLocationARB(my_shader, "model_color");
   glUniform3fvARB(location, 1, model_color);

   glUseProgramObjectARB(0);

   return;
}

//------------------------------------------------------------------------------
// Clean up the texture and shader. 
//------------------------------------------------------------------------------
void ShutDown()
{
   glDeleteObjectARB(my_shader);
   glDeleteObjectARB(my_combine);
   glDeleteObjectARB(my_high_pass);
   glDeleteObjectARB(my_blur);

   for (int i = 0; i < 4; ++i)
   {
      glDeleteTextures(1, &my_pass_texture[i]);
      glDeleteFramebuffersEXT(1, &my_pass_fbo[i]);

      glDeleteTextures(1, &my_swap_texture[i]);
      glDeleteFramebuffersEXT(1, &my_swap_fbo[i]);
   }

   return;
}

//------------------------------------------------------------------------------
// Resize the OpenGL environment.
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
// Let's blur!
//------------------------------------------------------------------------------
void Blur(unsigned int src_texture[], unsigned int dst_fbo[], direction dirn)
{
   glUseProgramObjectARB(my_blur);
   unsigned int location = glGetUniformLocation(my_blur, "source");
   glUniform1i(location, 0);
   glEnable(GL_TEXTURE_2D);

   for (int i = 0; i < 4; ++i)
   {
      int width = WIDTH >> i;
      float offset[2] = {0.0f, 0.0f};
            offset[dirn] = 1.2f / width;

      location = glGetUniformLocationARB(my_blur, "offset");
      glUniform2fvARB(location, 1, offset);

      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, dst_fbo[i]);
      glViewport(0, 0, width, HEIGHT >> i);
      glBindTexture(GL_TEXTURE_2D, src_texture[i]);
      glBegin(GL_QUADS);
         glTexCoord2i(0, 0); glVertex2i(-1, -1);
         glTexCoord2i(0, 1); glVertex2i(-1,  1);
         glTexCoord2i(1, 1); glVertex2i( 1,  1);
         glTexCoord2i(1, 0); glVertex2i( 1, -1);
      glEnd();
   }

   glUseProgramObjectARB(0);

   return;
}

//------------------------------------------------------------------------------
// Renders the scene of our Teapot in all its HDR glory.
//------------------------------------------------------------------------------
void Render()
{
   static color color_range[MAX_COLORS] = {{0.8f, 0.8f, 0.8f},
                                           {0.8f, 0.2f, 0.2f},
                                           {0.2f, 0.8f, 0.3f},
                                           {0.8f, 0.8f, 0.4f},
                                           {0.2f, 0.2f, 0.8f}};
   static color goal_color   = {0.8f, 0.8f, 0.8f};
   static color actual_color = {0.8f, 0.1f, 0.6f};
   static int li = 0;

   if ((fabs(goal_color[x] - actual_color[x]) < EPSILON) &&
       (fabs(goal_color[y] - actual_color[y]) < EPSILON) &&
       (fabs(goal_color[z] - actual_color[z]) < EPSILON))
   {
      li = rand() % MAX_COLORS; // Assign a new color.
      goal_color[x] = color_range[li][x];
      goal_color[y] = color_range[li][y];
      goal_color[z] = color_range[li][z];
   }
   else
   {
      actual_color[x] += ((goal_color[x] - actual_color[x] > 0.0f) ? 0.01f : -0.01f);
      actual_color[y] += ((goal_color[y] - actual_color[y] > 0.0f) ? 0.01f : -0.01f);
      actual_color[z] += ((goal_color[z] - actual_color[z] > 0.0f) ? 0.01f : -0.01f);
   }

// Step 1 - Render our scene to the main texture / frame buffer.
   {
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, my_main_fbo);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glLoadIdentity();
      glTranslatef(0.0f, 0.0f, -g_mouse_z);
      glRotatef(-g_mouse_y, 1.0f, 0.0f, 0.0f);
      glRotatef(-g_mouse_x, 0.0f, 1.0f, 0.0f);
      glEnable(GL_DEPTH_TEST);

      float matrix[16];
      float eye[3];
      glGetFloatv(GL_MODELVIEW_MATRIX, matrix); // This is how we can get the eye / camera
                                                // position in OpenGL.
      eye[x] = -(matrix[0] * matrix[12] + matrix[1] * matrix[13] + matrix[2] * matrix[14]);
      eye[y] = -(matrix[4] * matrix[12] + matrix[5] * matrix[13] + matrix[6] * matrix[14]);
      eye[z] = -(matrix[8] * matrix[12] + matrix[9] * matrix[13] + matrix[10] * matrix[14]);

      glUseProgramObjectARB(my_shader);
      unsigned int location = glGetUniformLocationARB(my_shader, "eye_position");
      glUniform3fvARB(location, 1, eye);
                   location = glGetUniformLocationARB(my_shader, "model_color");
      glUniform3fvARB(location, 1, actual_color);
      glDisable(GL_CULL_FACE);
      RenderSolidTeapot(5.0);
      glEnable(GL_CULL_FACE);

      glDisable(GL_DEPTH_TEST);
   }

   glMatrixMode(GL_MODELVIEW); /* Reset and store the model and projection matrices. */
   glPushMatrix();
   glLoadIdentity();
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();

// Step 2 - High pass filter.
   {
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, my_pass_fbo[0]);
      glUseProgramObjectARB(my_high_pass);
      unsigned int location = glGetUniformLocation(my_high_pass, "source");
      glUniform1i(location, 0);
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, my_main_texture);
      glBegin(GL_QUADS);
         glTexCoord2i(0, 0); glVertex2i(-1, -1);
         glTexCoord2i(0, 1); glVertex2i(-1, 1);
         glTexCoord2i(1, 1); glVertex2i(1, 1);
         glTexCoord2i(1, 0); glVertex2i(1, -1);
      glEnd();
      glUseProgramObjectARB(0);
   }

// Step 3 - Down sample.
   {
      glBindTexture(GL_TEXTURE_2D, my_pass_texture[0]);

      for (int i = 1; i < 4; ++i)
      {
         glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, my_pass_fbo[i]);
         glViewport(0, 0, WIDTH >> i, HEIGHT >> i);
         glBegin(GL_QUADS);
            glTexCoord2i(0, 0); glVertex2i(-1, -1);
            glTexCoord2i(0, 1); glVertex2i(-1, 1);
            glTexCoord2i(1, 1); glVertex2i(1, 1);
            glTexCoord2i(1, 0); glVertex2i(1, -1);
         glEnd();
      }
   }

// Step 4 - Perform simple blurring.
// Perform the horizontal blurring pass.
   Blur(my_pass_texture, my_swap_fbo, HORIZONTAL);
// Perform the vertical blurring pass.
   Blur(my_swap_texture, my_pass_fbo, VERTICAL);

// Step 5 - Render the final frame by combining all the downsampled hi-pass filtered textures.
   {
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
      glViewport(0, 0, WIDTH, HEIGHT);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(0.0, 1.0, 1.0, 0.0, -1.0, 1.0);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      glUseProgramObjectARB(my_combine);

      glActiveTexture(GL_TEXTURE0 + 0);
      glBindTexture(GL_TEXTURE_2D, my_pass_texture[0]);
      glEnable(GL_TEXTURE_2D);
      unsigned int location = glGetUniformLocation(my_combine, "Pass0");
      glUniform1i(location, 0);

      glActiveTexture(GL_TEXTURE0 + 1);
      glBindTexture(GL_TEXTURE_2D, my_pass_texture[1]);
      glEnable(GL_TEXTURE_2D);
      location = glGetUniformLocation(my_combine, "Pass1");
      glUniform1i(location, 1);

      glActiveTexture(GL_TEXTURE0 + 2);
      glBindTexture(GL_TEXTURE_2D, my_pass_texture[2]);
      glEnable(GL_TEXTURE_2D);
      location = glGetUniformLocation(my_combine, "Pass2");
      glUniform1i(location, 2);

      glActiveTexture(GL_TEXTURE0 + 3);
      glBindTexture(GL_TEXTURE_2D, my_pass_texture[3]);
      glEnable(GL_TEXTURE_2D);
      location = glGetUniformLocation(my_combine, "Pass3");
      glUniform1i(location, 3);

      glActiveTexture(GL_TEXTURE0 + 4);
      glBindTexture(GL_TEXTURE_2D, my_main_texture);
      glEnable(GL_TEXTURE_2D);
      location = glGetUniformLocation(my_combine, "Scene");
      glUniform1i(location, 4);

      glBegin(GL_QUADS);
         glTexCoord2i(0, 1); glVertex2i(0, 0);
         glTexCoord2i(1, 1); glVertex2i(1, 0);
         glTexCoord2i(1, 0); glVertex2i(1, 1);
         glTexCoord2i(0, 0); glVertex2i(0, 1);
      glEnd();

      glUseProgramObjectARB(0);
   }

   glMatrixMode(GL_PROJECTION); // Restore the projection and model matrices.
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();

   glActiveTexture(GL_TEXTURE0 + 3);
   glDisable(GL_TEXTURE_2D);
   glActiveTexture(GL_TEXTURE0 + 2);
   glDisable(GL_TEXTURE_2D);
   glActiveTexture(GL_TEXTURE0 + 1);
   glDisable(GL_TEXTURE_2D);
   glActiveTexture(GL_TEXTURE0 + 0);
   glDisable(GL_TEXTURE_2D);

   return;
}
