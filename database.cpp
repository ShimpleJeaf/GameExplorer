#include "database.h"
#include "pubheader.h"
#include <iostream>

Database *Database::instance()
{
    static Database* ins = new Database();
    return ins;
}

bool Database::update(const Data& data)
{
    QString presql;
    if (exist(data.name))
        presql = "UPDATE games SET name=:name, enname=:enname, exepath=:exepath, describe=:describe, comment=:comment, updatetime=:updatetime where id=:id";
    else
        presql = "INSERT INTO games VALUES (:id, :name, :enname, :exepath, :describe, :comment, :updatetime)";
    QSqlQuery query(db);
    query.prepare(presql);
    query.bindValue(":name", data.name);
    query.bindValue(":enname", data.enname);
    query.bindValue(":exepath", data.exepath);
    query.bindValue(":describe", data.describe);
    query.bindValue(":comment", data.comment);
    query.bindValue(":updatetime", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    query.bindValue(":id", data.id);
    if (!query.exec()) {
        qDebug() << query.lastError();
        qDebug() << "sql: " << query.executedQuery();
        return false;
    }
    return true;
}

bool Database::update(QString id, QString colname, QString value)
{
    QSqlQuery query(db);
    query.prepare(QString("UPDATE games SET %1='%2', updatetime='%3' where id='%4'").arg(colname).arg(value).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")).arg(id));
    if (!query.exec()) {
        qDebug() << db.lastError().text();
        qDebug() << "sql: " << query.executedQuery();
        return false;
    }
    return true;
}

bool Database::remove(QString id)
{
    QSqlQuery query(db);
    query.prepare(QString("DELETE FROM games WHERE id=?"));
    query.bindValue(0, id);
    if (!query.exec()) {
        qDebug() << query.lastError();
        return false;
    }
    return true;
}

bool Database::exist(QString name)
{
    QSqlQuery query(db);
    query.prepare("select 1 from games where name=? limit 1");
    query.bindValue(0, name);
    if (!query.exec())
        qDebug() << query.lastError();
    if (query.next())
        return true;
    return false;
}

int Database::rowCount()
{
    QSqlQuery query(db);
    query.prepare("select count(*) from games");
    if (!query.exec())
        qDebug() << query.lastError();
    if (query.next())
        return query.value(0).toInt(0);
    else
        return 0;
}

void Database::loadAllData()
{
    QSqlQuery query(db);
    query.prepare("select * from games");
    if (!query.exec())
        qDebug() << query.lastError();
    while (query.next()) {
        Data d;
        d.id = query.value("id").toString();
        d.name = query.value("name").toString();
        d.enname = query.value("enname").toString();
        d.exepath = query.value("exepath").toString();
        d.describe = query.value("describe").toString();
        d.comment = query.value("comment").toString();
        emit dataLoaded(d);
    }
}

Database::Database() :
    QObject()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("game.db");
    if (db.open()) {
        qDebug() << "database open succeed";
    }
    else {
        qDebug() << db.lastError().text();
        exit(-1);
    }

    createTable();
}

void Database::createTable()
{
    QString sql = \
        "CREATE TABLE IF NOT EXISTS games ( \
        id              STRING PRIMARY KEY NOT NULL, \
        name            STRING, \
        enname          STRING, \
        exepath         STRING, \
        describe        STRING, \
        comment         STRING, \
        updatetime      DATETIME \
        );";
    QSqlQuery query(sql, db);
    if (!query.exec()) {
        qDebug() << "table 'games' created failed";
        qDebug() << "sql: " << sql;
        qDebug() << query.lastError();
        exit(-1);
    }
}
