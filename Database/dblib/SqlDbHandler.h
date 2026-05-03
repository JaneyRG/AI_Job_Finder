#pragma once
#include <string>
#include <vector>
#include "sqlite3.h"

#define ROOT_PATH "E:\\___GitHub_WorkArea\\AI_Job_Finder\\"
inline constexpr const char* ROOT = ROOT_PATH;
inline constexpr const char* DB_PATH = ROOT_PATH "Database";

namespace JobFinderDB
{
	struct Job; // Forward declaration of the Job struct to be used in the Jobs type alias.
	inline constexpr const char* DB_NAME = "Jobs.db"; // Type alias for a string representing the name of the database.
	using Jobs = std::vector<Job>; // Type alias for a vector of Job structs, representing a list of job listings.

	// Enum to represent the status of a job application.
	// the numbers is how the status will be stored in the database, 
	// for example, NotApplied will be stored as 0, Applied as 1, and so on.
	enum class jobStatus
	{
		NotApplied		= 0,
		Applied			= 1,
		Interviewing	= 2,
		Offered			= 3,
		Rejected		= 4
	};

	// Struct to represent a job listing, including the title, description, company name, match score, and application status.
	struct Job
	{
		std::string title;
		std::string description;
		std::string companyName;
		int match; // is this job a match for the user? 1 to 10
		jobStatus status; // the current status of the job application
	};


	// Class to handle interactions with the SQL database for managing companies and job applications.
	class SqlDbHandler
	{
	public:
			SqlDbHandler();
			~SqlDbHandler();

			// Adds a company to the database with the given name and sector.
			void AddCompany(
				const std::string& name, 
				const std::string& sector
			);

			// Adds a job to the database with the given title, description, company name, and match score.
			void AddJob(
				const std::string& title,
				const std::string& description,
				const std::string& companyName,
				const int& match, // is this job a match for the user? 1 to 10
				const jobStatus& status = jobStatus::NotApplied // the current status of the job application
			);

			// Updates the status of a job application in the database for the given job title.
			void UpdateJobStatus(
				const std::string& title,
				const jobStatus& newStatus
			);

			// Retrieves a list of jobs from the database that have the specified status.
			Jobs GetJobsByStatus(
				const jobStatus& status
			);

	private:
		void ConnectToDatabase(); // Establishes a connection to the SQL database.
		void DisconnectFromDatabase(); // Closes the connection to the SQL database.
		
		sqlite3* m_db;                    // SQLite database connection handle
		std::string m_dbPath;             // Path to the SQLite database file
	};

} // namespace JobFinderDB

