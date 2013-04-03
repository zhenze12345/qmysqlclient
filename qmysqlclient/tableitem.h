#ifndef TABLE_ITEM_H
#define TABLE_ITEM_H

#include <databaseitem.h>
#include <databaseinfo.h>
#include <QtGui/QStandardItem>
#include <QtSql/QSqlDatabase>

class TableItem :public QStandardItem
{
public:
	TableItem(QString s, DatabaseItem *item)
		:QStandardItem(s), databaseItem(item) {}
	virtual ~TableItem() {}
	QSqlDatabase &getDBConnection()
	{
		return databaseItem->getDBConnection();
	}

	DatabaseInfo *getDBInfo()
	{
		return databaseItem->getDBItem()->getDBInfo();
	}
	bool checkDBConnection()
	{
		return databaseItem->checkDBConnection();
	}
	QSqlQuery executeSql(QString sql)
	{
		return databaseItem->executeSql(sql);
	}
	QString getDatabaseName()
	{
		return databaseItem->getDatabaseName();
	}
	QString getTableName()
	{
		return this->text();
	}
	DatabaseItem *getDatabaseItem()
	{
		return databaseItem;
	}
	void closeDBConnection()
	{
		databaseItem->closeDBConnection();
	}
	DBItem *getDBItem()
	{
		return databaseItem->getDBItem();
	}

private:
	DatabaseItem *databaseItem;
};

#endif