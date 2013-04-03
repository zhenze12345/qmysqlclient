#include "qmysqlclient.h"
#include "newconnectiondialog.h"
#include <QtCore/QDebug>
#include <QtGui/QStandardItem>
#include <QtGui/QStandardItemModel>
#include "dbitem.h"
#include "databaseitem.h"
#include "tableitem.h"
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QScrollBar>
#include <QtSql/QSqlTableModel>
#include "tabledialog.h"
#include <QtCore/QSettings>
#include <QtCore/QFile>

#define CONFIG_FILE "qmysqlclient.ini"

QMysqlClient::QMysqlClient(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	connect(ui.actionNew, SIGNAL(triggered()), this, SLOT(newConnection()));
	connect(ui.actionQT, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
	connect(ui.actionQuit, SIGNAL(triggered()), this, SLOT(appQuit()));
	ui.treeView->header()->hide();
	connect(ui.treeView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(databaseSelected(const QModelIndex &)));
	connect(ui.tableView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(tableSelected(const QModelIndex &)));
	QStandardItemModel *model = new QStandardItemModel(this);
	readConfigure(model);
	ui.treeView->setModel(model);
	ui.treeView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.treeView, SIGNAL(customContextMenuRequested(const QPoint)), this, SLOT(showTreeViewRightMenu(const QPoint)));
	model = new QStandardItemModel(this);
	ui.tableView->setModel(model);
	firstLabel = new QLabel();
	ui.statusBar->addWidget(firstLabel);
	secondLabel = new QLabel();
	secondLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	ui.statusBar->addWidget(secondLabel);
}

void QMysqlClient::showTreeViewRightMenu(const QPoint point)
{
	QModelIndex index = ui.treeView->indexAt(point);
	QStandardItem *parentItem = static_cast<QStandardItem *>(index.internalPointer());
	// 如果点在空白处，则忽略
	if (parentItem == NULL)
	{
		return;
	}

	QStandardItemModel *model = dynamic_cast<QStandardItemModel *>(ui.treeView->model());
	QStandardItem *rootItem = model->invisibleRootItem();
	QStandardItem *selectedItem = parentItem->child(index.row(), index.column());
	QMenu *menu = new QMenu(ui.treeView);
	// 如果是一级目录
	if (parentItem == rootItem)
	{
		QAction *action = new QAction(QObject::tr("Refresh"), this);
		connect(action, SIGNAL(triggered()), this, SLOT(treeFirstMenuRefresh()));
		menu->addAction(action);
		action = new QAction(QObject::tr("Close"), this);
		connect(action, SIGNAL(triggered()), this, SLOT(treeFirstMenuClose()));
		menu->addAction(action);
		action = new QAction(QObject::tr("Delete"), this);
		connect(action, SIGNAL(triggered()), this, SLOT(treeFirstMenuDelete()));
		menu->addAction(action);
	}
	// 如果是二级目录
	else
	{
		QAction *action = new QAction(QObject::tr("Refresh"), this);
		connect(action, SIGNAL(triggered()), this, SLOT(treeSecondMenuRefresh()));
		menu->addAction(action);
		action = new QAction(QObject::tr("Delete"), this);
		connect(action, SIGNAL(triggered()), this, SLOT(treeSecondMenuDelete()));
		menu->addAction(action);
	}
	menu->exec(QCursor::pos());
	menu->clear();
	delete menu;
}

