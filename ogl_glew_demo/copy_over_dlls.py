#------------------------------------------------------------------------------
#           Name: copy_over_dlls.py
#         Author: Kevin Harris (kevin@codesampler.com)
#  Last Modified: 03/01/05
#    Description: 
#------------------------------------------------------------------------------

import os
import shutil

if not os.path.isdir( "./ogl_glew_demo/Debug/" ):
    os.mkdir( "./ogl_glew_demo/Debug/" )

if os.path.isfile( "./glew/bin/glew32d.dll" ):
    print "Copying glew32d.dll over..."
    shutil.copyfile( "./glew/bin/glew32d.dll", "./ogl_glew_demo/Debug/glew32d.dll" )
else:
    print "Can't find glew32d.dll. Build GLEW first."
    

if not os.path.isdir( "./ogl_glew_demo/Release/" ):
    os.mkdir( "./ogl_glew_demo/Release/" )
    
if os.path.isfile( "./glew/bin/glew32.dll" ):
    print "Copying glew32.dll over..."
    shutil.copyfile( "./glew/bin/glew32.dll", "./ogl_glew_demo/Release/glew32.dll" )
else:
    print "Can't find glew32.dll. Build GLEW first."
    
raw_input( '\n\nPress Enter to exit...' )
