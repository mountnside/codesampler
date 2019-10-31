Unlike other samples, which are available from CodeSampler.com, this sample 
does not ship with the necessary .dlls to run it. You'll need to download and
install a SDK to work with this sample.

Follow the steps outlined below to compile and run this sample.

-------------------------------------------------------------------------------

1. If you haven't already, you'll need to download and install the NovodeX 
   Physics SDK v2.1.2.

   http://www.ageia.com/novodex.html

-------------------------------------------------------------------------------

2. Once the SDK is installed and you're able to run the SDK samples, you'll
   need to modify your system's environment so you can run this sample. 

   This must be done because CodeSampler.com samples are setup to run 
   standalone, unlike the SDK samples which use relative paths in 
   their workspace files to find the required NovodeX .dll(s).
   
   1. Press and hold the "Window" key on your keyboard and hit the 
      "Pause/Break" key to bring up the "System Properties" dialog box.
      
   2. Once the "System Properties" dialog box pops up, select the "Advanced" 
      tab and click the "Environment Variables" button.
   
   3. In the "System Variables" List box, double-click the "Path" variable and 
      add a path to the engine's .dlls so the sample can find them. On my 
      machine I added this to the end of the "Path" variable. 
      
      ;C:\Program Files\NovodeX SDK 2.1.2\Bin\win32
      
      Your path may be different than mine depending on where you installed
      the NovodeX SDK. Also, remember to separate your new path from 
      the other paths with a semi-colon.

-------------------------------------------------------------------------------

3. The last step is to add some new paths to Visual Studio so the header and
   library files for NovodeX can be located for compiling:

   To set these, navigate the menus to "Tools->Options". When the Options" 
   dialog opens, expand the tree control on the left to 
   "Projects->VC++ Directories" and add the following:

   Add these to Visual Studio's include path:

   C:\Program Files\NovodeX SDK 2.1.2\SDKs\Foundation\include
   C:\Program Files\NovodeX SDK 2.1.2\SDKs\Physics\include

   Add these to Visual Studio's library path:

   C:\Program Files\NovodeX SDK 2.1.2\SDKs\Foundation\lib\win32\Release
   C:\Program Files\NovodeX SDK 2.1.2\SDKs\Physics\lib\win32\Release
   
-------------------------------------------------------------------------------

