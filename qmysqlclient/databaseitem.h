#ifndef _DATABASE_ITEM_H
#define _DATABASE_ITEM_H

#include <QtGui/QStandardItem>
#include <QtCore/QList>
#include <QtSql/QSqlDatabase>
#include "dbitem.h"

class DatabaseItem : public QStandardItem
{
public:
	DatabaseItem(QString s, DBItem *item)
		: QStandardItem(s), dbItem(item) {}
	virtual ~DatabaseItem() {}
	QSqlDatabase &getDBConnection()
	{
		return dbItem->getDBConnection();
	}
	bool checkDBConnection()
	{
		return dbItem->checkDBConnection();
	}
	QSqlQuery executeSql(QString sql)
	{
		return dbItem->executeSql(sql);
	}
	QString getDatabaseName()
	{
		return this->text();
	}
	QList<QString> *getTableList()
	{
		return &list;
	}
	DBItem *getDBItem()
	{
		return dbItem;
	}
	void closeDBConnection()
	{
		dbItem->closeDBConnection();
	}
private:
	DBItem *dbItem;
	QList<QString> list;
};

#endif