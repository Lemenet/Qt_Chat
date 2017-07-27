#ifdef WIN32
#define _WIN32_WINNT 0x0501
#endif // WIN32


#include "Client.h"
#include "ChatClient.hpp"
#include "QMessageBox"
#include "QCloseEvent"
#include <QDebug>



Client::Client(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	ui.lineEdit->setText("192.168.0.125");
	ui.lineEdit_2->setText("8888");
	
	connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(SlotStartConnect()));
	connect(ui.pushButton_2, SIGNAL(clicked()), this, SLOT(SlotSendMessage()));
}

void Client::closeEvent(QCloseEvent *event)
{
	
	qDebug() << QString::fromLocal8Bit("Debug:准备退出");
	QMessageBox::StandardButton button;
	button = QMessageBox::question(this, QString::fromLocal8Bit("退出程序"),
		QString::fromLocal8Bit("警告！是否需要退出程序"),
		QMessageBox::Yes | QMessageBox::No);

	if (button == QMessageBox::No)
	{
		event->ignore();
	}
	else if (button == QMessageBox::Yes)
	{
		if (client.get() != nullptr)
		{
			client->Close();
			ioservice_.stop();
		}

		event->accept();
	}


}

void Client::SlotSendMessage()
{
	/*QString str = ui.plainTextEdit->toPlainText();
	msg_.SetBodyLength(str.size());
	memcpy(msg_.body(), str.data(), str.size());
	qDebug() << QString::fromLocal8Bit("Debug:") << str;
	msg_.EncodeHeader();
	client->Send(msg_);*/

	QByteArray buf = ui.plainTextEdit->toPlainText().toLocal8Bit();

	msg_.SetBodyLength(buf.size());
	memcpy(msg_.body(), buf.data(), buf.size());
	msg_.SetBodyLength(buf.size());
	msg_.EncodeHeader();
	client->Send(msg_);
}

void Client::HandlePackage()
{
	if (client)
	{

		//QMessageBox::critical(NULL, QString::fromLocal8Bit("标题"), ui.lineEdit->text(), QMessageBox::Ok);
		ChatMessage msg = client->GetMessage();

		char szText[ChatMessage::MAX_BODY_LENGTH + 1] = { 0 };

		memcpy(szText, msg.body(), msg.length());

		ui.textEdit->append(QString::fromLocal8Bit(szText));
	}
}


void Client::SlotStartConnect()
{


	
	tcp::resolver resolver(ioservice_);
	tcp::resolver::query query(ui.lineEdit->text().toStdString().c_str(),
								ui.lineEdit_2->text().toStdString().c_str());
	tcp::resolver::iterator iterator = resolver.resolve(query);

	client.reset(new ChatClient(ioservice_, iterator));

	work.reset(new io_service::work(ioservice_));

	QObject::connect(client.get(), SIGNAL(ReceivePackage()), this, SLOT(HandlePackage()));

	boost::shared_ptr<boost::thread> t(
		boost::make_shared<boost::thread>(
			boost::bind(&boost::asio::io_service::run, &ioservice_))
	);

	t->detach();

	//Do something
	//char line[ChatMessage::MAX_BODY_LENGTH + 1];



	//while (std::cin.getline(line, sizeof(line)))
	//{
	//	//memcpy(line, ui.textEdit_2->acceptRichText().toStdString(), ui.textEdit_2->
	//	if (strcmp("exit", line) == 0)
	//	{
	//		break;
	//	}
	//	ChatMessage msg;
	//	msg.SetBodyLength(strlen(line));
	//	memcpy(msg.body(), line, msg.bodyLength());

	//	msg.EncodeHeader();//包头加密

	//	client->Send(msg);
	//}


	//client->Close();
}