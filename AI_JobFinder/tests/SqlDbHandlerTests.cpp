#include "pch.h"
#include <gtest/gtest.h>
#include "../dblib/SqlDbHandler.h"
#include <filesystem>
#include <fstream>

using namespace JobFinderDB;

// Test fixture for SqlDbHandler tests
class SqlDbHandlerTest : public ::testing::Test
{
protected:
    SqlDbHandler* handler;
    std::string testDbPath;

    void SetUp() override
    {
        // Create a test database for each test
        testDbPath = "test_jobs.db";

        // Remove existing test database if it exists
        if (std::filesystem::exists(testDbPath))
        {
            std::filesystem::remove(testDbPath);
        }

        // Initialize the database handler
        handler = new SqlDbHandler();

        // Create tables for testing
        InitializeTestDatabase();
    }

    void TearDown() override
    {
        delete handler;

        // Clean up test database
        if (std::filesystem::exists(testDbPath))
        {
            std::filesystem::remove(testDbPath);
        }
    }

    void InitializeTestDatabase()
    {
        // Note: This assumes SqlDbHandler creates tables automatically
        // If not, you'll need to execute CREATE TABLE statements here
    }
};

// Test: Constructor and Destructor
TEST_F(SqlDbHandlerTest, ConstructorInitializesDatabase)
{
    // Arrange & Act - done in SetUp
    // Assert
    ASSERT_NE(handler, nullptr);
}

// Test: AddCompany with valid data
TEST_F(SqlDbHandlerTest, AddCompany_ValidData_AddsSuccessfully)
{
    // Arrange
    std::string companyName = "TechCorp";
    std::string sector = "Technology";

    // Act
    handler->AddCompany(companyName, sector);

    // Assert
    // We can verify by adding a job with this company and retrieving it
    handler->AddJob("Software Engineer", "Great job", companyName, 8, jobStatus::NotApplied);
    Jobs jobs = handler->GetJobsByStatus(jobStatus::NotApplied);

    ASSERT_EQ(jobs.size(), 1);
    EXPECT_EQ(jobs[0].companyName, companyName);
}

// Test: AddCompany with duplicate company name
TEST_F(SqlDbHandlerTest, AddCompany_DuplicateName_IgnoresDuplicate)
{
    // Arrange
    std::string companyName = "DuplicateCorp";
    std::string sector = "Finance";

    // Act
    handler->AddCompany(companyName, sector);
    handler->AddCompany(companyName, sector); // Add again

    // Assert - Should not throw exception
    SUCCEED();
}

// Test: AddJob with valid data
TEST_F(SqlDbHandlerTest, AddJob_ValidData_AddsSuccessfully)
{
    // Arrange
    std::string title = "Senior Developer";
    std::string description = "Amazing opportunity";
    std::string companyName = "DevCompany";
    int match = 9;
    jobStatus status = jobStatus::NotApplied;

    // Act
    handler->AddJob(title, description, companyName, match, status);
    Jobs jobs = handler->GetJobsByStatus(jobStatus::NotApplied);

    // Assert
    ASSERT_EQ(jobs.size(), 1);
    EXPECT_EQ(jobs[0].title, title);
    EXPECT_EQ(jobs[0].description, description);
    EXPECT_EQ(jobs[0].companyName, companyName);
    EXPECT_EQ(jobs[0].match, match);
    EXPECT_EQ(jobs[0].status, status);
}

// Test: AddJob with different statuses
TEST_F(SqlDbHandlerTest, AddJob_WithAppliedStatus_StoresCorrectly)
{
    // Arrange
    std::string title = "QA Engineer";
    std::string description = "Testing role";
    std::string companyName = "TestCorp";

    // Act
    handler->AddJob(title, description, companyName, 7, jobStatus::Applied);
    Jobs jobs = handler->GetJobsByStatus(jobStatus::Applied);

    // Assert
    ASSERT_EQ(jobs.size(), 1);
    EXPECT_EQ(jobs[0].status, jobStatus::Applied);
}

// Test: UpdateJobStatus from NotApplied to Applied
TEST_F(SqlDbHandlerTest, UpdateJobStatus_NotAppliedToApplied_UpdatesSuccessfully)
{
    // Arrange
    std::string title = "Backend Developer";
    handler->AddJob(title, "Backend role", "BackendCorp", 8, jobStatus::NotApplied);

    // Act
    handler->UpdateJobStatus(title, jobStatus::Applied);
    Jobs appliedJobs = handler->GetJobsByStatus(jobStatus::Applied);

    // Assert
    ASSERT_EQ(appliedJobs.size(), 1);
    EXPECT_EQ(appliedJobs[0].title, title);
    EXPECT_EQ(appliedJobs[0].status, jobStatus::Applied);
}

