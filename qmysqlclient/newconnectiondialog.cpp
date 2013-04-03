#include "newconnectiondialog.h"
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlDatabase>
#include <QtWidgets/QMessageBox>
#include <QtSql/QSqlError>

NewConnectionDialog::NewConnectionDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	connect(ui.pushButtonTest, SIGNAL(clicked()), this, SLOT(testConnection()));
	connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui.okButton, SIGNAL(clicked()), this, SLOT(okButtonPressed()));
}

void NewConnectionDialog::okButtonPressed()
{
	if (ui.lineEditName->text() == "")
	{
		QMessageBox::warning(this, QObject::tr("Create error"), QObject::tr("Please give connection a name"), QMessageBox::Ok);
		return;
	}

	DatabaseInfo *info = new DatabaseInfo(ui.lineEditName->text(), ui.lineEditIP->text(), ui.lineEditPort->text().toInt(), ui.lineEditUsername->text(), ui.lineEditPasswd->text());
	emit createNewConnection(info);
	this->close();
}

NewConnectionDialog::~NewConnectionDialog()
{
	
}

void NewConnectionDialog::testConnection()
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
	db.setHostName(ui.lineEditIP->text());
	db.setDatabaseName("");
	db.setUserName(ui.lineEditUsername->text());
	db.setPassword(ui.lineEditPasswd->text());
	db.setPort(ui.lineEditPort->text().toInt());
	bool ok = db.open();
	if (ok)
	{
		QMessageBox::information(this, QObject::tr("Connect OK"), QObject::tr("Connect successfully"), QMessageBox::Ok);
	}
	else
	{
		QString s = QObject::tr("Connect failed ");
		s += QString::number(db.lastError().number());
		QMessageBox::warning(this, s, db.lastError().text(), QMessageBox::Ok);
	}
	db.close();
}

void NewConnectionDialog::createNewResult(bool result)
{
	if (result)
	{
		this->close();
	}
	else
	{
		QMessageBox::warning(this, QObject::tr("Please check the name"), QObject::tr("There is already the same name"), QMessageBox::Ok);
	}
}