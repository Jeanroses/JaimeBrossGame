#pragma once
#include <string>
#include <vector>
#include <iostream>

struct ScoreEntry {
    std::string username;
    int score;
    int level;
};

// Protecciones para evitar conflictos entre Windows.h y Raylib
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#include <windows.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <sql.h>

// Des-definir macros conflictivas
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

    // Cadena de conexión CORREGIDA con la barra invertida escapada
    std::wstring connectionString = L"Driver={ODBC Driver 17 for SQL Server};"
        L"Server=LAPTOP-645QQ1GG\\SQLEXPRESS;" // <-- ¡AQUÍ ESTÁ EL CAMBIO!
        L"Database=DB_JaimeBrossGame;"
        L"Trusted_Connection=Yes;"
        L"TrustServerCertificate=Yes;";

    void CheckError(SQLHANDLE handle, SQLSMALLINT type, const char* msg);

public:
    DbContext();
    ~DbContext();

    bool Connect();
    void Disconnect();

    // Método actualizado para Login/Registro con contraseña
    int LoginOrRegister(std::string username, std::string password);

    bool InsertScore(int userId, int scoreValue, int levelReached, bool isCompleted);
    std::vector<ScoreEntry> GetTopScores();
};