// Test: UpdateJobStatus through all stages
TEST_F(SqlDbHandlerTest, UpdateJobStatus_ThroughAllStages_UpdatesCorrectly)
{
    // Arrange
    std::string title = "Full Stack Developer";
    handler->AddJob(title, "Full stack role", "FullStackCorp", 10, jobStatus::NotApplied);

    // Act & Assert - NotApplied to Applied
    handler->UpdateJobStatus(title, jobStatus::Applied);
    Jobs jobs = handler->GetJobsByStatus(jobStatus::Applied);
    ASSERT_EQ(jobs.size(), 1);
    EXPECT_EQ(jobs[0].status, jobStatus::Applied);

    // Act & Assert - Applied to Interviewing
    handler->UpdateJobStatus(title, jobStatus::Interviewing);
    jobs = handler->GetJobsByStatus(jobStatus::Interviewing);
    ASSERT_EQ(jobs.size(), 1);
    EXPECT_EQ(jobs[0].status, jobStatus::Interviewing);

    // Act & Assert - Interviewing to Offered
    handler->UpdateJobStatus(title, jobStatus::Offered);
    jobs = handler->GetJobsByStatus(jobStatus::Offered);
    ASSERT_EQ(jobs.size(), 1);
    EXPECT_EQ(jobs[0].status, jobStatus::Offered);
}

// Test: UpdateJobStatus to Rejected marks job as inactive
TEST_F(SqlDbHandlerTest, UpdateJobStatus_ToRejected_MarksInactive)
{
    // Arrange
    std::string title = "Data Analyst";
    handler->AddJob(title, "Data role", "DataCorp", 6, jobStatus::Applied);

    // Act
    handler->UpdateJobStatus(title, jobStatus::Rejected);
    Jobs rejectedJobs = handler->GetJobsByStatus(jobStatus::Rejected);
    Jobs appliedJobs = handler->GetJobsByStatus(jobStatus::Applied);

    // Assert
    ASSERT_EQ(rejectedJobs.size(), 1);
    EXPECT_EQ(rejectedJobs[0].title, title);
    EXPECT_EQ(appliedJobs.size(), 0); // Should not appear in applied
}

// Test: GetJobsByStatus with NotApplied status
TEST_F(SqlDbHandlerTest, GetJobsByStatus_NotApplied_ReturnsOnlyNotAppliedJobs)
{
    // Arrange
    handler->AddJob("Job1", "Desc1", "Company1", 5, jobStatus::NotApplied);
    handler->AddJob("Job2", "Desc2", "Company2", 6, jobStatus::Applied);
    handler->AddJob("Job3", "Desc3", "Company3", 7, jobStatus::NotApplied);

    // Act
    Jobs jobs = handler->GetJobsByStatus(jobStatus::NotApplied);

    // Assert
    ASSERT_EQ(jobs.size(), 2);
}

// Test: GetJobsByStatus with Applied status
TEST_F(SqlDbHandlerTest, GetJobsByStatus_Applied_ReturnsOnlyAppliedJobs)
{
    // Arrange
    handler->AddJob("Job1", "Desc1", "Company1", 5, jobStatus::NotApplied);
    handler->AddJob("Job2", "Desc2", "Company2", 6, jobStatus::Applied);
    handler->AddJob("Job3", "Desc3", "Company3", 7, jobStatus::Applied);
    handler->AddJob("Job4", "Desc4", "Company4", 8, jobStatus::Interviewing);

    // Act
    Jobs jobs = handler->GetJobsByStatus(jobStatus::Applied);

    // Assert
    ASSERT_EQ(jobs.size(), 2);
}

// Test: GetJobsByStatus with Interviewing status
TEST_F(SqlDbHandlerTest, GetJobsByStatus_Interviewing_ReturnsOnlyInterviewingJobs)
{
    // Arrange
    handler->AddJob("Job1", "Desc1", "Company1", 8, jobStatus::Interviewing);
    handler->AddJob("Job2", "Desc2", "Company2", 9, jobStatus::Offered);

    // Act
    Jobs jobs = handler->GetJobsByStatus(jobStatus::Interviewing);

    // Assert
    ASSERT_EQ(jobs.size(), 1);
    EXPECT_EQ(jobs[0].status, jobStatus::Interviewing);
}

