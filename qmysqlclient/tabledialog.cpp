#include "tabledialog.h"
#include <QtSql/QSqlResult>
#include <QtSql/QSqlRecord>
#include <QtCore/QDebug>
#include <QtSql/QSqlField>
#include <QtCore/QDateTime>

TableDialog::TableDialog(TableItem *item, QWidget *parent)
	:startNumber(0), interval(1000), tableItem(item)
{
	ui.setupUi(this);
	if (!checkDBConnection())
	{
		return;
	}
	putDataOnTable();
	this->setWindowTitle(tableItem->getTableName());
	this->setWindowFlags(Qt::WindowMinimizeButtonHint|Qt::WindowMaximizeButtonHint|Qt::WindowCloseButtonHint);
	connect(ui.pushButtonNext, SIGNAL(clicked()), this, SLOT(nextButtonPushed()));
	connect(ui.PushButtonPrev, SIGNAL(clicked()), this, SLOT(prevButtonPushed()));
	connect(ui.pushButtonFirst, SIGNAL(clicked()), this, SLOT(firstButtonPushed()));
	connect(ui.pushButtonLast, SIGNAL(clicked()), this, SLOT(lastButtonPushed()));
	connect(ui.lineEdit, SIGNAL(editingFinished()), this, SLOT(pageNumberChanged()));
	ui.pushButtonInvalid->hide();
}

bool TableDialog::checkDBConnection()
{
	if (!tableItem->checkDBConnection())
	{
		return false;
	}
	QSqlQuery query = tableItem->executeSql(constructUseSql());
	if (query.lastError().number() != -1)
	{
		return false;
	}
	return true;
}

void TableDialog::putDataOnTable()
{
	QSqlQuery query = tableItem->executeSql(constructSelectSql());
	if (query.lastError().number() != -1)
	{
		return;
	}
	QStandardItemModel *oldModel = dynamic_cast<QStandardItemModel *>(ui.tableView->model());
	QStandardItemModel *model = new QStandardItemModel(this);
	putResultToModel(model, query);
	ui.tableView->setModel(model);
	ui.tableView->resizeColumnsToContents();
	if (oldModel != NULL)
	{
		oldModel->clear();
		delete oldModel;
	}
	ui.lineEdit->setText(QString::number(calculatePageNum()));
}

void TableDialog::viewTable()
{
	if (checkDBConnection())
	{
		putDataOnTable();
	}
}

QString TableDialog::constructSelectSql()
{
	return "select * from `" + tableItem->getTableName() + "` limit " + QString::number(startNumber) + ", " + QString::number(interval);
}

QString TableDialog::constructUseSql()
{
	return "use `" + tableItem->getDatabaseName() + "`";
}

int TableDialog::calculatePageNum()
{
	return 1 + startNumber / interval;
}

void TableDialog::putResultToModel(QStandardItemModel *model, QSqlQuery &query)
{
	QSqlRecord record = query.record();
	model->setColumnCount(record.count());
	model->setRowCount(query.size());
	for (int i = 0; i < record.count(); i++)
	{
		model->setHeaderData(i, Qt::Horizontal, record.fieldName(i));
	}
	int j = 0;
	while(query.next())
	{
		record = query.record();
		for (int i = 0; i < record.count(); i++)
		{
			QStandardItem *item;
			if (record.field(i).type() == QVariant::DateTime)
			{
				QDateTime time = record.value(i).toDateTime();
				item = new QStandardItem(time.toString("yyyy-mm-dd hh:MM:ss"));
			}
			else
			{
				qDebug() << record.field(i).type();
				item = new QStandardItem(record.value(i).toString());
			}
			model->setItem(j, i, item);
		}
		j++;
	}
}

void TableDialog::nextButtonPushed()
{
	startNumber += interval;
	viewTable();
}

void TableDialog::prevButtonPushed()
{
	int tmp = startNumber - interval;
	if (tmp >= 0)
	{
		startNumber = tmp;
		viewTable();
	}
}

void TableDialog::firstButtonPushed()
{
	if (startNumber != 0)
	{
		startNumber = 0;
		viewTable();
	}
}

void TableDialog::lastButtonPushed()
{
	if(!checkDBConnection())
	{
		return;
	}

	QSqlQuery query = tableItem->executeSql("select count(*) from `" + tableItem->getTableName() + "`");
	if (query.lastError().number() != -1)
	{
		return;
	}

	query.next();
	QSqlRecord record = query.record();
	int count = record.value(0).toInt();
	int tmp = count / interval * interval;
	if (startNumber != tmp)
	{
		startNumber = tmp;
		putDataOnTable();
	}
}

void TableDialog::pageNumberChanged()
{
	int tmp = ui.lineEdit->text().toInt();
	tmp = (tmp - 1) * interval;
	if (tmp < 0)
	{
		ui.lineEdit->setText(QString::number(calculatePageNum()));
		return;
	}
		
	if (startNumber != tmp)
	{
		startNumber = tmp;
		viewTable();
	}
}

TableDialog::~TableDialog()
{
	QStandardItemModel *model = dynamic_cast<QStandardItemModel *>(ui.tableView->model());
	if (model != NULL)
	{
		model->clear();
		delete model;
	}
}