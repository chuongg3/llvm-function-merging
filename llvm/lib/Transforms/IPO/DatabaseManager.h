#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <string>
#include <sqlite3.h>
#include <iostream>

using namespace std;

class DatabaseManager {
public:
    // Constructors 
    DatabaseManager() {
        dbLocation = "/home_alt/m19364tg/ThirdYearProject/scripts/log/database.db";
        DB = nullptr;
    }
    DatabaseManager(std::string inputLocation) {
        dbLocation = inputLocation;
        DB = nullptr;
    }

    // Destructor
    ~DatabaseManager() {
        sqlite3_close(DB);
    }

    // Prepare statement
    // int prepareStatement(string query, sqlite3_stmt** stmt, int maxLen = -1);
    int prepareStatement(string query, sqlite3_stmt** stmt, int maxLen = -1) {
        int exit = sqlite3_prepare_v2(DB, query.c_str(), -1, stmt, nullptr);
        if (exit != SQLITE_OK) {
            std::cerr << "Error preparing query: " << sqlite3_errmsg(DB) << std::endl;
            return exit;
        }
        return exit;
    }

    // int executeQuery(string query, sqlite3_stmt** stmt);
    // Execute a query
    int executeQuery(string query, sqlite3_stmt** stmt) {
        int exit = prepareStatement(query, stmt);
        if (exit != SQLITE_OK) {
            return exit;
        }

        // Execute the query
        exit = sqlite3_step(*stmt);
        if (exit != SQLITE_ROW) {
            std::cerr << "Error executing query: " << sqlite3_errmsg(DB) << std::endl;
            sqlite3_finalize(*stmt);
            return exit;
        }

        return SQLITE_OK;
    }

    // Opens the database from a location
    // Creates tables if database is empty
    int openDB() {
        // Open or create the database
        int exit = sqlite3_open(dbLocation.c_str(), &DB);
        if (exit) {
            std::cerr << "Error opening database: " << sqlite3_errmsg(DB) << std::endl;
            return -1;
        }
        std::cout << "Opened Database Successfully!" << std::endl;
        
        if(initialiseTables() != SQLITE_OK) {
            return -1;
        }

        std::cout << "Database is ready to be used\n";
        return 0;
    }

    // Closes the database
    void closeDB() {
        sqlite3_close(DB);
    }

    sqlite3 * getDB() {
        return DB;
    }

    bool insertBenchmarkName(string item) {
        // SQL insert statement with a placeholder for the value
        std::string sql = "INSERT INTO Benchmarks (BenchmarkName) VALUES (?);";
        sqlite3_stmt* stmt = nullptr;

        // Prepare the SQL statement
        int rc = sqlite3_prepare_v2(DB, sql.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "Error preparing statement: " << sqlite3_errmsg(DB) << std::endl;
            return false;
        }

        // Bind the string value to the first placeholder (index 1)
        rc = sqlite3_bind_text(stmt, 1, item.c_str(), -1, SQLITE_STATIC);
        if (rc != SQLITE_OK) {
            std::cerr << "Error binding value: " << sqlite3_errmsg(DB) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }

        // Execute the statement
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_CONSTRAINT) {
            std::cerr << "Primary Key already exists: " << sqlite3_errmsg(DB) << std::endl;
            sqlite3_finalize(stmt);
            return true;
        }
        
        else if (rc != SQLITE_DONE) {
            std::cerr << "Error inserting benchmark name: " << sqlite3_errmsg(DB) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }

        // Finalize the prepared statement
        sqlite3_finalize(stmt);

