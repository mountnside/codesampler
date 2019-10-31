/* File: Shader.cpp; Mode: C++; Tab-width: 3; Author: Simon Flannery;         */

#define _CRT_SECURE_NO_DEPRECATE /* Stop MS VS 2005 from advertising their new
                                    and more secure CRT library! */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include <windows.h>
#include "glew.h"

#include "Shader.h"

void Shader::Load(const char* vertex_source, const char* fragment_source)
{
   program = 0;

   char error[4096] = {0};
   char* vertex_data = NULL;
   FILE* file = fopen(vertex_source, "r");
   
   if (file != NULL) /* A technique for loading shader files by Kevin Harris. */
   {
      struct _stat file_stats;

      if (_stat(vertex_source, &file_stats) == 0)
      {
         vertex_data = new char[file_stats.st_size];

         size_t bytes = fread(vertex_data, 1, file_stats.st_size, file);

         vertex_data[bytes] = 0;
      }

      fclose(file);
   }
   else
   {
      vertex_data = new char[strlen(vertex_source) + 1];
      strcpy(vertex_data, vertex_source);
   }

   char* fragment_data = NULL;
         file = fopen(fragment_source, "r");
   
   if (file != NULL) /* A technique for loading shader files by Kevin Harris. */
   {
      struct _stat file_stats;

      if (_stat(fragment_source, &file_stats) == 0)
      {
         fragment_data = new char[file_stats.st_size];

         size_t bytes = fread(fragment_data, 1, file_stats.st_size, file);

         fragment_data[bytes] = 0;
      }

      fclose(file);
   }
   else
   {
      fragment_data = new char[strlen(fragment_source) + 1];
      strcpy(fragment_data, fragment_source);
   }

   if (vertex_data != NULL)
   {
      program = glCreateProgramObjectARB();
      
      unsigned int vertex_object = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
      glShaderSourceARB(vertex_object, 1, (const char**) &vertex_data, NULL);
      glCompileShaderARB(vertex_object);
      glAttachObjectARB(program, vertex_object);
      glDeleteObjectARB(vertex_object);

      delete [] vertex_data;

      int compiled = 0;
      glGetObjectParameterivARB(vertex_object, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

      if (compiled == 0)
      {
         glGetInfoLogARB(vertex_object, sizeof(error), NULL, error);
         MessageBox(NULL, error, "Vertex Shader Compile Error", MB_OK | MB_ICONEXCLAMATION);
      }
      else
      {
      /* Setup some common attributes - most not used. */
         glBindAttribLocationARB(program, 0, "s_attribute_0");
         glBindAttribLocationARB(program, 1, "s_attribute_1");
         glBindAttribLocationARB(program, 2, "s_attribute_2");
         glBindAttribLocationARB(program, 3, "s_attribute_3");
         glBindAttribLocationARB(program, 4, "s_attribute_4");
         glBindAttribLocationARB(program, 5, "s_attribute_5");

         glBindAttribLocationARB(program, 0, "s_xyz");
         glBindAttribLocationARB(program, 1, "s_normal");
         glBindAttribLocationARB(program, 2, "s_tangent");
         glBindAttribLocationARB(program, 3, "s_binormal");
         glBindAttribLocationARB(program, 4, "s_texcoord");
      }
   }

   if (fragment_data != NULL)
   {
      if (program == 0)
      {
         program = glCreateProgramObjectARB();
      }

      unsigned int fragment_object = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
      glShaderSourceARB(fragment_object, 1, (const char**) &fragment_data, NULL);
      glCompileShaderARB(fragment_object);
      glAttachObjectARB(program, fragment_object);
      glDeleteObjectARB(fragment_object);

      delete [] fragment_data;

      int compiled = 0;
      glGetObjectParameterivARB(fragment_object, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

      if (compiled == 0)
      {
         glGetInfoLogARB(fragment_object, sizeof(error), NULL, error);
         MessageBox(NULL, error, "Fragment Shader Compile Error", MB_OK | MB_ICONEXCLAMATION);
      }
   }
   
   if (program != 0)
   {
      glLinkProgramARB(program);
      int linked = 0;
      glGetObjectParameterivARB(program, GL_OBJECT_LINK_STATUS_ARB, &linked);

      if (linked == 0)
      {
         glGetInfoLogARB(program, sizeof(error), NULL, error);
         MessageBox(NULL, error, "Shader Linking Error", MB_OK | MB_ICONEXCLAMATION);
      }

      glUseProgramObjectARB(program);
      
      for (int i = 0; i < 8; ++i)
      {
         char texture[32] = {0};
         sprintf(texture, "s_texture_%d", i);
         int location = glGetUniformLocationARB(program, texture);

         if (location >= 0)
         {
            glUniform1iARB(location, i);
         }
      }
      
      glUseProgramObjectARB(0);
   }

   return;
}

void Shader::Delete()
{
   glDeleteObjectARB(program);

   return;
}

void Shader::Enable()
{
   return;
}

void Shader::Disable()
{
   glUseProgramObjectARB(0);

   return;
}

void Shader::Bind()
{
   if (program != 0)
   {
      glUseProgramObjectARB(program);
   }

   return;
}

void Shader::Bind(const char* name, const float* value, size_t size)
{
   if (program != 0)
   {
      glUseProgramObjectARB(program);

      unsigned int location = glGetUniformLocationARB(program, name); /* This is EXPENSIVE
                                                                         and should be avoided. */

      if      (size == 1)  glUniform1fvARB(location, 1, value);
      else if (size == 2)  glUniform2fvARB(location, 1, value);
      else if (size == 3)  glUniform3fvARB(location, 1, value);
      else if (size == 4)  glUniform4fvARB(location, 1, value);
      else if (size == 9)  glUniformMatrix3fvARB(location, 1, false, value);
      else if (size == 16) glUniformMatrix4fvARB(location, 1, false, value);
   }

   return;
}
