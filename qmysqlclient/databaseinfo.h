#ifndef DATABASE_INFO_H
#define DATABASE_INFO_H

#include <QtCore/QString>

class DatabaseInfo
{
public:
	DatabaseInfo(QString infoName, QString infoIP, int infoPort, QString infoUsername, QString infoPassword)
		:name(infoName), ip(infoIP), port(infoPort), username(infoUsername), password(infoPassword) {}
	~DatabaseInfo() {}
public:
	QString name;
	QString ip;
	int port;
	QString username;
	QString password;
};

#endif