// Test: GetJobsByStatus with Offered status
TEST_F(SqlDbHandlerTest, GetJobsByStatus_Offered_ReturnsOnlyOfferedJobs)
{
    // Arrange
    handler->AddJob("Job1", "Desc1", "Company1", 10, jobStatus::Offered);
    handler->AddJob("Job2", "Desc2", "Company2", 9, jobStatus::Offered);
    handler->AddJob("Job3", "Desc3", "Company3", 8, jobStatus::Applied);

    // Act
    Jobs jobs = handler->GetJobsByStatus(jobStatus::Offered);

    // Assert
    ASSERT_EQ(jobs.size(), 2);
}

// Test: GetJobsByStatus with Rejected status
TEST_F(SqlDbHandlerTest, GetJobsByStatus_Rejected_ReturnsOnlyRejectedJobs)
{
    // Arrange
    handler->AddJob("Job1", "Desc1", "Company1", 5, jobStatus::Applied);
    handler->UpdateJobStatus("Job1", jobStatus::Rejected);
    handler->AddJob("Job2", "Desc2", "Company2", 6, jobStatus::Applied);

    // Act
    Jobs jobs = handler->GetJobsByStatus(jobStatus::Rejected);

    // Assert
    ASSERT_EQ(jobs.size(), 1);
    EXPECT_EQ(jobs[0].title, "Job1");
}

// Test: GetJobsByStatus with empty database
TEST_F(SqlDbHandlerTest, GetJobsByStatus_EmptyDatabase_ReturnsEmptyVector)
{
    // Arrange - fresh database from SetUp

    // Act
    Jobs jobs = handler->GetJobsByStatus(jobStatus::NotApplied);

    // Assert
    EXPECT_EQ(jobs.size(), 0);
}

// Test: Multiple jobs with same company
TEST_F(SqlDbHandlerTest, AddJob_MultipleJobsSameCompany_AllAddedSuccessfully)
{
    // Arrange
    std::string companyName = "MultiJobCorp";

    // Act
    handler->AddJob("Job1", "First job", companyName, 7, jobStatus::NotApplied);
    handler->AddJob("Job2", "Second job", companyName, 8, jobStatus::Applied);
    handler->AddJob("Job3", "Third job", companyName, 9, jobStatus::Interviewing);

    // Assert
    Jobs notApplied = handler->GetJobsByStatus(jobStatus::NotApplied);
    Jobs applied = handler->GetJobsByStatus(jobStatus::Applied);
    Jobs interviewing = handler->GetJobsByStatus(jobStatus::Interviewing);

    EXPECT_EQ(notApplied.size(), 1);
    EXPECT_EQ(applied.size(), 1);
    EXPECT_EQ(interviewing.size(), 1);
}

// Test: Match score validation
TEST_F(SqlDbHandlerTest, AddJob_VariousMatchScores_StoresCorrectly)
{
    // Arrange & Act
    handler->AddJob("Job1", "Low match", "Company1", 1, jobStatus::NotApplied);
    handler->AddJob("Job2", "Mid match", "Company2", 5, jobStatus::NotApplied);
    handler->AddJob("Job3", "High match", "Company3", 10, jobStatus::NotApplied);

    // Act
    Jobs jobs = handler->GetJobsByStatus(jobStatus::NotApplied);

    // Assert
    ASSERT_EQ(jobs.size(), 3);

    // Find each job and verify match score
    for (const auto& job : jobs)
    {
        if (job.title == "Job1")
            EXPECT_EQ(job.match, 1);
        else if (job.title == "Job2")
            EXPECT_EQ(job.match, 5);
        else if (job.title == "Job3")
            EXPECT_EQ(job.match, 10);
    }
}

// Test: UpdateJobStatus with non-existent job
TEST_F(SqlDbHandlerTest, UpdateJobStatus_NonExistentJob_HandlesGracefully)
{
    // Arrange
    std::string nonExistentTitle = "NonExistentJob";

    // Act & Assert - Should not throw or crash
    EXPECT_NO_THROW(handler->UpdateJobStatus(nonExistentTitle, jobStatus::Applied));
}

// Test: Job with empty description
TEST_F(SqlDbHandlerTest, AddJob_EmptyDescription_AddsSuccessfully)
{
    // Arrange
    std::string title = "Job With No Description";
    std::string description = "";
    std::string companyName = "MinimalCorp";

    // Act
    handler->AddJob(title, description, companyName, 5, jobStatus::NotApplied);
    Jobs jobs = handler->GetJobsByStatus(jobStatus::NotApplied);

    // Assert
    ASSERT_EQ(jobs.size(), 1);
    EXPECT_EQ(jobs[0].description, "");
}

// Main function for running tests
int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}