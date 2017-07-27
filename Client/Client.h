#pragma once

#include <QtWidgets/QMainWindow>
#include <boost/asio.hpp>
#include "ChatClient.hpp"
#include "ui_Client.h"

//class boost::asio::io_service;


class Client : public QMainWindow
{
	Q_OBJECT

public:
	Client(QWidget *parent = Q_NULLPTR);

private:
	Ui::ClientClass ui;
	boost::asio::io_service ioservice_;
	boost::shared_ptr<ChatClient> client;
	boost::shared_ptr<io_service::work> work;
	ChatMessage msg_;

private slots:
	void SlotStartConnect();
	void closeEvent(QCloseEvent *event);
	void SlotSendMessage();
	void HandlePackage();
};
