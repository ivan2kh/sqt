#include "dbconnection.h"
#include <QVector>
#include <QVariant>

DbConnection::DbConnection() :
    QObject(nullptr)
{
    _query_state = QueryState::Inactive;
}

DbConnection::~DbConnection()
{
    clearResultsets();
}

void DbConnection::setDatabase(const QString &database)
{
    if (_database != database)
    {
        close();
        _database = database;
    }
}

void DbConnection::setConnectionString(const QString &connectionString)
{
    close();
    _connection_string = connectionString;
}

QString DbConnection::connectionString() const
{
    return _connection_string;
}

QueryState DbConnection::queryState() const
{
    return _query_state;
}

DataTable* DbConnection::execute(const QString &query, const QVariantList &params)
{
    QVector<QVariant> p = params.toVector();
    // * synchronous usage only - no need to use _resultsetsGuard
    if (execute(query, &p) && !_resultsets.empty())
    {
        // return only last resultset to keep scripting api simple
        // (script takes ownership of the pointer)
        return _resultsets.takeLast();
    }
    return nullptr;
}

void DbConnection::setQueryState(QueryState state)
{
    if (_query_state != state)
    {
        _query_state = state;
        if (state == QueryState::Inactive)
            _elapsed_ms = _timer.elapsed();
        emit queryStateChanged();
    }
}

QString DbConnection::elapsed()
{
    int elapsed_ms = _elapsed_ms;
    int precision = 3;
    if (_query_state != QueryState::Inactive)
    {
        // round milliseconds during execution because of refresh period 0.2 sec
        elapsed_ms = _timer.elapsed() / 100 * 100;
        // rough precision to display simple float
        precision = 1;
    }

    if (elapsed_ms < 60000)
        return QString::number(elapsed_ms / 1000.0, 'f', precision) + " sec";

    return QDateTime::fromMSecsSinceEpoch(elapsed_ms).
            toString(elapsed_ms < 60 * 60000 ?
                         "mm:ss.zzz":
                         "HH:mm:ss");
}

void DbConnection::clearResultsets()
{
    qDeleteAll(_resultsets);
    _resultsets.clear();
}