        // std::cout << "Row inserted successfully!" << std::endl;
        return true;
    }

    bool insertFunctionDetail(int benchmarkID, int FunctionID, string FunctionName, double FingerprintSize, double EstimatedSize) {
        // SQL insert statement with a placeholder for the value
        std::string sql = "INSERT INTO Functions (BenchmarkID, FunctionID, FunctionName, FingerprintSize, EstimatedSize) VALUES (?, ?, ?, ?, ?);";
        sqlite3_stmt* stmt = nullptr;

        // Prepare the SQL statement
        int rc = sqlite3_prepare_v2(DB, sql.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "Error preparing statement: " << sqlite3_errmsg(DB) << std::endl;
            return false;
        }

        // Bind the first placeholder (?) to the Benchmark ID
        rc = sqlite3_bind_int(stmt, 1, benchmarkID);
        if (rc != SQLITE_OK) {
            std::cerr << "Error binding ID: " << sqlite3_errmsg(DB) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }

        // Bind the second placeholder (?) to the Function ID
        rc = sqlite3_bind_int(stmt, 2, FunctionID);
        if (rc != SQLITE_OK) {
            std::cerr << "Error binding ID: " << sqlite3_errmsg(DB) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }

        // Bind the third placeholder (?) to the FunctionName
        rc = sqlite3_bind_text(stmt, 3, FunctionName.c_str(), -1, SQLITE_STATIC);
        if (rc != SQLITE_OK) {
            std::cerr << "Error binding value: " << sqlite3_errmsg(DB) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }

        // Bind the fourth placeholder (?) to the double Value
        rc = sqlite3_bind_double(stmt, 4, FingerprintSize);
        if (rc != SQLITE_OK) {
            std::cerr << "Error binding Value: " << sqlite3_errmsg(DB) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }

        // Bind the fifth placeholder (?) to the double Value
        rc = sqlite3_bind_double(stmt, 5, EstimatedSize);
        if (rc != SQLITE_OK) {
            std::cerr << "Error binding Value: " << sqlite3_errmsg(DB) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }

        // Execute the statement
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            std::cerr << "Error inserting data: " << sqlite3_errmsg(DB) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }

        // Finalize the prepared statement
        sqlite3_finalize(stmt);

        // std::cout << "Row inserted successfully!" << std::endl;
        return true;
    }

    bool insertFunctionPairDetail(int benchmarkID, int function1ID, int function2ID, string Technique, double AlignmentScore, double FrequencyDistance, double MinHashDistance, int MergeSuccessful, double MergeEstimatedSize) {
        std::cout << "Inserting Function Pair Details\n" << std::endl;
        // SQL insert statement with a placeholder for the value
        std::string sql = "INSERT INTO FunctionPairs (BenchmarkID, Function1ID, Function2ID, Technique, AlignmentScore, FrequencyDistance, MinHashDistance, MergeSuccessful, MergedEstimatedSize) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";
        sqlite3_stmt* stmt = nullptr;

        // Prepare the SQL statement
        int rc = sqlite3_prepare_v2(DB, sql.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "Error preparing statement: " << sqlite3_errmsg(DB) << std::endl;
            return false;
        }

        // Bind the first placeholder (?) to the Benchmark ID
        rc = sqlite3_bind_int(stmt, 1, benchmarkID);
        if (rc != SQLITE_OK) {
            std::cerr << "Error binding ID: " << sqlite3_errmsg(DB) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }

        // Bind the second placeholder (?) to the Function1 ID
        rc = sqlite3_bind_int(stmt, 2, function1ID);
        if (rc != SQLITE_OK) {
            std::cerr << "Error binding ID: " << sqlite3_errmsg(DB) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }

        // Bind the third placeholder (?) to the Function2 ID
        rc = sqlite3_bind_int(stmt, 3, function2ID);
        if (rc != SQLITE_OK) {
            std::cerr << "Error binding ID: " << sqlite3_errmsg(DB) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }

        // Bind the fourth placeholder (?) to the Merging Technique
        rc = sqlite3_bind_text(stmt, 4, Technique.c_str(), -1, SQLITE_STATIC);
        if (rc != SQLITE_OK) {
            std::cerr << "Error binding ID: " << sqlite3_errmsg(DB) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }

        // Bind the fifth placeholder (?) to the AlignmentScore
        rc = sqlite3_bind_double(stmt, 5, AlignmentScore);
        if (rc != SQLITE_OK) {
            std::cerr << "Error binding ID: " << sqlite3_errmsg(DB) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }

        // Bind the sixth placeholder (?) to the FrequencyDistance
        rc = sqlite3_bind_double(stmt, 6, FrequencyDistance);
        if (rc != SQLITE_OK) {
            std::cerr << "Error binding ID: " << sqlite3_errmsg(DB) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }

        // Bind the seventh placeholder (?) to the MinHashDistance
        rc = sqlite3_bind_double(stmt, 7, MinHashDistance);
        if (rc != SQLITE_OK) {
            std::cerr << "Error binding ID: " << sqlite3_errmsg(DB) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }

        // Bind the eighth placeholder (?) to the MergeSuccessful
        rc = sqlite3_bind_int(stmt, 8, MergeSuccessful);
        if (rc != SQLITE_OK) {
            std::cerr << "Error binding ID: " << sqlite3_errmsg(DB) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }

        // Bind the ninth placeholder (?) to the MergeEstimatedSize
        rc = sqlite3_bind_double(stmt, 9, MergeEstimatedSize);
        if (rc != SQLITE_OK) {
            std::cerr << "Error binding ID: " << sqlite3_errmsg(DB) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }

        // Execute the statement
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            std::cerr << "Error inserting data: " << sqlite3_errmsg(DB) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }

        // Finalize the prepared statement
        sqlite3_finalize(stmt);

        // std::cout << "Row inserted successfully!" << std::endl;
        return true;
    }

    int getBenchmarkID(string BenchmarkName) {
        sqlite3_stmt* stmt;
        std::string sql = "SELECT ROWID FROM Benchmarks WHERE BenchmarkName = ?";

        // Prepare the SQL statement
        int rc = sqlite3_prepare_v2(DB, sql.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(DB) << std::endl;
            return -1;
        }

        // Bind the primary key value
        rc = sqlite3_bind_text(stmt, 1, BenchmarkName.c_str(), -1, SQLITE_STATIC);

        // Execute the statement and get the row index
        int rowIndex = -1;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            rowIndex = sqlite3_column_int(stmt, 0);
        }

        // Finalize the statement
        sqlite3_finalize(stmt);

        return rowIndex;
    }

