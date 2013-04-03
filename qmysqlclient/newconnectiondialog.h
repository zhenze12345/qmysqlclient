#ifndef NEW_CONNECTION_DIALOG_H
#define NEW_CONNECTION_DIALOG_H

#include <QtWidgets/QDialog>
#include "ui_newconnection.h"
#include "databaseinfo.h"

class NewConnectionDialog : public QDialog
{
	Q_OBJECT

public:
	NewConnectionDialog(QWidget *parent = 0);
	virtual ~NewConnectionDialog();

signals:
	void createNewConnection(DatabaseInfo *info);

public slots:
	void createNewResult(bool result);

private slots:
	void testConnection();
	void okButtonPressed();

private:
	Ui::newConnectionDialog ui;
};

#endif // !NEW_CONNECTION_DIALOG
