#include "DbContext.h"
#include <format> 

DbContext::DbContext() {
    hEnv = SQL_NULL_HENV;
    hDbc = SQL_NULL_HDBC;
    hStmt = SQL_NULL_HSTMT;
}

DbContext::~DbContext() {
    Disconnect();
}

bool DbContext::Connect() {
    // 1. Asignar Environment Handle
    if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) != SQL_SUCCESS) return false;
    if (SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0) != SQL_SUCCESS) return false;

    // 2. Asignar Connection Handle
    if (SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc) != SQL_SUCCESS) return false;

    // 3. Conectar a SQL Server
    SQLWCHAR outConnStr[1024];
    SQLSMALLINT outConnStrLen;

    // Casting necesario para strings anchos (wstring)
    retcode = SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)connectionString.c_str(), SQL_NTS,
        outConnStr, 1024, &outConnStrLen, SQL_DRIVER_NOPROMPT);

    if (SQL_SUCCEEDED(retcode)) {
        std::cout << "Conectado a la Base de Datos exitosamente." << std::endl;
        return true;
    }
    else {
        CheckError(hDbc, SQL_HANDLE_DBC, "Error al conectar");
        return false;
    }
}

void DbContext::Disconnect() {
    if (hStmt != SQL_NULL_HSTMT) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    if (hDbc != SQL_NULL_HDBC) {
        SQLDisconnect(hDbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    }
    if (hEnv != SQL_NULL_HENV) SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
}

// INSERTAR PUNTAJE
bool DbContext::InsertScore(int userId, int scoreValue, int levelReached, bool isCompleted) {
    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) != SQL_SUCCESS) return false;

    // Construimos la query SQL (En C# EF esto es automático, aquí es manual)
    // Nota: En producción real, usarías parámetros (?) para evitar SQL Injection.
    // Usamos std::format (C++20) o string append.
    std::string query = "INSERT INTO Scores (UserID, ScoreValue, LevelReached, IsCompleted) VALUES (" +
        std::to_string(userId) + ", " +
        std::to_string(scoreValue) + ", " +
        std::to_string(levelReached) + ", " +
        std::to_string(isCompleted ? 1 : 0) + ")";

    // Convertir a wstring para ODBC
    std::wstring wQuery(query.begin(), query.end());

    retcode = SQLExecDirect(hStmt, (SQLWCHAR*)wQuery.c_str(), SQL_NTS);

    bool success = SQL_SUCCEEDED(retcode);
    if (!success) CheckError(hStmt, SQL_HANDLE_STMT, "Error al insertar score");

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt); // Liberar el statement
    hStmt = SQL_NULL_HSTMT;
    return success;
}

// OBTENER TOP SCORES (SELECT)
std::vector<ScoreEntry> DbContext::GetTopScores() {
    std::vector<ScoreEntry> scores;
    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) != SQL_SUCCESS) return scores;

    // Query con JOIN (Igual que la que te di en el diseño SQL)
    std::wstring query = L"SELECT TOP 10 u.Username, s.ScoreValue, s.LevelReached FROM Scores s INNER JOIN Users u ON s.UserID = u.UserID ORDER BY s.ScoreValue DESC";

    if (SQL_SUCCEEDED(SQLExecDirect(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS))) {
        char username[50];
        int score;
        int level;
        SQLLEN indicator;

        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            // Obtenemos columna 1 (Username), 2 (Score), 3 (Level)
            SQLGetData(hStmt, 1, SQL_C_CHAR, username, sizeof(username), &indicator);
            SQLGetData(hStmt, 2, SQL_C_LONG, &score, 0, &indicator);
            SQLGetData(hStmt, 3, SQL_C_LONG, &level, 0, &indicator);

            scores.push_back({ std::string(username), score, level });
        }
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    hStmt = SQL_NULL_HSTMT;
    return scores;
}

// Manejo de errores básico
void DbContext::CheckError(SQLHANDLE handle, SQLSMALLINT type, const char* msg) {
    SQLSMALLINT i = 0;
    SQLINTEGER native;
    SQLWCHAR state[7];
    SQLWCHAR text[256];
    SQLSMALLINT len;
    SQLRETURN ret;

    std::cout << msg << std::endl;
    do {
        ret = SQLGetDiagRec(type, handle, ++i, state, &native, text, sizeof(text) / sizeof(SQLWCHAR), &len);
        if (SQL_SUCCEEDED(ret)) {
            std::wcout << L"Mensaje: " << text << L" | Estado: " << state << std::endl;
        }
    } while (ret == SQL_SUCCESS);
}