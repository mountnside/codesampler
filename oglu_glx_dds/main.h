//-----------------------------------------------------------------------------
//           Name: main.h
//         Author: Andreas T Jonsson
//  Last Modified: 06/09/06
//    Description:  This sample demonstrates how to load DDS textures under Linux,
//                  without using any DX headers. It is based on Kevin Harris
//                  'ogl_glx_sample' and the 'ogl_dds_texture_loader' so check them
//                  out first.
//
//   Control Keys: 
//-----------------------------------------------------------------------------

#ifndef _MAIN_H_
#define _MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <GL/glext.h>

extern PFNGLCOMPRESSEDTEXIMAGE2DARBPROC glCompressedTexImage2DARB;

#include "dds.h"

#endif //_MAIN_H_
