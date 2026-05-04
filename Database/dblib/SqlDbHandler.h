#pragma once
#include <string>
#include <vector>
#include "sqlite3.h"

// DLL export/import macro
#ifdef DBLIB_EXPORTS
#define DBLIB_API __declspec(dllexport)
#else
#define DBLIB_API __declspec(dllimport)
#endif

namespace JobFinderDB
{
	struct Job;
	inline const std::string ROOT = "E:\\___GitHub_WorkArea\\AI_Job_Finder\\flags\\";
	inline const std::string DB_PATH = ROOT + "Database";
	inline constexpr const char* DB_NAME = "Jobs.db";
	using Jobs = std::vector<Job>;

	enum class jobStatus
	{
		NotApplied		= 0,
		Applied			= 1,
		Interviewing	= 2,
		Offered			= 3,
		Rejected		= 4
	};

	struct Job
	{
		std::string title;
		std::string description;
		std::string companyName;
		int match;
		jobStatus status;
	};

	class DBLIB_API SqlDbHandler
	{
	public:
		SqlDbHandler();
		~SqlDbHandler();

		void AddCompany(const std::string& name, const std::string& sector);
		void AddJob(const std::string& title, const std::string& description, 
		           const std::string& companyName, const int& match, 
		           const jobStatus& status = jobStatus::NotApplied);
		void UpdateJobStatus(const std::string& title, const jobStatus& newStatus);
		Jobs GetJobsByStatus(const jobStatus& status);

	private:
		void ConnectToDatabase();
		void DisconnectFromDatabase();
		
		sqlite3* m_db;
		std::string m_dbPath;
	};
}