private:
    string dbLocation;
    sqlite3* DB;

    int initialiseTables() {
        const char* sqlCreateSoftwareTable = 
            "CREATE TABLE IF NOT EXISTS Benchmarks (\n"
            "    BenchmarkName TEXT NOT NULL PRIMARY KEY\n"
            ");\n";

        const char* sqlCreateFunctionTable = 
            "CREATE TABLE IF NOT EXISTS Functions (\n"
            "    BenchmarkID INTEGER NOT NULL,\n"
            "    FunctionID INTEGER NOT NULL,\n"
            "    FunctionName TEXT,\n"
            "    FingerprintSize REAL,\n"
            "    EstimatedSize REAL,\n"
            "    Encoding BLOB,\n"
            "    PRIMARY KEY (BenchmarkID, FunctionID),\n"
            "    FOREIGN KEY (BenchmarkID) REFERENCES Benchmarks(ROWID)\n"
            ");\n";

        const char* sqlCreatePairwiseTable = 
            "CREATE TABLE IF NOT EXISTS FunctionPairs (\n"
            "    BenchmarkID INTEGER NOT NULL,\n"
            "    Function1ID INTEGER NOT NULL,\n"
            "    Function2ID INTEGER NOT NULL,\n"
            "    Technique TEXT NOT NULL,\n"
            "    AlignmentScore REAL,\n"
            "    FrequencyDistance REAL,\n"
            "    MinHashDistance REAL,\n"
            "    MergeSuccessful BOOLEAN,\n"
            "    MergedEstimatedSize INTEGER,\n"
            "    MergedLLVMIR TEXT,\n"
            "    MergedEncoding TEXT,\n"
            "    PRIMARY KEY (BenchmarkID, Function1ID, Function2ID, Technique),\n"
            "    FOREIGN KEY (BenchmarkID) REFERENCES Benchmarks(ROWID),\n"
            "    FOREIGN KEY (Function1ID) REFERENCES Functions(FunctionID),\n"
            "    FOREIGN KEY (Function2ID) REFERENCES Functions(FunctionID)\n"
            ");\n";

        char* errMsg = nullptr;
        int rc;
// std::cerr << "Creating the Benchmarks Table\n";
        rc = sqlite3_exec(DB, sqlCreateSoftwareTable, 0, 0, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            return rc;
        }

// std::cerr << "Creating the Functions Table\n";
        rc = sqlite3_exec(DB, sqlCreateFunctionTable, 0, 0, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            return rc;
        }

// std::cerr << "Creating the Pairwise Table\n";
        rc = sqlite3_exec(DB, sqlCreatePairwiseTable, 0, 0, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            return rc;
        }
        return rc;
    }
};

#endif // DATABASEMANAGER_H