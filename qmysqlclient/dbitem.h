#ifndef DB_ITEM_H
#define DB_ITEM_H
#include <QtGui/QStandardItem>
#include <QtSql/QSqlDatabase>
#include <QtWidgets/QMessageBox>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include "databaseinfo.h"

class DBItem :public QStandardItem
{
public:
	DBItem(DatabaseInfo *info)
		:info(info), QStandardItem(info->name) {}
	virtual ~DBItem() 
	{
		delete info;
		closeDBConnection();
	}
	bool checkDBConnection()
	{
		if (db.isOpen())
		{
			return true;
		}
		db = QSqlDatabase::addDatabase("QMYSQL", this->text());
		db.setHostName(info->ip);
		db.setDatabaseName("");
		db.setPort(info->port);
		db.setUserName(info->username);
		db.setPassword(info->password);
		if (!db.open())
		{
			QString s = QObject::tr("Connect failed ");
			s += QString::number(db.lastError().number());
			QMessageBox::warning(NULL, s, db.lastError().text(), QMessageBox::Ok);
			return false;
		}

		return true;
	}
	QSqlQuery executeSql(QString sql)
	{
		QSqlQuery query = db.exec(sql);
		if (query.lastError().number() != -1)
		{
			QString s = QObject::tr("Execute sql failed ");
			s += QString::number(db.lastError().number());
			QMessageBox::warning(NULL, s, "\"" + sql + "\": " + db.lastError().text(), QMessageBox::Ok);
		}
		return query;
	}
	QSqlDatabase &getDBConnection()
	{
		return db;
	}
	DatabaseInfo *getDBInfo()
	{
		return info;
	}
	void closeDBConnection()
	{
		if (db.isOpen())
		{
			db.close();
			db = QSqlDatabase();
			QSqlDatabase::removeDatabase(this->text());
		}
	}
private:
	DatabaseInfo *info;
	QSqlDatabase db;
};

#endif