Project files for MS Visual Studio 2010 :

By default the project expects SDL 1.2, SDL Image 1.2 and Lua 5.3 to be installed
in directories :
  ..\..\..\..\SDL-1.2.15
  ..\..\..\..\SDL_image-1.2.12
  ..\..\..\..\lua
Edit the file libraries.props to set the correct paths for your configuration.

That is, if grafX2 sources are in C:\stuff\code\grafX2
Visual studio project files are in C:\stuff\code\grafX2\project\msvc
and it is expected you have C:\stuff\lua etc.

Download precompiled libraries from :

  https://www.libsdl.org/download-1.2.php :
    https://www.libsdl.org/release/SDL-devel-1.2.15-VC.zip

  https://www.libsdl.org/projects/SDL_image/release-1.2.html
    https://www.libsdl.org/projects/SDL_image/release/SDL_image-devel-1.2.12-VC.zip

  http://luabinaries.sourceforge.net/
    https://sourceforge.net/projects/luabinaries/files/5.3.4/Windows%20Libraries/Static/


Download recoil-4.2.0.tar.gz from https://sourceforge.net/projects/recoil/files/recoil/4.2.0/
and copy recoil.c and recoil.h to src subdirectory.
You can also disable RECOIL by defining the NORECOIL macro.
