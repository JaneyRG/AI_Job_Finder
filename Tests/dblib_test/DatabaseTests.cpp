// DatabaseTests.cpp
//
// This file contains an example of how to write C++ unit tests for a
// Windows dynamic‑link library (DLL) using the Microsoft C++ Unit Test
// framework.  The goal of this sample is to illustrate how to load a
// DLL at run time, obtain pointers to its exported functions via
// `GetProcAddress`, and then exercise those functions in a suite of
// deterministic tests.  Because the actual contents of the DLL
// contained in the provided `Database.7z` archive are unknown at the
// time this sample was prepared, the function names and signatures
// below are placeholders.  Replace them with the real exports from
// your DLL once you have extracted it.

#include "pch.h" // Precompiled header for faster build times

#include <Windows.h>
#include <string>
#include <stdexcept>

// Include the Visual Studio C++ test framework headers.  This header
// provides the `TEST_CLASS` and `TEST_METHOD` macros along with a set
// of assertion helpers.
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

// Helper class responsible for loading the target DLL and resolving
// pointers to the exported functions.  Each typedef corresponds to a
// function signature expected from the DLL.  Adjust these typedefs
// according to the real API exposed by your DLL.
class DatabaseLibrary
{
public:
    // Typedefs for exported function signatures.  Replace these with
    // the correct signatures once you know them.  For example, if
    // your DLL exposes `int InitializeDatabase(const wchar_t* path);`
    // then update the typedef accordingly.
    typedef int (__stdcall *InitFunc)();
    typedef int (__stdcall *AddRecordFunc)(int id, const wchar_t* value);
    typedef int (__stdcall *GetRecordFunc)(int id, wchar_t* buffer, int bufferSize);
    typedef int (__stdcall *DeleteRecordFunc)(int id);

    DatabaseLibrary(const std::wstring& dllPath)
        : m_module(nullptr),
          Initialize(nullptr),
          AddRecord(nullptr),
          GetRecord(nullptr),
          DeleteRecord(nullptr)
    {
        // Attempt to load the DLL.  When running tests under Visual
        // Studio, the working directory defaults to the project
        // location.  You may need to adjust the relative path here or
        // copy the DLL into your test project's output directory.
        m_module = ::LoadLibraryW(dllPath.c_str());
        if (!m_module) {
            // Provide a descriptive error message if the DLL fails
            // to load so that the test output is useful.  `GetLastError()`
            // returns a Windows error code; use `std::system_category` to
            // translate it to a human readable message.
            throw std::runtime_error("Failed to load DLL: " +
                                     std::to_string(::GetLastError()));
        }

        // Resolve exported functions.  If any of these lookups fail
        // (i.e. return nullptr) you can either throw here or test
        // their validity in your test cases.
        Initialize    = reinterpret_cast<InitFunc>(::GetProcAddress(m_module, "Initialize"));
        AddRecord     = reinterpret_cast<AddRecordFunc>(::GetProcAddress(m_module, "AddRecord"));
        GetRecord     = reinterpret_cast<GetRecordFunc>(::GetProcAddress(m_module, "GetRecord"));
        DeleteRecord  = reinterpret_cast<DeleteRecordFunc>(::GetProcAddress(m_module, "DeleteRecord"));
    }

    ~DatabaseLibrary()
    {
        if (m_module) {
            ::FreeLibrary(m_module);
            m_module = nullptr;
        }
    }

    // Public member variables hold the resolved function pointers.  They
    // remain null if the symbol is absent from the DLL.
    InitFunc        Initialize;
    AddRecordFunc   AddRecord;
    GetRecordFunc   GetRecord;
    DeleteRecordFunc DeleteRecord;

private:
    HMODULE m_module;
};

// Test suite for the Database DLL.  Each TEST_METHOD exercises a
// particular aspect of the library.  When filling in your own tests,
// follow the AAA pattern: arrange state, act by calling into the
// library, and assert on the expected outcome.
TEST_CLASS(DatabaseDllTests)
{
public:

    // Helper routine to construct the DatabaseLibrary object.  If
    // loading the DLL fails, the exception will propagate to the test
    // and cause a failure with a descriptive message.
    static DatabaseLibrary LoadLibrary()
    {
        // You may need to adjust this path depending on where the
        // extracted DLL lives relative to your test binary.  During
        // development you can place the DLL next to the test
        // executable and specify just L"Database.dll".
        return DatabaseLibrary(L"..\\..\\Database.dll");
    }

    TEST_METHOD(DllLoadsSuccessfully)
    {
        // Arrange & Act
        DatabaseLibrary dll = LoadLibrary();

        // Assert that all required function pointers are valid.  If
        // your DLL exports additional functions, add them here.
        Assert::IsNotNull(dll.Initialize, L"Initialize function should be exported");
        Assert::IsNotNull(dll.AddRecord,  L"AddRecord function should be exported");
        Assert::IsNotNull(dll.GetRecord,  L"GetRecord function should be exported");
        Assert::IsNotNull(dll.DeleteRecord, L"DeleteRecord function should be exported");
    }

    TEST_METHOD(CanInitializeAndShutdown)
    {
        // Arrange
        DatabaseLibrary dll = LoadLibrary();
        Assert::IsNotNull(dll.Initialize);

        // Act
        int result = dll.Initialize();

        // Assert
        Assert::AreEqual(0, result, L"Initialize should return 0 for success");

        // If your DLL requires an explicit shutdown, add a call
        // here and verify it succeeds.
    }

    TEST_METHOD(AddAndRetrieveRecord)
    {
        // Arrange
        DatabaseLibrary dll = LoadLibrary();
        Assert::IsNotNull(dll.Initialize);
        Assert::IsNotNull(dll.AddRecord);
        Assert::IsNotNull(dll.GetRecord);
        Assert::IsNotNull(dll.DeleteRecord);
        int initResult = dll.Initialize();
        Assert::AreEqual(0, initResult, L"Initialize should succeed");

        // Act: add a record with ID 1 and value "Example"
        const wchar_t* testValue = L"Example";
        int addResult = dll.AddRecord(1, testValue);

        // Assert: verify add succeeded
        Assert::AreEqual(0, addResult, L"AddRecord should succeed");

        // Act: retrieve the record
        wchar_t buffer[64] = {};
        int getResult = dll.GetRecord(1, buffer, static_cast<int>(std::size(buffer)));

        // Assert: retrieval should succeed and the returned string
        // should match the inserted value.
        Assert::AreEqual(0, getResult, L"GetRecord should succeed");
        Assert::AreEqual(std::wstring(testValue), std::wstring(buffer), L"Retrieved value should match added value");

        // Clean up: delete the record
        int delResult = dll.DeleteRecord(1);
        Assert::AreEqual(0, delResult, L"DeleteRecord should succeed");
    }
};
