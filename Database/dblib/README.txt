SQLite3 import library for Visual Studio C++ (x64)

Files:
- sqlite3.dll : SQLite runtime DLL supplied in the uploaded ZIP
- sqlite3.def : export definition file supplied in the uploaded ZIP
- sqlite3.lib : generated x64 MSVC import library for linking against sqlite3.dll

Usage in Visual Studio:
1. Add sqlite3.lib to Linker > Input > Additional Dependencies.
2. Add this folder to Linker > General > Additional Library Directories, or copy sqlite3.lib into your project/lib folder.
3. Ensure sqlite3.dll is copied next to your built .exe at runtime, or somewhere on PATH.
4. Include sqlite3.h from the matching SQLite amalgamation/header package in your project.

Generated with: lld-link /lib /machine:x64 /def:sqlite3.def /out:sqlite3.lib
