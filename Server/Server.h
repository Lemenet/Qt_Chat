#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Server.h"

class Server : public QMainWindow
{
	Q_OBJECT

public:
	Server(QWidget *parent = Q_NULLPTR);
	~Server();

	
private:
	Ui::ServerClass ui;


private slots:
	void SlotStartServer();
	void SlotStopServer();
};
