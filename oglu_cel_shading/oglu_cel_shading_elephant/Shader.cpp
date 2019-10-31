/* File: Shader.cpp; Mode: C++; Tab-width: 3; Author: Simon Flannery;         */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include <windows.h>
#include "glew.h"

#include "Shader.h"

void Shader::Load(const char* source)
{
   target = id = 0;

   char* data = NULL;
   FILE* file = fopen(source, "r");

   if (file != NULL) /* A technique for loading shader files by Kevin Harris. */
   {
      struct _stat file_stats;

      if (_stat(source, &file_stats) == 0)
      {
         data = new char[file_stats.st_size];

         size_t bytes = fread(data, 1, file_stats.st_size, file);

         data[bytes] = '\0';
      }

      fclose(file);
   }
   else
   {
      data = new char[strlen(source) + 1];
      strcpy(data, source);
   }

   int error = -1;

   if (strstr(data, "!!ARBvp1.0") == data) /* Vertex programs. */
   {
      target = GL_VERTEX_PROGRAM_ARB;
      glGenProgramsARB(1, &id);
      glBindProgramARB(target, id);
      glProgramStringARB(target, GL_PROGRAM_FORMAT_ASCII_ARB, (int) strlen(data), data);
      glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &error);
   }
   else if (strstr(data, "!!ARBfp1.0") == data) /* Fragment programs. */
   {
      target = GL_FRAGMENT_PROGRAM_ARB;
      glGenProgramsARB(1, &id);
      glBindProgramARB(target, id);
      glProgramStringARB(target, GL_PROGRAM_FORMAT_ASCII_ARB, (int) strlen(data), data);
      glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &error);
   }

   if (error != -1)
   {
      int line = 0;
      char* s = data;
      while (error-- && *s)
      {
         if (*s++ == '\n')
         {
            line++;
         }
      }

      while (s >= data && *s != '\n') {   s--;   }
      char* e = ++s;
      while (*e != '\n' && *e != '\0') {   e++;   }
      *e = '\0';
      fprintf(stderr, "Error at line %d:\n\"%s\"\n", line, s);
   }

   delete [] data;

   return;
}

void Shader::Delete()
{
   glDeleteProgramsARB(1, &id);

   return;
}

void Shader::Enable()
{
   if (target != 0)
   {
      glEnable(target);
   }

   return;
}

void Shader::Disable()
{
   if (target != 0)
   {
      glDisable(target);
   }

   return;
}

void Shader::Bind()
{
   if (target != 0 && id != 0)
   {
      glBindProgramARB(target, id);
   }

   return;
}

void Shader::EnvParameter(unsigned int index, float a, float b, float c, float d)
{
   if (target != 0 && id != 0)
   {
      glProgramEnvParameter4fARB(target, index, a, b, c, d);
   }

   return;
}

void Shader::LocalParameter(unsigned int index, float a, float b, float c, float d)
{
   if (target != 0 && id != 0)
   {
      glProgramLocalParameter4fARB(target, index, a, b, c, d);
   }

   return;
}
