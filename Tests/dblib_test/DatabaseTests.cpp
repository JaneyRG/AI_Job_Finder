// DatabaseTests.cpp
//
// Unit tests for the SqlDbHandler class from the dblib DLL.
// This test suite uses link-time binding to the DLL via its import library (.lib)
// and tests the job finder database functionality including adding companies,
// adding jobs, updating job status, and retrieving jobs by status.
    
#include "pch.h" // Precompiled header for faster build times   
#include "CppUnitTest.h"

// Include the SqlDbHandler header to access the API
#include "../../Database/dblib/SqlDbHandler.h"

#include <string>

// Workaround for filesystem issues
#if _MSC_VER >= 1900 && _MSC_VER < 1920
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace JobFinderDB;

// Test suite for the SqlDbHandler class
TEST_CLASS(SqlDbHandlerTests)
{
public:
    // Setup method called before each test
    TEST_METHOD_INITIALIZE(Setup)
    {
        // Clean up any existing test database to ensure tests start fresh
        fs::remove(DB_NAME);
    }

    // Cleanup method called after each test
    TEST_METHOD_CLEANUP(Cleanup)
    {
        // Optional: Remove test database after each test
        fs::remove(DB_NAME);
    }

    TEST_METHOD(ConstructorConnectsToDatabase)
    {
        // Arrange & Act
        SqlDbHandler handler;
        
        // Assert - if construction succeeds without exception, connection worked
        Assert::IsTrue(true, L"SqlDbHandler should construct successfully");
    }

    TEST_METHOD(CanAddCompany)
    {
        // Arrange
        SqlDbHandler handler;

        // Act - add a company to the database
        handler.AddCompany("Microsoft", "Technology");

        // Assert - no exception thrown means success
        Assert::IsTrue(true, L"AddCompany should complete without throwing");
    }

    TEST_METHOD(CanAddMultipleCompanies)
    {
        // Arrange
        SqlDbHandler handler;

        // Act - add multiple companies
        handler.AddCompany("Google", "Technology");
        handler.AddCompany("Amazon", "E-Commerce");
        handler.AddCompany("Meta", "Social Media");

        // Assert
        Assert::IsTrue(true, L"Should be able to add multiple companies");
    }

    TEST_METHOD(CanAddJobWithDefaultStatus)
    {
        // Arrange
        SqlDbHandler handler;
        handler.AddCompany("Apple", "Technology");

        // Act - add job with default NotApplied status
        handler.AddJob(
            "Software Engineer",
            "C++ Developer position",
            "Apple",
            8  // match score
        );

        // Assert - retrieve jobs with NotApplied status
        Jobs jobs = handler.GetJobsByStatus(jobStatus::NotApplied);
        Assert::IsTrue(jobs.size() > 0, L"Should have at least one job with NotApplied status");
        Assert::AreEqual(std::string("Software Engineer"), jobs[0].title, L"Job title should match");
        Assert::AreEqual(std::string("Apple"), jobs[0].companyName, L"Company name should match");
        Assert::AreEqual(8, jobs[0].match, L"Match score should be 8");
    }

    TEST_METHOD(CanAddJobWithSpecificStatus)
    {
        // Arrange
        SqlDbHandler handler;
        handler.AddCompany("Netflix", "Entertainment");

        // Act - add job with Applied status
        handler.AddJob(
            "DevOps Engineer",
            "Cloud infrastructure specialist",
            "Netflix",
            9,
            jobStatus::Applied
        );

        // Assert
        Jobs jobs = handler.GetJobsByStatus(jobStatus::Applied);
        Assert::IsTrue(jobs.size() > 0, L"Should have at least one applied job");
        Assert::AreEqual(std::string("DevOps Engineer"), jobs[0].title, L"Job title should match");
    }

    TEST_METHOD(CanUpdateJobStatus)
    {
        // Arrange
        SqlDbHandler handler;
        handler.AddCompany("Tesla", "Automotive");
        handler.AddJob("ML Engineer", "AI/ML role", "Tesla", 7, jobStatus::NotApplied);

        // Act - update job status from NotApplied to Interviewing
        handler.UpdateJobStatus("ML Engineer", jobStatus::Interviewing);

        // Assert - job should now appear in Interviewing status
        Jobs interviewingJobs = handler.GetJobsByStatus(jobStatus::Interviewing);
        Assert::IsTrue(interviewingJobs.size() > 0, L"Should find job with Interviewing status");
        Assert::AreEqual(std::string("ML Engineer"), interviewingJobs[0].title, L"Job title should match");
        
        // Verify it's no longer in NotApplied
        Jobs notAppliedJobs = handler.GetJobsByStatus(jobStatus::NotApplied);
        bool foundInNotApplied = false;
        for (const auto& job : notAppliedJobs) {
            if (job.title == "ML Engineer") {
                foundInNotApplied = true;
                break;
            }
        }
        Assert::IsFalse(foundInNotApplied, L"Job should no longer be in NotApplied status");
    }

    TEST_METHOD(CanRetrieveJobsByMultipleStatuses)
    {
        // Arrange
        SqlDbHandler handler;
        handler.AddCompany("SpaceX", "Aerospace");
        handler.AddJob("Rocket Engineer", "Propulsion systems", "SpaceX", 10, jobStatus::NotApplied);
        handler.AddJob("Flight Software Engineer", "Embedded C++", "SpaceX", 9, jobStatus::Applied);
        handler.AddJob("Systems Engineer", "Integration testing", "SpaceX", 8, jobStatus::Interviewing);

        // Act & Assert - retrieve jobs by different statuses
        Jobs notApplied = handler.GetJobsByStatus(jobStatus::NotApplied);
        Jobs applied = handler.GetJobsByStatus(jobStatus::Applied);
        Jobs interviewing = handler.GetJobsByStatus(jobStatus::Interviewing);

        Assert::AreEqual(size_t(1), notApplied.size(), L"Should have 1 NotApplied job");
        Assert::AreEqual(size_t(1), applied.size(), L"Should have 1 Applied job");
        Assert::AreEqual(size_t(1), interviewing.size(), L"Should have 1 Interviewing job");
    }

    TEST_METHOD(GetJobsByStatusReturnsEmptyWhenNoMatches)
    {
        // Arrange
        SqlDbHandler handler;
        handler.AddCompany("Adobe", "Software");
        handler.AddJob("UX Designer", "Design role", "Adobe", 6, jobStatus::NotApplied);

        // Act - query for Offered status when no jobs have that status
        Jobs offeredJobs = handler.GetJobsByStatus(jobStatus::Offered);

        // Assert
        Assert::AreEqual(size_t(0), offeredJobs.size(), L"Should return empty vector when no jobs match status");
    }

    TEST_METHOD(CanHandleRejectedStatus)
    {
        // Arrange
        SqlDbHandler handler;
        handler.AddCompany("Oracle", "Database Software");
        handler.AddJob("Database Admin", "DBA position", "Oracle", 5, jobStatus::Applied);

        // Act - update to rejected status
        handler.UpdateJobStatus("Database Admin", jobStatus::Rejected);

        // Assert
        Jobs rejectedJobs = handler.GetJobsByStatus(jobStatus::Rejected);
        Assert::IsTrue(rejectedJobs.size() > 0, L"Should have rejected job");
        Assert::AreEqual(std::string("Database Admin"), rejectedJobs[0].title, L"Job title should match");
    }

    TEST_METHOD(JobStructContainsAllExpectedFields)
    {
        // Arrange
        SqlDbHandler handler;
        handler.AddCompany("IBM", "Technology");
        handler.AddJob(
            "Quantum Researcher",
            "Quantum computing research",
            "IBM",
            10,
            jobStatus::Offered
        );

        // Act
        Jobs jobs = handler.GetJobsByStatus(jobStatus::Offered);

        // Assert - verify all fields are populated correctly
        Assert::IsTrue(jobs.size() > 0, L"Should retrieve the job");
        const Job& job = jobs[0];
        Assert::AreEqual(std::string("Quantum Researcher"), job.title, L"Title should match");
        Assert::AreEqual(std::string("Quantum computing research"), job.description, L"Description should match");
        Assert::AreEqual(std::string("IBM"), job.companyName, L"Company name should match");
        Assert::AreEqual(10, job.match, L"Match score should be 10");
        Assert::IsTrue(job.status == jobStatus::Offered, L"Status should be Offered");
    }

    TEST_METHOD(CanAddMultipleJobsToSameCompany)
    {
        // Arrange
        SqlDbHandler handler;
        handler.AddCompany("Nvidia", "Hardware");

        // Act - add multiple jobs for the same company
        handler.AddJob("CUDA Engineer", "GPU programming", "Nvidia", 9, jobStatus::NotApplied);
        handler.AddJob("Graphics Driver Engineer", "Driver development", "Nvidia", 8, jobStatus::NotApplied);
        handler.AddJob("AI Researcher", "Deep learning research", "Nvidia", 10, jobStatus::Applied);

        // Assert
        Jobs notApplied = handler.GetJobsByStatus(jobStatus::NotApplied);
        Assert::IsTrue(notApplied.size() >= 2, L"Should have at least 2 NotApplied jobs for Nvidia");
    }
};
