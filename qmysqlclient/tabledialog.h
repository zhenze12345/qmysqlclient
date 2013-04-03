#ifndef TABLE_DIALOG_H
#define TABLE_DIALOG_H

#include <QtWidgets/QMainWindow>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtWidgets/QDialog>
#include "ui_table.h"
#include "tableitem.h"

class TableDialog : public QDialog
{
	Q_OBJECT

public:
	TableDialog(TableItem *item, QWidget *parent = 0);
	~TableDialog();
	bool isReady()
	{
		return ui.tableView->model() != NULL;
	}

private:
	void putResultToModel(QStandardItemModel *model, QSqlQuery &query);
	QString constructSelectSql();
	QString constructUseSql();
	int calculatePageNum();
	void viewTable();
	bool checkDBConnection();
	void putDataOnTable();

private slots:
	void nextButtonPushed();
	void prevButtonPushed();
	void firstButtonPushed();
	void lastButtonPushed();
	void pageNumberChanged();

private:
	Ui::TableView ui;
	TableItem *tableItem;
	int startNumber;
	int interval;
};

#endif