void QMysqlClient::treeSecondMenuDelete()
{
	QModelIndex index = ui.treeView->currentIndex();
	QStandardItem *parent = static_cast<QStandardItem *>(index.internalPointer());
	DBItem *dbItem = dynamic_cast<DBItem *>(parent);
	if (dbItem == NULL)
	{
		return;
	}

	DatabaseItem *databaseItem = dynamic_cast<DatabaseItem *>(dbItem->child(index.row(), index.column()));
	if (databaseItem == NULL)
	{
		return;
	}

	QStandardItemModel *tableModel = dynamic_cast<QStandardItemModel *>(ui.tableView->model());
	if (tableModel == NULL)
	{
		return;
	}
	
	QStandardItem *tableRootItem = tableModel->invisibleRootItem();
	if (tableRootItem == NULL)
	{
		return;
	}

	TableItem *tableItem = dynamic_cast<TableItem *>(tableRootItem->child(0));
	if (tableItem == NULL)
	{
		return;
	}

	QMessageBox::StandardButton button = QMessageBox::warning(this, QObject::tr("Warning"), 
		QObject::tr("Are you sure you want to delete '%1' database?").arg(databaseItem->getDatabaseName()),
		QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
	if (button == QMessageBox::No)
	{
		return;
	}

	if (!dbItem->checkDBConnection())
	{
		return;
	}

	QSqlQuery query = dbItem->executeSql("drop database `" + databaseItem->getDatabaseName() + "`");
	if (query.lastError().number() != -1)
	{
		return;
	}

	dbItem->removeRow(index.row());
	if (tableItem->getDatabaseItem() == databaseItem)
	{
		tableModel->clear();
	}
}

void QMysqlClient::treeSecondMenuRefresh()
{
	QModelIndex index = ui.treeView->currentIndex();
	QStandardItem *parent = static_cast<QStandardItem *>(index.internalPointer());
	DBItem *dbItem = dynamic_cast<DBItem *>(parent);
	if (dbItem == NULL)
	{
		return;
	}

	DatabaseItem *databaseItem = dynamic_cast<DatabaseItem *>(dbItem->child(index.row(), index.column()));
	if (databaseItem == NULL)
	{
		return;
	}

	refreshTables(databaseItem);
}

void QMysqlClient::treeFirstMenuRefresh()
{
	QModelIndex index = ui.treeView->currentIndex();
	QStandardItem *parent = static_cast<QStandardItem *>(index.internalPointer());
	if (parent == NULL)
	{
		return;
	}

	DBItem *dbItem = dynamic_cast<DBItem *>(parent->child(index.row(), index.column()));
	if (dbItem == NULL)
	{
		return;
	}

	refreshDatabases(dbItem, index);
	ui.treeView->expand(index);
}

void QMysqlClient::treeFirstMenuClose()
{
	QModelIndex index = ui.treeView->currentIndex();
	QStandardItem *parentItem = static_cast<QStandardItem *>(index.internalPointer());
	if (parentItem == NULL)
	{
		return;
	}

	DBItem *selectedItem = dynamic_cast<DBItem *>(parentItem->child(index.row(), index.column()));
	if (selectedItem == NULL)
	{
		return;
	}

	QStandardItemModel *tableModel = dynamic_cast<QStandardItemModel *>(ui.tableView->model());
	if (tableModel == NULL)
	{
		return;
	}

	QStandardItem *tableRootItem = tableModel->invisibleRootItem();
	if (tableRootItem == NULL)
	{
		return;
	}

	if (!tableRootItem->hasChildren())
	{
		selectedItem->closeDBConnection();
		selectedItem->removeRows(0, selectedItem->rowCount());
		return;
	}

	TableItem *tableSampleItem = dynamic_cast<TableItem *>(tableRootItem->child(0));
	if (tableSampleItem == NULL)
	{
		return;
	}

	if (tableSampleItem->getDBItem() == selectedItem)
	{
		tableModel->clear();
	}
		
	selectedItem->closeDBConnection();
	selectedItem->removeRows(0, selectedItem->rowCount());
}

void QMysqlClient::treeFirstMenuDelete()
{
	QModelIndex index = ui.treeView->currentIndex();
	int row = index.row();
	QStandardItem *parentItem = static_cast<QStandardItem *>(index.internalPointer());
	if (parentItem == NULL)
	{
		return;
	}
	QStandardItemModel *model = dynamic_cast<QStandardItemModel *>(ui.treeView->model());	
	if (model == NULL)
	{
		return;
	}

	QStandardItem *rootItem = model->invisibleRootItem();
	if (rootItem == NULL || parentItem != rootItem)
	{
		return;
	}

	rootItem->removeRow(row);
}

void QMysqlClient::readConfigure(QStandardItemModel *model)
{
	if(!QFile::exists(CONFIG_FILE))
	{
		return;
	}
	QSettings *settings = new QSettings(CONFIG_FILE, QSettings::IniFormat);
	if (settings == NULL)
	{
		return;
	}
	foreach(QString group, settings->childGroups())
	{
		settings->beginGroup(group);
		DatabaseInfo *info 
			= new DatabaseInfo(group,
								settings->value("ip").toString(),
								settings->value("port").toInt(),
								settings->value("username").toString(),
								settings->value("password").toString());
		settings->endGroup();
		DBItem *item = new DBItem(info);
		model->appendRow(item);
	}
	delete settings;
}

void QMysqlClient::saveConfigure()
{
	QStandardItemModel *model = dynamic_cast<QStandardItemModel *>(ui.treeView->model());
	QStandardItem *rootItem = model->invisibleRootItem();
	QSettings *settings = new QSettings(CONFIG_FILE, QSettings::IniFormat);
	settings->clear();
	for (int i = 0; i < rootItem->rowCount(); i++)
	{
		DBItem *item = dynamic_cast<DBItem *>(rootItem->child(i));
		if (item == NULL)
		{
			continue;
		}
		settings->beginGroup(item->text());
		settings->setValue("ip", item->getDBInfo()->ip);
		settings->setValue("port", item->getDBInfo()->port);
		settings->setValue("username", item->getDBInfo()->username);
		settings->setValue("password", item->getDBInfo()->password);
		settings->endGroup();
	}
	delete settings;
}

void QMysqlClient::tableSelected(const QModelIndex &index)
{
	QStandardItem *rootItem = static_cast<QStandardItem *>(index.internalPointer());
	TableItem *tableItem = dynamic_cast<TableItem *>(rootItem->child(index.row(), index.column()));
	if (tableItem == NULL || rootItem == NULL)
	{
		return;
	}

	TableDialog dialog = TableDialog(tableItem, this);
	if (dialog.isReady())
	{
		dialog.show();
		dialog.exec();
	}
}

void QMysqlClient::refreshDatabases(DBItem *item, const QModelIndex &index)
{
	if (!item->checkDBConnection())
	{
		return;
	}
	item->removeRows(0, item->rowCount());
	QSqlQuery query = item->executeSql("show databases");
	if (query.lastError().number() != -1)
	{
		return;
	}
	while(query.next())
	{
		DatabaseItem *childItem = new DatabaseItem(query.value(0).toString(), item);
		item->appendRow(childItem);
	}
	ui.treeView->collapse(index);
}

void QMysqlClient::refreshTables(DatabaseItem *item)
{
	QStandardItemModel *tableModel = dynamic_cast<QStandardItemModel *>(ui.tableView->model());
	if (tableModel == NULL)
	{
		return;
	}
	if(!item->checkDBConnection())
	{
		return;
	}
	QSqlQuery query = item->executeSql("use `" + item->getDatabaseName() + "`");
	if (query.lastError().number() != -1)
	{
		return;
	}
	query = item->executeSql("show tables");
	if (query.lastError().number() != -1)
	{
		return;
	}
	int rowCount = calTableRows();
	int colCount = query.size() / rowCount + (query.size() % rowCount == 0 ? 0 : 1);
	int row = 0;
	int col = 0;
	QStandardItemModel *newModel = new QStandardItemModel();
	newModel->setColumnCount(colCount);
	newModel->setRowCount(rowCount);
	item->getTableList()->clear();
	while(query.next())
	{
		item->getTableList()->push_back(query.value(0).toString());
		TableItem *tableItem = new TableItem(query.value(0).toString(), item);
		newModel->setItem(row++, col, tableItem);
		if (row > rowCount - 1)
		{
			row = 0;
			col++;
		}
	}
	ui.tableView->setModel(newModel);
	ui.tableView->resizeColumnsToContents();
	tableModel->clear();
	delete tableModel;
}

void QMysqlClient::databaseSelected(const QModelIndex &index)
{
	QStandardItemModel *model = dynamic_cast<QStandardItemModel *>(ui.treeView->model());
	QStandardItem *rootItem = model->invisibleRootItem();
	QStandardItem *clickedRootItem = static_cast<QStandardItem *>(index.internalPointer());
	// 如果是需要显示连接的数据库
	if (rootItem == clickedRootItem)
	{
		DBItem *item = dynamic_cast<DBItem *>(clickedRootItem->child(index.row(), index.column()));
		if (item != NULL)
		{
			if (item->rowCount() != 0)
			{
				return;
			}
			refreshDatabases(item, index);
		}
	}
	// 如果是需要显示表名
	else
	{
		DBItem *item = dynamic_cast<DBItem *>(clickedRootItem);
		DatabaseItem *childItem = dynamic_cast<DatabaseItem *>(clickedRootItem->child(index.row(), index.column()));
		QStandardItemModel *tableModel = dynamic_cast<QStandardItemModel *>(ui.tableView->model());
		if (childItem != NULL && item != NULL)
		{
			if (childItem->getTableList()->size() == 0)
			{
				refreshTables(childItem);
			}
			else
			{
				QStandardItemModel *newModel = new QStandardItemModel();
				int rowCount = calTableRows();
				int colCount = childItem->getTableList()->size() / rowCount + (childItem->getTableList()->size() % rowCount == 0 ? 0 : 1);
				int row = 0;
				int col = 0;

				for (QList<QString>::Iterator it = childItem->getTableList()->begin(); it != childItem->getTableList()->end(); it++)
				{
					TableItem *tableItem = new TableItem(*it, childItem);
					newModel->setItem(row++, col, tableItem);
					if (row >rowCount - 1)
					{
						row = 0;
						col++;
					}
				}
				ui.tableView->setModel(newModel);
				ui.tableView->resizeColumnsToContents();
				tableModel->clear();
				delete tableModel;
			}
		}
	}
}

int QMysqlClient::calTableRows()
{
	return ( ui.tableView->size().height() - ui.tableView->horizontalScrollBar()->height()) / ui.tableView->verticalHeader()->defaultSectionSize();
}

void QMysqlClient::appQuit()
{
	qApp->exit();
}

void QMysqlClient::newConnection()
{
	NewConnectionDialog dialog;
	connect(&dialog, SIGNAL(createNewConnection(DatabaseInfo *)), this, SLOT(createNewConnection(DatabaseInfo *)));
	connect(this, SIGNAL(createNewResult(bool)), &dialog, SLOT(createNewResult(bool)));
	dialog.show();
	dialog.exec();
}

void QMysqlClient::createNewConnection(DatabaseInfo *info)
{
	QStandardItemModel *model = dynamic_cast<QStandardItemModel *>(ui.treeView->model());
	QStandardItem *rootItem = model->invisibleRootItem();
	for (int i = 0; i < rootItem->rowCount(); i++)
	{
		if (rootItem->child(i) == NULL)
		{
			continue;
		}
		if (rootItem->child(i)->text() == info->name)
		{
			emit createNewResult(false);
			return;
		}
	}
	DBItem *item = new DBItem(info);
	model->appendRow(item);
	emit createNewResult(true);
}

void QMysqlClient::resizeEvent(QResizeEvent *event)
{
	firstLabel->setFixedWidth(ui.treeView->x() + ui.treeView->width());
	QStandardItemModel *model = dynamic_cast<QStandardItemModel *>(ui.tableView->model());
	TableItem *tableItem = dynamic_cast<TableItem *>(model->item(0,0));
	if (tableItem == NULL)
	{
		return;
	}

	int oldRowCount = ui.tableView->model()->rowCount();
	int rowCount = calTableRows();
	if (oldRowCount == rowCount)
	{
		return;
	}

	if (ui.tableView->model()->columnCount() == 1 && oldRowCount < rowCount)
	{
		return;
	}

	QStandardItemModel *newModel = new QStandardItemModel();
	DatabaseItem *databaseItem = tableItem->getDatabaseItem();
	int colCount = databaseItem->getTableList()->size() / rowCount + (databaseItem->getTableList()->size() % rowCount == 0 ? 0 : 1);
	int row = 0;
	int col = 0;
	foreach(QString s, *databaseItem->getTableList())
	{
		TableItem *newItem = new TableItem(s, databaseItem);
		newModel->setItem(row++, col, newItem);
		if (row > rowCount - 1)
		{
			row = 0;
			col++;
		}
	}
	ui.tableView->setModel(newModel);
	ui.tableView->resizeColumnsToContents();
	model->clear();	
	delete model;
}

void QMysqlClient::closeEvent(QCloseEvent *event)
{
	qApp->quit();
}

QMysqlClient::~QMysqlClient()
{
	saveConfigure();
	QStandardItemModel *model = dynamic_cast<QStandardItemModel *>(ui.tableView->model());
	if (model != NULL)
	{
		model->clear();
		delete model;
	}

	model = dynamic_cast<QStandardItemModel *>(ui.treeView->model());
	if (model != NULL)
	{
		model->clear();
		delete model;
	}

	if (firstLabel != NULL)
	{
		delete firstLabel;
	}

	if (secondLabel != NULL)
	{
		delete secondLabel;
	}
}