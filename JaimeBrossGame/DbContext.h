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

    // Cadena de conexión
    std::wstring connectionString = L"Server=DESKTOP-U2NJFRP\\SQLEXPRESS;Database=DB_JaimeBrossGame;Trusted_Connection=True;TrustServerCertificate=True;";

    void CheckError(SQLHANDLE handle, SQLSMALLINT type, const char* msg);

public:
    DbContext();
    ~DbContext();

    bool Connect();
    void Disconnect();

    // Nuevo método para Login/Registro
    int LoginOrRegister(std::string username);

    bool InsertScore(int userId, int scoreValue, int levelReached, bool isCompleted);
    std::vector<ScoreEntry> GetTopScores();
};