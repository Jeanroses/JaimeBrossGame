#pragma once
#include <windows.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <sql.h>
#include <string>
#include <vector>
#include <iostream>

// Estructura simple para devolver datos (similar a una Entidad)
struct ScoreEntry {
    std::string username;
    int score;
    int level;
};

class DbContext
{
private:
    // Handles de ODBC (Manejadores de conexión)
    SQLHENV hEnv;
    SQLHDBC hDbc;
    SQLHSTMT hStmt;
    SQLRETURN retcode;

    // Tu cadena de conexión
    // NOTA: Ajusta SERVER si usas SQLExpress (ej: .\\SQLEXPRESS o localhost)
    std::wstring connectionString = L"Server=DESKTOP-U2NJFRP\\SQLEXPRESS;Database=DB_JaimeBrossGame;Trusted_Connection=True;TrustServerCertificate=True;";

    void CheckError(SQLHANDLE handle, SQLSMALLINT type, const char* msg);

public:
    DbContext();
    ~DbContext();

    bool Connect();
    void Disconnect();

    // Métodos equivalentes a tus DbSet.Add() / SaveChanges()
    bool InsertScore(int userId, int scoreValue, int levelReached, bool isCompleted);

    // Método equivalente a una consulta LINQ .ToList()
    std::vector<ScoreEntry> GetTopScores();
};