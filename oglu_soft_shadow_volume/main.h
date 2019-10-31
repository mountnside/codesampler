//-----------------------------------------------------------------------------
// Just handles the includes.
//-----------------------------------------------------------------------------

#ifndef _MAIN_H_
#define _MAIN_H_

#include <windows.h>
#include <iostream>
#include <sys/stat.h>
#include <gl/gl.h>
#include <gl/glu.h>

// For convenience, this project ships with its own "glext.h" extension header 
// file. If you have trouble running this sample, it may be that this "glext.h" 
// file is defining something that your hardware doesn’t actually support. 
// Try recompiling the sample using your own local, vendor-specific "glext.h" 
// header file.

#include "glext.h"      // Sample's header file
//#include <GL/glext.h> // Your local header file

#include "extensions.h"
#include "fbo.h"
#include "shaders.h"

#endif
