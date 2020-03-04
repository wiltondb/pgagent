//////////////////////////////////////////////////////////////////////////
//
// pgAgent - PostgreSQL Tools
//
// Copyright (C) 2002 - 2020, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// connection.h - database connection
//
//////////////////////////////////////////////////////////////////////////


#ifndef CONNECTION_H
#define CONNECTION_H

#include <libpq-fe.h>

class DBresult;

class CONNinfo
{
public:
    bool Set(const std::wstring& connStr);
	const std::wstring Get(const std::wstring &dbName=L"") const;
    const std::wstring& GetError() const { return m_error; }

	static const std::wstring Parse(
			const std::wstring& connStr, std::wstring *error,
			std::wstring *dbName, bool forLogging=false
		);

	operator bool() const { return m_connStr.empty(); }

private:
    std::wstring  m_connStr;
    std::wstring  m_dbName;
    std::wstring  m_error;
};

class DBconn
{
protected:
	DBconn(const std::wstring& connStr);
	~DBconn();

public:
	std::wstring qtDbString(const std::wstring &value);

	bool BackendMinimumVersion(int major, int minor);

	static DBconn *Get(const std::wstring &connStr=L"", const std::wstring &db=L"");
	static DBconn *InitConnection(const std::wstring &connectString);

	static void ClearConnections(bool allIncludingPrimary = false);

	std::wstring GetLastError();

    operator bool() const { return m_conn != NULL; }

	bool LastCommandOk()
	{
		return IsCommandOk((ExecStatusType)m_lastResult);
	}

	bool IsCommandOk(ExecStatusType ret)
	{
		switch (ret)
		{
			case PGRES_COMMAND_OK:
			case PGRES_TUPLES_OK:
			case PGRES_COPY_OUT:
			case PGRES_COPY_IN:
#if (PG_VERSION_NUM >= 90100)
			case PGRES_COPY_BOTH:
#endif
				return true;
			default:
				return false;
		};
	}

	void SetLastResult(int res)
	{
		m_lastResult = res;
	}

	int GetLastResult()
	{
		return m_lastResult;
	}

	DBresult *Execute(const std::wstring &query);
	std::wstring ExecuteScalar(const std::wstring &query);
	int ExecuteVoid(const std::wstring &query);
	void Return();

    const std::wstring &DebugConnectionStr() const;

private:
	bool Connect(const std::wstring &connectString);

	int              m_minorVersion,
                     m_majorVersion;

protected:
	static CONNinfo  ms_basicConnInfo;
	static DBconn   *ms_primaryConn;

	std::wstring     m_lastError;
	std::wstring     m_connStr;

	PGconn          *m_conn;
	DBconn          *m_next;
	DBconn          *m_prev;

	bool             m_remoteDatabase;
	bool             m_inUse;
	int              m_lastResult;

	friend class DBresult;

};


class DBresult
{
protected:
	DBresult(DBconn *conn, const std::wstring &query);

public:
	~DBresult();

	std::wstring GetString(int col) const;
	std::wstring GetString(const std::wstring &colname) const;

	bool IsValid() const
	{
		return m_result != NULL;
	}
	bool HasData() const
	{
		return m_currentRow < m_maxRows;
	}
	void MoveNext()
	{
		if (m_currentRow < m_maxRows) m_currentRow++;
	}

	long RowsAffected() const
	{
		return atol(PQcmdTuples(m_result));
	}

protected:
	PGresult *m_result;
	int m_currentRow, m_maxRows;

	friend class DBconn;
};


class DBresultPtr
{
public:
	DBresultPtr(DBresult* in_ptr)
		: m_ptr(in_ptr)
	{}
	~DBresultPtr()
	{
		if (m_ptr) {
			delete m_ptr;
			m_ptr = NULL;
		}
	}
	DBresultPtr& operator=(DBresult *other)
	{
		if (m_ptr) {
			delete m_ptr;
		}
		m_ptr = other;
		return *this;
	}
	const DBresult& operator*() const
	{
		return (*(const DBresult *)m_ptr);
	}
	const DBresult* operator->() const
	{
		return (const DBresult*)(m_ptr);
	}
	DBresult& operator*()
	{
		return (*(DBresult *)m_ptr);
	}
	DBresult* operator->()
	{
		return (DBresult *)m_ptr;
	}
	operator void*() const
	{
		return (DBresult *)m_ptr;
	}
	operator bool() const { return (m_ptr != NULL); }

protected:
	DBresult* m_ptr;
};

#endif // CONNECTION_H

