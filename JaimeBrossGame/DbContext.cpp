#include "DbContext.h"

DbContext::DbContext() {
    hEnv = SQL_NULL_HENV;
    hDbc = SQL_NULL_HDBC;
    hStmt = SQL_NULL_HSTMT;
}

DbContext::~DbContext() {
    Disconnect();
}

void DbContext::CheckError(SQLHANDLE handle, SQLSMALLINT type, const char* msg) {
    SQLSMALLINT i = 0;
    SQLINTEGER native;
    SQLCHAR state[7];
    SQLCHAR text[256];
    SQLSMALLINT len;
    SQLRETURN ret;
    do {
        ret = SQLGetDiagRecA(type, handle, ++i, state, &native, text, sizeof(text), &len);
        if (SQL_SUCCEEDED(ret)) {
            std::cout << msg << ": " << state << ":" << i << ":" << native << ":" << text << std::endl;
        }
    } while (ret == SQL_SUCCESS);
}

bool DbContext::Connect() {
    if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) != SQL_SUCCESS) return false;
    if (SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0) != SQL_SUCCESS) return false;
    if (SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc) != SQL_SUCCESS) return false;
    SQLWCHAR connStr[1024];
    wcscpy_s(connStr, connectionString.c_str());
    retcode = SQLDriverConnectW(hDbc, NULL, connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
    if (!SQL_SUCCEEDED(retcode)) {
        CheckError(hDbc, SQL_HANDLE_DBC, "Connection Failed");
        return false;
    }
    return true;
}

void DbContext::Disconnect() {
    if (hStmt != SQL_NULL_HSTMT) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    if (hDbc != SQL_NULL_HDBC) {
        SQLDisconnect(hDbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    }
    if (hEnv != SQL_NULL_HENV) SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
}

// LÓGICA NUEVA: LOGIN O REGISTRO
int DbContext::LoginOrRegister(std::string username) {
    int userId = -1;
    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) != SQL_SUCCESS) return -1;

    // 1. Intentar encontrar al usuario
    std::string query = "SELECT Id FROM Users WHERE Username = '" + username + "'";
    retcode = SQLExecDirectA(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);

    if (SQL_SUCCEEDED(retcode)) {
        if (SQLFetch(hStmt) == SQL_SUCCESS) {
            SQLGetData(hStmt, 1, SQL_C_LONG, &userId, 0, NULL);
        }
    }
    SQLFreeStmt(hStmt, SQL_CLOSE);

    // 2. Si no existe (userId sigue siendo -1), insertarlo
    if (userId == -1) {
        query = "INSERT INTO Users (Username, CreatedAt) OUTPUT INSERTED.Id VALUES ('" + username + "', GETDATE())";
        retcode = SQLExecDirectA(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);
        if (SQL_SUCCEEDED(retcode)) {
            if (SQLFetch(hStmt) == SQL_SUCCESS) {
                SQLGetData(hStmt, 1, SQL_C_LONG, &userId, 0, NULL);
            }
        }
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return userId;
}

bool DbContext::InsertScore(int userId, int scoreValue, int levelReached, bool isCompleted) {
    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) != SQL_SUCCESS) return false;
    std::string query = "INSERT INTO Scores (UserId, Score, LevelReached, IsCompleted, DateAchieved) VALUES ("
        + std::to_string(userId) + ", " + std::to_string(scoreValue) + ", "
        + std::to_string(levelReached) + ", " + (isCompleted ? "1" : "0") + ", GETDATE())";

    retcode = SQLExecDirectA(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);
    bool success = SQL_SUCCEEDED(retcode);
    if (!success) CheckError(hStmt, SQL_HANDLE_STMT, "Insert Score Failed");
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return success;
}

std::vector<ScoreEntry> DbContext::GetTopScores() {
    std::vector<ScoreEntry> scores;
    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) != SQL_SUCCESS) return scores;

    std::string query = "SELECT TOP 8 u.Username, s.Score, s.LevelReached FROM Scores s JOIN Users u ON s.UserId = u.Id ORDER BY s.Score DESC";
    retcode = SQLExecDirectA(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);

    if (SQL_SUCCEEDED(retcode)) {
        char name[50];
        int score, level;
        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            SQLGetData(hStmt, 1, SQL_C_CHAR, name, sizeof(name), NULL);
            SQLGetData(hStmt, 2, SQL_C_LONG, &score, 0, NULL);
            SQLGetData(hStmt, 3, SQL_C_LONG, &level, 0, NULL);
            scores.push_back({ std::string(name), score, level });
        }
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return scores;
}