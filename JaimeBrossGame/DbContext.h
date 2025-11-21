#pragma once
#include <string>
#include <vector>
#include <iostream>

struct ScoreEntry {
    std::string username;
    int score;
    int level;
};

#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#include <windows.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <sql.h>

#undef PlaySound
#undef DrawText
#undef DrawTextEx
#undef CloseWindow
#undef ShowCursor
#undef Rectangle

class DbContext
{
private:
    SQLHENV hEnv;
    SQLHDBC hDbc;
    SQLHSTMT hStmt;
    SQLRETURN retcode;

    // Cadena de conexión 
    std::wstring connectionString = L"Driver={ODBC Driver 18 for SQL Server};"
        L"Server=DESKTOP-U2NJFRP\\SQLEXPRESS;"
        L"Database=DB_JaimeBrossGame;"
        L"Trusted_Connection=Yes;"
        L"TrustServerCertificate=Yes;";

    void CheckError(SQLHANDLE handle, SQLSMALLINT type, const char* msg);

public:
    DbContext();
    ~DbContext();

    bool Connect();
    void Disconnect();

    int LoginOrRegister(std::string username, std::string password);

    bool InsertScore(int userId, int scoreValue, int levelReached, bool isCompleted);
    std::vector<ScoreEntry> GetTopScores();
};