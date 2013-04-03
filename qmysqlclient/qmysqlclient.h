#ifndef QMYSQLCLIENT_H
#define QMYSQLCLIENT_H

#include <QtWidgets/QMainWindow>
#include <QtCore/QList>
#include <QtGui/QStandardItem>
#include "ui_qmysqlclient.h"
#include "newconnectiondialog.h"
#include "dbitem.h"
#include "databaseitem.h"

class QMysqlClient : public QMainWindow
{
	Q_OBJECT

public:
	QMysqlClient(QWidget *parent = 0);
	~QMysqlClient();

private:
	void closeEvent(QCloseEvent *event);
	int calTableRows();
	void resizeEvent(QResizeEvent *event);
	void readConfigure(QStandardItemModel *model);
	void saveConfigure();
	void refreshDatabases(DBItem *item, const QModelIndex &index);
	void refreshTables(DatabaseItem *item);

signals:
	void createNewResult(bool result);

public slots:
	void createNewConnection(DatabaseInfo *info);
	void showTreeViewRightMenu(const QPoint point);

private slots:
	void newConnection();
	void appQuit();
	void databaseSelected(const QModelIndex &index);
	void tableSelected(const QModelIndex &index);
	void treeFirstMenuDelete();
	void treeFirstMenuClose();
	void treeFirstMenuRefresh();
	void treeSecondMenuRefresh();
	void treeSecondMenuDelete();

private:
	Ui::QMysqlClientClass ui;
	QLabel *firstLabel;
	QLabel *secondLabel;
};

#endif // QMYSQLCLIENT_H
