#ifndef DATABASE_H
#define DATABASE_H
#include <QString>
#include <QSqlDatabase>
#include <QObject>

class Data
{
public:
    QString id;
    QString name;
    QString enname;
    QString exepath;
    QString describe;
    QString comment;
    QString lastplaytime;
};

Q_DECLARE_METATYPE(Data)

class Database : public QObject
{
    Q_OBJECT
public:
    static Database* instance();
    bool update(const Data& data);
    bool update(QString id, QString colname, QString value);
    bool remove(QString id);
    bool exist(QString name);
    int rowCount();
    void loadAllData();

signals:
    void dataLoaded(Data d);

private:
    Database();
    void createTable();
    QSqlDatabase db;
};

#endif // DATABASE_H
