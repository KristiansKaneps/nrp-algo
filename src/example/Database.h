#ifndef DATABASE_H
#define DATABASE_H

#include <cassert>
#include <iostream>
#include <libpq-fe.h>

class Database {
public:
    Database() : m_ConnClosed(false),
                 m_Conn(PQconnectdb("postgresql://postgres:root@localhost/shiftapp")) {
        if (PQstatus(m_Conn) != CONNECTION_OK) {
            std::cerr << PQerrorMessage(m_Conn) << std::endl;
            m_ConnClosed = true;
            PQfinish(m_Conn);
            return;
        }

        PGresult *res = PQexec(m_Conn, "SET CLIENT_ENCODING TO 'UTF8'");
        PQclear(res);

        res = PQexec(m_Conn, "SELECT 1");
        assert(atoi(PQgetvalue(res, 0, 0)) == 1 && "Test query failed.");
        PQclear(res);
    }

    ~Database() { close(); }

    [[nodiscard]] const char *errorMessage() const { return PQerrorMessage(m_Conn); }

    void close() {
        clearRes();
        m_ActiveTransactionCount = 0;
        if (m_ConnClosed) return;
        m_ConnClosed = true;
        PQfinish(m_Conn);
    }

    void printTables() const {
        PGresult *res = PQexec(
            m_Conn,
            "SELECT table_schema || '.' || table_name FROM information_schema.tables WHERE table_type = 'BASE TABLE' AND table_schema NOT IN ('pg_catalog', 'information_schema')");
        if (PQresultStatus(res) != PGRES_TUPLES_OK) [[unlikely]] {
            PQclear(res);
            return;
        }
        for (int i = 0; i < PQntuples(res); i++) {
            for (int j = 0; j < PQnfields(res); j++)
                std::cout << PQgetvalue(res, i, j) << std::endl;
        }
        PQclear(res);
    }

    void startTransaction() {
        PGresult *res = PQexec(m_Conn, "BEGIN");
        if (PQresultStatus(res) != PGRES_COMMAND_OK) [[unlikely]]
            std::cerr << "Failed to begin transaction: " << PQerrorMessage(m_Conn) << std::endl;
        else
            m_ActiveTransactionCount += 1;
        PQclear(res);
    }

    void endTransaction() {
        if (m_ActiveTransactionCount == 0) [[unlikely]] return;
        PGresult *res = PQexec(m_Conn, "END");
        if (PQresultStatus(res) != PGRES_COMMAND_OK) [[unlikely]]
            std::cerr << "Failed to end transaction: " << PQerrorMessage(m_Conn) << std::endl;
        else
            m_ActiveTransactionCount -= 1;
        PQclear(res);
    }

    void commit() {
        PGresult *res = PQexec(m_Conn, "COMMIT");
        if (PQresultStatus(res) != PGRES_COMMAND_OK) [[unlikely]]
            std::cerr << "Failed to commit: " << PQerrorMessage(m_Conn) << std::endl;
        PQclear(res);
    }

    void execute(const std::string& query) { execute(query.c_str()); }

    void execute(const char *query) {
        if (m_Res != nullptr) PQclear(m_Res);
        m_Res = PQexec(m_Conn, query);
        if (!strncmp(query, "SELECT", 6)) {
            if (PQresultStatus(m_Res) != PGRES_TUPLES_OK) [[unlikely]] {
                std::cerr << "Failed to execute query \"" << query << "\": " << PQerrorMessage(m_Conn) << std::endl;
            }
            m_AttributeCount = 0;
            m_TupleCount = 0;
        } else {
            m_AttributeCount = PQnfields(m_Res);
            m_TupleCount = PQntuples(m_Res);
        }
    }

    [[nodiscard]] uint32_t attributeCount() const { return m_AttributeCount; }

    [[nodiscard]] uint32_t tupleCount() const { return m_TupleCount; }

    [[nodiscard]] bool isNull(const uint32_t tupleIndex, const uint32_t attributeIndex) const {
        return PQgetisnull(m_Res, static_cast<int>(tupleIndex), static_cast<int>(attributeIndex));
    }

    [[nodiscard]] const char *value(const uint32_t tupleIndex, const uint32_t attributeIndex) const {
        if (m_Res == nullptr) [[unlikely]] return nullptr;
        return PQgetvalue(m_Res, static_cast<int>(tupleIndex), static_cast<int>(attributeIndex));
    }

protected:
    void clearRes() {
        if (m_Res != nullptr) PQclear(m_Res);
        m_Res = nullptr;
        m_AttributeCount = 0;
        m_TupleCount = 0;
    }

private:
    bool m_ConnClosed;
    PGconn *m_Conn;
    PGresult *m_Res = nullptr;
    uint32_t m_ActiveTransactionCount = 0;
    uint32_t m_AttributeCount = 0, m_TupleCount = 0;
};


#endif //DATABASE_H
