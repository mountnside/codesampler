Note: 

This demonstration of GLEW already includes the source for 
GLEW 1.3.1, so you will not need to download GLEW separately unless 
there's a newer version out that you would like to test.

--------------------------------------------------------------------

 --Build Steps –

Step 1. Build GLEW by loading up the Visual Studio 6.0 workspace at 
        "glew\build\vc6\glew.dsw".

Step 2. Next, build the "ogl_glew_demo" test sample.

Step 3. Copy over the glew .dlls by either running the Python 
        script, "copy_over_dlls.py", or by manually copying them 
        over like so...

        copy "glew\bin\glew32d.dll" to "ogl_glew_demo\Debug"
        copy "glew\bin\glew32.dll"  to "ogl_glew_demo\Release"

Step 4. Run the "ogl_glew_demo" test sample.

Step 5. Never make another call to wglGetProcAddress or 
        glXGetProcAddressARB again. Yeah!

--------------------------------------------------------------------

 -- Other cool stuff to do --

Run the cool command line utitlies: "glewinfo.exe" and 
"visualinfo.exe" located in "glew\bin".

--------------------------------------------------------------------
