sm2port <version> - Spider-Man 2 on Windows
=================================================================

An unofficial, non-commercial fan project: a native Windows port of the
Xbox release of Spider-Man 2, run from your own disc image. It is not
affiliated with, endorsed by or sponsored by Marvel, Activision, Treyarch
or Microsoft. No game data is included or distributed: no disc images, no
executables, no textures, sounds, music or films. Please do not ask for
game files and do not share them.

sm2port is in early development and may still have problems. If you find
one, please open an issue on GitHub: https://github.com/SickSartori/sm2port/issues
The full guide is README.md in this folder and online at
https://github.com/SickSartori/sm2port


WHAT YOU NEED
-------------

- Windows 10 or 11, 64-bit. Nothing else to install. Windows "N" editions
  only: install the Media Feature Pack (Settings > Apps > Optional
  features), or sm2runtime.dll will not load.
- A graphics card and driver with Direct3D 11 (feature level 11_0).
- A disc image (.iso or .cci) of Spider-Man 2 for the Xbox, Italian or
  European (French, German, Spanish). XISO and Redump dumps both work, in
  one file or split in parts, and so does the compressed CCI format. Other
  releases are not supported: the launcher tells you if the disc is a
  different version.


FIRST START
-----------

1. Extract the zip where you like and start sm2.exe.
2. Choose your disc image when the launcher asks. It checks it and
   remembers where it is.
3. Pick a quality preset and the frame rate, and press PLAY.

Controls: W A S D move, the mouse (or the arrow keys) moves the camera,
Space jumps, E clings to walls, the left mouse button attacks. Xbox
controllers work as on the console. F1 opens the in-game menu, F11 toggles
full screen, F10 toggles the modern effects. Every key can be changed in
the launcher's Controls page.


WHERE ARE MY FILES
------------------

Settings (sm2port.ini), saves (xbox_hdd), the log and the shader cache are
in %LOCALAPPDATA%\sm2port. To keep them next to sm2.exe, create an empty
file named portable.txt there.


IF SOMETHING GOES WRONG
-----------------------

Open the launcher's About page and press "Log folder": sm2port.log says
what happened last. "Report a problem" opens the form on GitHub with your
version and PC filled in. Known issue: some sound calls (3D positioning)
are not emulated yet.
