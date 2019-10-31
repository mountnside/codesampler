/* File: Shader.h; Mode: C++; Tab-width: 3; Author: Simon Flannery;           */

#ifndef SHADER_H
#define SHADER_H

class Shader
{
public:
   void Load(const char* source_vertex, const char* source_fragment);
   void Delete(); 

   void Enable();
   void Disable();
   void Bind();
   void Bind(const char* name, const float* value, size_t size);

protected:
   unsigned int program;
};

#endif
