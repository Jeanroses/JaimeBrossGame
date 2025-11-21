#include "DbContext.h"

DbContext::DbContext() : hEnv(SQL_NULL_HENV), hDbc(SQL_NULL_HDBC), hStmt(SQL_NULL_HSTMT), retcode(SQL_SUCCESS) {
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
            std::string error_msg = std::string(msg) + ": " + (const char*)state + ":" + std::to_string(i) + ":" + std::to_string(native) + ":" + (const char*)text + "\n";
            OutputDebugStringA(error_msg.c_str());
            std::cout << error_msg;
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

int DbContext::LoginOrRegister(std::string username, std::string password) {
    int userId = -1;

    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) != SQL_SUCCESS) return -1;

    SQLCHAR* queryLogin = (SQLCHAR*)"{CALL pa_ObtenerUserporNombre(?)}";

    if (!SQL_SUCCEEDED(SQLPrepareA(hStmt, queryLogin, SQL_NTS))) {
        CheckError(hStmt, SQL_HANDLE_STMT, "Login Prepare Failed");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return -1;
    }

    SQLLEN cbUsername = SQL_NTS;
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, username.length(), 0, (SQLPOINTER)username.c_str(), 0, &cbUsername);

    if (SQL_SUCCEEDED(SQLExecute(hStmt))) {
        if (SQLFetch(hStmt) == SQL_SUCCESS) {
            char storedPassword[256] = { 0 };
            SQLLEN indicator;

            SQLGetData(hStmt, 1, SQL_C_LONG, &userId, 0, NULL);
            SQLGetData(hStmt, 2, SQL_C_CHAR, storedPassword, sizeof(storedPassword), &indicator);

            if (password != std::string(storedPassword)) {
                userId = -2; 
            }
        }
    }
    else {
        CheckError(hStmt, SQL_HANDLE_STMT, "Login Execute Failed");
    }

    SQLFreeStmt(hStmt, SQL_CLOSE);
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);

    if (userId != -1) return userId; 


    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) != SQL_SUCCESS) return -1;

    SQLCHAR* queryRegister = (SQLCHAR*)"{CALL pa_CrearUser(?, ?)}";

    if (!SQL_SUCCEEDED(SQLPrepareA(hStmt, queryRegister, SQL_NTS))) {
        CheckError(hStmt, SQL_HANDLE_STMT, "Register Prepare Failed");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return -1;
    }

    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, username.length(), 0, (SQLPOINTER)username.c_str(), 0, &cbUsername);
    SQLLEN cbPassword = SQL_NTS;
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, password.length(), 0, (SQLPOINTER)password.c_str(), 0, &cbPassword);

    if (SQL_SUCCEEDED(SQLExecute(hStmt))) {
        if (SQLFetch(hStmt) == SQL_SUCCESS) {
            SQLGetData(hStmt, 1, SQL_C_LONG, &userId, 0, NULL);
        }
    }
    else {
        CheckError(hStmt, SQL_HANDLE_STMT, "Register Execute Failed");
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return userId;
}

bool DbContext::InsertScore(int userId, int scoreValue, int levelReached, bool isCompleted) {
    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) != SQL_SUCCESS) return false;

    SQLCHAR* query = (SQLCHAR*)"{CALL pa_InsertarScores(?, ?, ?, ?)}";

    if (!SQL_SUCCEEDED(SQLPrepareA(hStmt, query, SQL_NTS))) {
        CheckError(hStmt, SQL_HANDLE_STMT, "InsertScore Prepare Failed");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return false;
    }

    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &userId, 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &scoreValue, 0, NULL);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &levelReached, 0, NULL);

    unsigned char cIsCompleted = isCompleted ? 1 : 0;
    SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_BIT, SQL_BIT, 0, 0, &cIsCompleted, 0, NULL);

    retcode = SQLExecute(hStmt);
    bool success = SQL_SUCCEEDED(retcode);

    if (!success) {
        CheckError(hStmt, SQL_HANDLE_STMT, "InsertScore Execute Failed");
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return success;
}

std::vector<ScoreEntry> DbContext::GetTopScores() {
    std::vector<ScoreEntry> scores;
    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) != SQL_SUCCESS) return scores;

    SQLCHAR* query = (SQLCHAR*)"{CALL pa_ObtenerScores(?)}";

    if (!SQL_SUCCEEDED(SQLPrepareA(hStmt, query, SQL_NTS))) {
        CheckError(hStmt, SQL_HANDLE_STMT, "GetScores Prepare Failed");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return scores;
    }

    int topN = 8;
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &topN, 0, NULL);

    if (SQL_SUCCEEDED(SQLExecute(hStmt))) {
        char name[50] = { 0 };
        int score, level;
        SQLLEN indicator;
        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            SQLGetData(hStmt, 1, SQL_C_CHAR, name, sizeof(name), &indicator);
            SQLGetData(hStmt, 2, SQL_C_LONG, &score, 0, NULL);
            SQLGetData(hStmt, 3, SQL_C_LONG, &level, 0, NULL);
            scores.push_back({ std::string(name), score, level });
        }
    }
    else {
        CheckError(hStmt, SQL_HANDLE_STMT, "GetScores Execute Failed");
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return scores;
}