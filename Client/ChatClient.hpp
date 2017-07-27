#pragma once
#ifdef WIN32
#define _WIN32_WINNT 0x0501
#endif // WIN32


#include <thread>
#include <QObject>
#include <boost/smart_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <queue>
#include "ChatMessage.h"

using namespace boost::asio;
using namespace boost::system;
using boost::asio::ip::tcp;

typedef std::deque<ChatMessage> ChatMessageDeque;







class ChatClient : public QObject
{

	Q_OBJECT	//

		//信号
		Q_SIGNAL void ReceivePackage();	//只需定义，绑定交给UI完成，Qt自动绑定
private:
	io_service& io_service_;
	tcp::socket	socket_;
	ChatMessage readMsg_;
	ChatMessageDeque writeMsgs_;
	std::vector<ChatMessage> receiveMsgs_;
public:
	ChatClient(io_service& inservice, tcp::resolver::iterator endpoint_iterator) :
		io_service_(inservice),
		socket_(inservice)
	{
		boost::asio::async_connect(socket_, endpoint_iterator,
			bind(&ChatClient::ConnectHandle, this, _1));
	}

	void ConnectHandle(const error_code& error)
	{
		if (!error)
		{
			boost::asio::async_read(socket_,
				boost::asio::buffer(readMsg_.data(), ChatMessage::HEADER_LENGTH),
				boost::bind(&ChatClient::ReadHandleHeader, this, _1));
		}
	}

	void ReadHandleHeader(const error_code& error)
	{
		if (!error && readMsg_.DecodeHeader())
		{
			boost::asio::async_read(socket_,
				boost::asio::buffer(readMsg_.body(), readMsg_.bodyLength()),
				boost::bind(&ChatClient::ReadHandleBody, this, _1));
		}
		else
		{
			DoClose();
		}

	}

	void HandleMessage(void)
	{
		receiveMsgs_.push_back(readMsg_);
		emit ReceivePackage();	//触发这个信号
	}

	ChatMessage GetMessage()
	{
		ChatMessage m = receiveMsgs_[0];
		receiveMsgs_.erase(receiveMsgs_.begin());
		return m;
	}

	void ReadHandleBody(const error_code& error)
	{
		if (!error)
		{
			//std::cout.write(readMsg_.body(), readMsg_.bodyLength());
			//std::cout << "\n";

			//自动发送给Qt
			HandleMessage();

			boost::asio::async_read(socket_,
				boost::asio::buffer(readMsg_.data(), ChatMessage::HEADER_LENGTH),
				boost::bind(&ChatClient::ReadHandleHeader, this, _1));
		}
		else
		{
			DoClose();
		}
	}

	void Send(const ChatMessage& msg)
	{
		//同步
		io_service_.post(boost::bind(&ChatClient::DoWrite, this, msg));
	}

	void Close()
	{
		io_service_.post(boost::bind(&ChatClient::DoClose, this));
	}

	void DoWrite(ChatMessage msg)
	{
		//std::cout << "准备发包" << std::endl;

		writeMsgs_.push_back(msg);
		bool writeInProgress = !writeMsgs_.empty();
		if (writeInProgress)
		{

			boost::asio::async_write(socket_,
				boost::asio::buffer(writeMsgs_.front().data(), writeMsgs_.front().length()),
				boost::bind(&ChatClient::WriteHandle, this, _1));
		}
	}

	void WriteHandle(const error_code& error)
	{
		if (!error)
		{
			writeMsgs_.pop_front();
			if (!writeMsgs_.empty())
			{
				boost::asio::async_write(socket_,
					boost::asio::buffer(writeMsgs_.front().data(), writeMsgs_.front().length()),
					boost::bind(&ChatClient::WriteHandle, this, _1));
			}
		}
		else
		{
			DoClose();
		}
	}

	void DoClose()
	{
		socket_.close();
	}
};
