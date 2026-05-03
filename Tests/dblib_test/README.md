# Database DLL Test Project

This folder contains a sample Microsoft Visual Studio C++ test project
designed to exercise a Windows dynamic‑link library (DLL).  The DLL
itself is packaged inside the provided `Database.7z` archive.  Because
the archive contents cannot be inspected within this environment, the
test code uses placeholder function names and signatures.  After
extracting the DLL you must update those placeholders to match the
real exports.

## Contents

* `DatabaseTests.cpp` – Implements a simple test suite using the
  Visual Studio C++ Unit Test framework.  The class `DatabaseLibrary`
  handles loading the DLL and resolving function pointers.  The
  `DatabaseDllTests` test class exercises the library by calling
  `Initialize`, `AddRecord`, `GetRecord` and `DeleteRecord` and
  checking for success.
* `pch.h` / `pch.cpp` – Precompiled header setup used to speed up
  compilation.
* `README.md` – This document.

## Extracting the DLL

The file `Database.7z` is a 7‑Zip archive.  To extract its contents on
Windows install 7‑Zip from [7‑Zip.org](https://www.7-zip.org) and
right‑click `Database.7z`, then choose **7‑Zip → Extract here**.  If
you do not have 7‑Zip installed you can download the command line
version (`7z.exe`) and run:

```cmd
7z x Database.7z
```

After extraction you should find a DLL (for example `Database.dll`)
along with any associated header files or import libraries.  Copy the
DLL into your test project’s output directory (e.g. `msvc_test\Debug`)
so that the tests can load it at runtime.

## Updating the Test Code

1. **Determine the exported function names and signatures.**  Use
   `dumpbin /exports Database.dll` from the Visual Studio Developer
   Command Prompt to list the exported symbols.  Alternatively, open
   any provided header files to see the API.  Note the calling
   conventions (e.g. `__stdcall`) and parameter types.

2. **Update the typedefs in `DatabaseTests.cpp`.**  Replace
   `InitFunc`, `AddRecordFunc`, `GetRecordFunc`, and
   `DeleteRecordFunc` with the correct function signatures from your
   DLL.  Remove or add additional typedefs as needed.

3. **Update the `GetProcAddress` calls.**  Change the strings
   "Initialize", "AddRecord", etc. to match the actual export names.
   If your DLL uses undecorated names you can specify them directly;
   otherwise use the decorated names reported by `dumpbin`.

4. **Adjust test logic.**  The example tests assume that the
   functions return an integer status code of 0 on success and that
   records are identified by an integer key with a string payload.
   Modify the tests to reflect the real semantics of your API.

## Building the Test Project

1. Open **Visual Studio** and create a new **C++ Unit Test Project**.
2. Add the existing files `DatabaseTests.cpp`, `pch.h` and
   `pch.cpp` to the project via **Add → Existing Item**.
3. Configure the project to use precompiled headers.  In **Project
   Properties → C/C++ → Precompiled Headers**, set *Precompiled
   Header* to **Use (/Yu)** and specify `pch.h` as the header name.
4. Copy the extracted DLL into the build output folder (for example
   `Debug` or `Release`).  You can add a *Post‑Build Event* to do
   this automatically:

   ```cmd
   copy "$(SolutionDir)\Database.dll" "$(OutDir)"
   ```

5. Build the solution.  When the tests run they will load the DLL,
   execute the exported functions, and report success or failure.

## Running the Tests

In Visual Studio open the **Test Explorer** (Test → Test Explorer).
Build the project then run all tests.  The **DllLoadsSuccessfully**
test verifies that the DLL can be loaded and that the required
functions are exported.  The **CanInitializeAndShutdown** and
**AddAndRetrieveRecord** tests demonstrate how to call into the DLL
and validate behavior.

## Extending the Test Suite

Once you have a full understanding of the DLL’s API you should add
more tests.  Consider cases such as:

* Passing invalid arguments to ensure the DLL properly reports errors.
* Adding multiple records and verifying that retrieval works for each
  key.
* Attempting to retrieve or delete records that do not exist.
* Concurrency tests if the DLL is expected to be thread‑safe.
* Stress tests with large datasets or repeated initialization/shutdown
  cycles.

By following these steps and updating the provided skeleton, you can
create a comprehensive automated test suite for your DLL that runs
within the Visual Studio C++ Unit Test framework.
