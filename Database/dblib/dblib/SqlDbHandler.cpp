// Implementation of the SqlDbHandler class for managing jobs and companies
// using SQLite as the backing store.  This file provides simple helper
// functions to connect to the database, insert data and query jobs by
// status.  Note that error handling is minimal; production code should
// check return values and handle errors appropriately.

#include "pch.h"
#include "SqlDbHandler.h"

#include <iostream>

// Helper function to map a jobStatus enum to the three boolean fields in
// the Application table.  The Application table stores the progress of
// a job application using three integer columns: applied, interview and
// offer.  A value of 1 indicates that the candidate has reached that
// stage.  For example, for a candidate that is currently interviewing,
// both the applied and interview columns will be 1 while offer is 0.
static void statusToApplicationFlags(jobStatus status,
                                     int &applied,
                                     int &interview,
                                     int &offer)
{
    applied = 0;
    interview = 0;
    offer = 0;
    switch (status)
    {
    case jobStatus::NotApplied:
        // all flags remain 0
        break;
    case jobStatus::Applied:
        applied = 1;
        break;
    case jobStatus::Interviewing:
        applied = 1;
        interview = 1;
        break;
    case jobStatus::Offered:
        applied = 1;
        interview = 1;
        offer = 1;
        break;
    case jobStatus::Rejected:
        // Rejection is indicated by setting 'active' to 0 on the Job row,
        // but we still mark the progress achieved before rejection.
        applied = 1;
        interview = 1;
        // Do not set offer; rejection means no offer was accepted.
        break;
    }
}

SqlDbHandler::SqlDbHandler()
    : m_db(nullptr)
{
    // When constructed, attempt to connect to the database.  If the
    // connection fails, m_db will remain nullptr and operations will
    // effectively be no‑ops.
    ConnectToDatabase();
}

SqlDbHandler::~SqlDbHandler()
{
    // Ensure the database connection is closed on destruction.
    DisconnectFromDatabase();
}

void SqlDbHandler::ConnectToDatabase()
{
    if (m_db)
    {
        return;
    }

    // Use m_dbname as the database file.  On Windows this will be found in
    // the working directory of the process.  On other platforms you may
    // need to adjust the path.
    m_dbPath = m_dbname;
    int rc = sqlite3_open(m_dbPath.c_str(), &m_db);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Failed to open SQLite database: " << sqlite3_errmsg(m_db) << std::endl;
        sqlite3_close(m_db);
        m_db = nullptr;
    }
}

void SqlDbHandler::DisconnectFromDatabase()
{
    if (m_db)
    {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
}

void SqlDbHandler::AddCompany(const std::string &name, const std::string &sector)
{
    // Ensure the database is connected
    ConnectToDatabase();
    if (!m_db)
    {
        return;
    }

    // Use a prepared statement to insert the company.  The UNIQUE constraint
    // on company_name will prevent duplicates.  We ignore any error due
    // to a duplicate entry.
    const char *sql = "INSERT INTO Companies (company_name, sector, reapply) VALUES (?, ?, 0);";
    sqlite3_stmt *stmt = nullptr;
    int rc = sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);
    if (rc == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, sector.c_str(), -1, SQLITE_TRANSIENT);
        rc = sqlite3_step(stmt);
        // SQLITE_DONE indicates success.  If there is a constraint violation,
        // SQLITE_CONSTRAINT is returned; we ignore it.
    }
    sqlite3_finalize(stmt);
}

