#include "DbContext.h"

// Advertencia C26495 corregida: 'retcode' se inicializa en la lista de inicializadores.
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
            // Se muestra el error en la consola de salida de Visual Studio para facilitar la depuración
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

// LÓGICA CORREGIDA: LOGIN O REGISTRO CON CAMPO DE CONTRASEÑA
int DbContext::LoginOrRegister(std::string username, std::string password) {
    int userId = -1; // -1: no encontrado/error, -2: contraseña incorrecta
    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) != SQL_SUCCESS) {
        CheckError(hDbc, SQL_HANDLE_DBC, "Stmt Alloc Failed");
        return -1;
    }

    // 1. Intentar encontrar al usuario y verificar su contraseña
    SQLCHAR querySelect[] = "SELECT UserID, PasswordHash FROM Users WHERE Username = ?";

    retcode = SQLPrepareA(hStmt, querySelect, SQL_NTS);
    if (!SQL_SUCCEEDED(retcode)) {
        CheckError(hStmt, SQL_HANDLE_STMT, "SELECT Prepare Failed");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return -1;
    }

    SQLLEN cbUsername = SQL_NTS;
    retcode = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, username.length(), 0, (SQLPOINTER)username.c_str(), 0, &cbUsername);
    if (!SQL_SUCCEEDED(retcode)) {
        CheckError(hStmt, SQL_HANDLE_STMT, "SELECT Bind Param Failed");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return -1;
    }

    retcode = SQLExecute(hStmt);
    if (SQL_SUCCEEDED(retcode)) {
        if (SQLFetch(hStmt) == SQL_SUCCESS) {
            // Usuario encontrado, verificar contraseña
            char storedPassword[256] = { 0 };
            SQLLEN passIndicator;

            SQLGetData(hStmt, 1, SQL_C_LONG, &userId, 0, NULL);
            SQLGetData(hStmt, 2, SQL_C_CHAR, storedPassword, sizeof(storedPassword), &passIndicator);

            // ADVERTENCIA: Se recomienda usar una función de hash para comparar contraseñas.
            if (password != std::string(storedPassword)) {
                userId = -2; // Contraseña incorrecta
            }
        }
    }
    else {
        CheckError(hStmt, SQL_HANDLE_STMT, "SELECT Execute Failed");
    }

    SQLFreeStmt(hStmt, SQL_CLOSE);

    // Si el usuario fue encontrado (login correcto o incorrecto), terminamos.
    if (userId != -1) {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return userId;
    }

    // 2. Si el usuario no existe (userId sigue en -1), se crea uno nuevo.
    // ADVERTENCIA: Almacenar contraseñas en texto plano es inseguro.
    SQLCHAR queryInsert[] = "INSERT INTO Users (Username, PasswordHash, CreatedAt) OUTPUT INSERTED.UserID VALUES (?, ?, GETDATE())";

    retcode = SQLPrepareA(hStmt, queryInsert, SQL_NTS);
    if (!SQL_SUCCEEDED(retcode)) {
        CheckError(hStmt, SQL_HANDLE_STMT, "INSERT Prepare Failed");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return -1;
    }

    // Enlazar username
    retcode = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, username.length(), 0, (SQLPOINTER)username.c_str(), 0, &cbUsername);
    if (!SQL_SUCCEEDED(retcode)) {
        CheckError(hStmt, SQL_HANDLE_STMT, "INSERT Bind Param 1 Failed");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return -1;
    }

    // Enlazar password
    SQLLEN cbPassword = SQL_NTS;
    retcode = SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, password.length(), 0, (SQLPOINTER)password.c_str(), 0, &cbPassword);
    if (!SQL_SUCCEEDED(retcode)) {
        CheckError(hStmt, SQL_HANDLE_STMT, "INSERT Bind Param 2 Failed");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return -1;
    }

    retcode = SQLExecute(hStmt);
    if (SQL_SUCCEEDED(retcode)) {
        if (SQLFetch(hStmt) == SQL_SUCCESS) {
            SQLGetData(hStmt, 1, SQL_C_LONG, &userId, 0, NULL);
        }
    }
    else {
        CheckError(hStmt, SQL_HANDLE_STMT, "INSERT Execute Failed");
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return userId;
}

// LÓGICA CORREGIDA: NOMBRES DE COLUMNA CORREGIDOS
bool DbContext::InsertScore(int userId, int scoreValue, int levelReached, bool isCompleted) {
    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) != SQL_SUCCESS) return false;
    
    // Usa UserID, ScoreValue, LevelReached, IsCompleted, AchievedAt
    SQLCHAR query[] = "INSERT INTO Scores (UserID, ScoreValue, LevelReached, IsCompleted, AchievedAt) VALUES (?, ?, ?, ?, GETDATE())";
    
    retcode = SQLPrepareA(hStmt, query, SQL_NTS);
    if (!SQL_SUCCEEDED(retcode)) {
        CheckError(hStmt, SQL_HANDLE_STMT, "Insert Score Prepare Failed");
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return false;
    }

    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &userId, 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &scoreValue, 0, NULL);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &levelReached, 0, NULL);
    
    SQLCHAR cIsCompleted = isCompleted ? 1 : 0;
    SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_BIT, SQL_BIT, 0, 0, &cIsCompleted, 0, NULL);
    
    retcode = SQLExecute(hStmt);

    bool success = SQL_SUCCEEDED(retcode);
    if (!success) {
        CheckError(hStmt, SQL_HANDLE_STMT, "Insert Score Execute Failed");
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return success;
}

// LÓGICA CORREGIDA: NOMBRES DE COLUMNA CORREGIDOS
std::vector<ScoreEntry> DbContext::GetTopScores() {
    std::vector<ScoreEntry> scores;
    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) != SQL_SUCCESS) return scores;

    // Usa u.UserID, s.UserID, s.ScoreValue
    std::string query = "SELECT TOP 8 u.Username, s.ScoreValue, s.LevelReached FROM Scores s JOIN Users u ON s.UserID = u.UserID ORDER BY s.ScoreValue DESC";
    retcode = SQLExecDirectA(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);

    if (SQL_SUCCEEDED(retcode)) {
        // Advertencia C6054 corregida: El búfer 'name' se inicializa con ceros.
        char name[50] = {0};
        int score, level;
        SQLLEN indicator;
        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            SQLGetData(hStmt, 1, SQL_C_CHAR, name, sizeof(name), &indicator);
            SQLGetData(hStmt, 2, SQL_C_LONG, &score, 0, NULL);
            SQLGetData(hStmt, 3, SQL_C_LONG, &level, 0, NULL);
            scores.push_back({ std::string(name), score, level });
        }
    } else {
        CheckError(hStmt, SQL_HANDLE_STMT, "Get Top Scores Failed");
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return scores;
}