void SqlDbHandler::AddJob(const std::string &title,
                          const std::string &description,
                          const std::string &companyName,
                          const int &match,
                          const jobStatus &status)
{
    ConnectToDatabase();
    if (!m_db)
    {
        return;
    }

    // Start a transaction to ensure atomicity
    char *errmsg = nullptr;
    sqlite3_exec(m_db, "BEGIN TRANSACTION;", nullptr, nullptr, &errmsg);
    if (errmsg)
    {
        std::cerr << "Error starting transaction: " << errmsg << std::endl;
        sqlite3_free(errmsg);
    }

    // Ensure company exists (will ignore duplicate due to unique constraint)
    AddCompany(companyName, "");

    // Retrieve company_id
    int companyId = -1;
    const char *sqlCompany = "SELECT id FROM Companies WHERE company_name = ?;";
    sqlite3_stmt *companyStmt = nullptr;
    if (sqlite3_prepare_v2(m_db, sqlCompany, -1, &companyStmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_text(companyStmt, 1, companyName.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(companyStmt) == SQLITE_ROW)
        {
            companyId = sqlite3_column_int(companyStmt, 0);
        }
    }
    sqlite3_finalize(companyStmt);
    if (companyId < 0)
    {
        // Cannot insert job without company id
        sqlite3_exec(m_db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return;
    }

    // Insert into Application table
    int applied = 0, interview = 0, offer = 0;
    statusToApplicationFlags(status, applied, interview, offer);
    const char *sqlApp = "INSERT INTO Application (applied, interview, offer) VALUES (?, ?, ?);";
    sqlite3_stmt *appStmt = nullptr;
    int rc = sqlite3_prepare_v2(m_db, sqlApp, -1, &appStmt, nullptr);
    if (rc == SQLITE_OK)
    {
        sqlite3_bind_int(appStmt, 1, applied);
        sqlite3_bind_int(appStmt, 2, interview);
        sqlite3_bind_int(appStmt, 3, offer);
        rc = sqlite3_step(appStmt);
    }
    sqlite3_finalize(appStmt);
    if (rc != SQLITE_DONE)
    {
        sqlite3_exec(m_db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return;
    }
    // Retrieve application id
    int applicationId = (int)sqlite3_last_insert_rowid(m_db);

    // Insert into Jobs table
    const char *sqlJob = "INSERT INTO Jobs (job_title, company_id, application_id, job_description, suitability, active) VALUES (?, ?, ?, ?, ?, 1);";
    sqlite3_stmt *jobStmt = nullptr;
    rc = sqlite3_prepare_v2(m_db, sqlJob, -1, &jobStmt, nullptr);
    if (rc == SQLITE_OK)
    {
        sqlite3_bind_text(jobStmt, 1, title.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(jobStmt, 2, companyId);
        sqlite3_bind_int(jobStmt, 3, applicationId);
        sqlite3_bind_text(jobStmt, 4, description.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(jobStmt, 5, std::to_string(match).c_str(), -1, SQLITE_TRANSIENT);
        rc = sqlite3_step(jobStmt);
    }
    sqlite3_finalize(jobStmt);
    if (rc != SQLITE_DONE)
    {
        sqlite3_exec(m_db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return;
    }

    // Commit transaction
    sqlite3_exec(m_db, "COMMIT;", nullptr, nullptr, nullptr);
}

void SqlDbHandler::UpdateJobStatus(const std::string &title, const jobStatus &newStatus)
{
    ConnectToDatabase();
    if (!m_db)
    {
        return;
    }

    // Find the job and its application id
    const char *sqlJob = "SELECT id, application_id FROM Jobs WHERE job_title = ?;";
    sqlite3_stmt *jobStmt = nullptr;
    int rc = sqlite3_prepare_v2(m_db, sqlJob, -1, &jobStmt, nullptr);
    int jobId = -1;
    int applicationId = -1;
    if (rc == SQLITE_OK)
    {
        sqlite3_bind_text(jobStmt, 1, title.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(jobStmt) == SQLITE_ROW)
        {
            jobId = sqlite3_column_int(jobStmt, 0);
            applicationId = sqlite3_column_int(jobStmt, 1);
        }
    }
    sqlite3_finalize(jobStmt);
    if (jobId < 0 || applicationId < 0)
    {
        return;
    }

    // Map status to flags
    int applied = 0, interview = 0, offer = 0;
    statusToApplicationFlags(newStatus, applied, interview, offer);

    // Begin transaction
    sqlite3_exec(m_db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
    // Update Application
    const char *sqlApp = "UPDATE Application SET applied = ?, interview = ?, offer = ? WHERE id = ?;";
    sqlite3_stmt *appStmt = nullptr;
    rc = sqlite3_prepare_v2(m_db, sqlApp, -1, &appStmt, nullptr);
    if (rc == SQLITE_OK)
    {
        sqlite3_bind_int(appStmt, 1, applied);
        sqlite3_bind_int(appStmt, 2, interview);
        sqlite3_bind_int(appStmt, 3, offer);
        sqlite3_bind_int(appStmt, 4, applicationId);
        rc = sqlite3_step(appStmt);
    }
    sqlite3_finalize(appStmt);

    if (rc != SQLITE_DONE)
    {
        sqlite3_exec(m_db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return;
    }

    // If status is Rejected we mark job as inactive
    if (newStatus == jobStatus::Rejected)
    {
        const char *sqlInactive = "UPDATE Jobs SET active = 0 WHERE id = ?;";
        sqlite3_stmt *inStmt = nullptr;
        rc = sqlite3_prepare_v2(m_db, sqlInactive, -1, &inStmt, nullptr);
        if (rc == SQLITE_OK)
        {
            sqlite3_bind_int(inStmt, 1, jobId);
            rc = sqlite3_step(inStmt);
        }
        sqlite3_finalize(inStmt);
        if (rc != SQLITE_DONE)
        {
            sqlite3_exec(m_db, "ROLLBACK;", nullptr, nullptr, nullptr);
            return;
        }
    }

    // Commit transaction
    sqlite3_exec(m_db, "COMMIT;", nullptr, nullptr, nullptr);
}

Jobs SqlDbHandler::GetJobsByStatus(const jobStatus &status)
{
    Jobs results;
    ConnectToDatabase();
    if (!m_db)
    {
        return results;
    }

    // Build SQL query to select job_title, job_description, company_name, suitability and status
    std::string whereClause;
    switch (status)
    {
    case jobStatus::NotApplied:
        whereClause = "A.applied = 0 AND J.active = 1";
        break;
    case jobStatus::Applied:
        whereClause = "A.applied = 1 AND A.interview = 0 AND J.active = 1";
        break;
    case jobStatus::Interviewing:
        whereClause = "A.interview = 1 AND A.offer = 0 AND J.active = 1";
        break;
    case jobStatus::Offered:
        whereClause = "A.offer = 1 AND J.active = 1";
        break;
    case jobStatus::Rejected:
        whereClause = "J.active = 0";
        break;
    }
    std::string sql =
        "SELECT J.job_title, J.job_description, C.company_name, J.suitability, A.applied, A.interview, A.offer, J.active "
        "FROM Jobs J "
        "JOIN Companies C ON J.company_id = C.id "
        "JOIN Application A ON J.application_id = A.id "
        "WHERE " + whereClause + ";";
    sqlite3_stmt *stmt = nullptr;
    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        return results;
    }
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        Job job;
        job.title = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        job.description = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        job.companyName = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        // suitability stored as text; convert to int if possible
        const unsigned char *suitability = sqlite3_column_text(stmt, 3);
        if (suitability)
        {
            try
            {
                job.match = std::stoi(reinterpret_cast<const char *>(suitability));
            }
            catch (...)
            {
                job.match = 0;
            }
        }
        else
        {
            job.match = 0;
        }
        // Determine status based on flags
        int appliedFlag = sqlite3_column_int(stmt, 4);
        int interviewFlag = sqlite3_column_int(stmt, 5);
        int offerFlag = sqlite3_column_int(stmt, 6);
        int activeFlag = sqlite3_column_int(stmt, 7);
        if (activeFlag == 0)
        {
            job.status = jobStatus::Rejected;
        }
        else if (offerFlag)
        {
            job.status = jobStatus::Offered;
        }
        else if (interviewFlag)
        {
            job.status = jobStatus::Interviewing;
        }
        else if (appliedFlag)
        {
            job.status = jobStatus::Applied;
        }
        else
        {
            job.status = jobStatus::NotApplied;
        }
        results.push_back(job);
    }
    sqlite3_finalize(stmt);
    return results;
}
