#ifdef WIN32
#define _WIN32_WINNT 0x0501
#endif // WIN32


#include <thread>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <queue>
#include <set>
#include "Server.h"
#include "QMessageBox"
#include "ChatMessage.h"




using namespace boost::asio;
using namespace boost::system;
using boost::asio::ip::tcp;


class ChatParticipant
{


public:
	virtual ~ChatParticipant() {}
	virtual void Deliver(const ChatMessage& msg) = 0;

};


typedef std::deque<ChatMessage> ChatMessageDeque;

class ChatRoom
{
private:
	std::set<boost::shared_ptr<ChatParticipant> > participants_;
	enum Length
	{
		MAX_RECENT_MESSAGE = 100	//发送用户100条信息
	};
	ChatMessageDeque recentMsgs_;
public:
	void JoinRoom(boost::shared_ptr<ChatParticipant> participant)
	{
		//std::cout << "有用户【加入】聊天室" << std::endl;
		participants_.insert(participant);
		//给新加入的用户发送历史信息
		std::for_each(recentMsgs_.begin(), recentMsgs_.end(),
			boost::bind(&ChatParticipant::Deliver, participant, _1));
	}

	void Leave(boost::shared_ptr<ChatParticipant> participant)
	{
		//std::cout << "有用户【离开】聊天室" << std::endl;
		participants_.erase(participant);
	}

	void Deliver(const ChatMessage& msg)
	{
		recentMsgs_.push_back(msg);

		while (recentMsgs_.size() > MAX_RECENT_MESSAGE)
		{
			recentMsgs_.pop_front();
		}

		std::for_each(participants_.begin(), participants_.end(),
			boost::bind(&ChatParticipant::Deliver, _1, boost::ref(msg))
		);
	}

};

class ChatSession : public ChatParticipant,
	public boost::enable_shared_from_this<ChatSession>
{
private:
	tcp::socket socket_;
	ChatMessage readMsg_;
	ChatRoom& room_;
	ChatMessageDeque writeMsgs_;
public:
	ChatSession(io_service& ioservice, ChatRoom& room) :
		socket_(ioservice),
		room_(room)
	{

	}

	~ChatSession()
	{

	}

	tcp::socket& socket()
	{
		return socket_;
	}


	void Start()
	{
		room_.JoinRoom(shared_from_this());

		boost::asio::async_read(socket_,
			boost::asio::buffer(readMsg_.data(), ChatMessage::HEADER_LENGTH),
			bind(&ChatSession::ReadHandleHeader, shared_from_this(), _1));
	}

	void ReadHandleHeader(const error_code& error)
	{
		if (!error && readMsg_.DecodeHeader())
		{
			//std::cout << "内容长度为：" << readMsg_.bodyLength();

			boost::asio::async_read(socket_,
				boost::asio::buffer(readMsg_.body(), readMsg_.bodyLength()),
				bind(&ChatSession::ReadHandleBody, shared_from_this(), _1));
		}
		else
		{
			room_.Leave(shared_from_this());//踢出去
		}
	}

	void ReadHandleBody(const error_code& error)
	{
		if (!error)
		{
			std::string str(readMsg_.body(), readMsg_.bodyLength());
			//std::cout << "内容为：" << str
			//	<< "内容长度:" << readMsg_.bodyLength() << std::endl;
			//Start();
			room_.Deliver(readMsg_);
			boost::asio::async_read(socket_,
				boost::asio::buffer(readMsg_.data(), ChatMessage::HEADER_LENGTH),
				bind(&ChatSession::ReadHandleHeader, shared_from_this(), _1)
			);
		}
		else
		{
			room_.Leave(shared_from_this());
		}
	}


	virtual void Deliver(const ChatMessage& msg)
	{
		bool writeInProgress = !writeMsgs_.empty();

		writeMsgs_.push_back(msg);

		if (!writeInProgress)
		{
			boost::asio::async_write(socket_,
				buffer(writeMsgs_.front().data(), writeMsgs_.front().length()),
				boost::bind(&ChatSession::WriteHandle, shared_from_this(), _1));
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
					boost::bind(&ChatSession::WriteHandle, shared_from_this(), _1));
			}
		}
		else
		{
			room_.Leave(shared_from_this());
		}
	}
};


class ChatServer
{
private:
	io_service& io_service_;
	tcp::acceptor acceptor_;
	ChatRoom chatRoom_;
public:
	ChatServer(io_service& inService, ip::tcp::endpoint inEndPoint) :
		io_service_(inService),
		acceptor_(inService, inEndPoint)
	{
		StartAccept();
	}

	void StartAccept()
	{
		boost::shared_ptr<ChatSession> newSession(boost::make_shared<ChatSession>(io_service_, chatRoom_));

		acceptor_.async_accept(newSession->socket(), boost::bind(&ChatServer::AcceptHandle, this, newSession, _1));
	}

	void AcceptHandle(boost::shared_ptr<ChatSession> session, const error_code& error)
	{
		if (!error)
		{
			//std::cout << "IP地址：" << session->socket().remote_endpoint().address() <<
			//	"端口号" << session->socket().remote_endpoint().port() << std::endl;
			session->Start();
		}
		StartAccept();
	}
};






Server::Server(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	//ui.lineEdit->setText(QString::fromLocal8Bit("你好世界"));
	ui.lineEdit->setText("8888");
	connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(SlotStartServer()));
	//connect(ui.pushButton_2, SIGNAL(clicked()), this, SLOT(SlotStopServer()));
}
Server::~Server() {}

void Server::SlotStartServer()
{
	QMessageBox::critical(NULL, QString::fromLocal8Bit("标题"), ui.lineEdit->text(), QMessageBox::Ok);

	std::thread  t([&]() {

		io_service ioservice;

		ip::tcp::endpoint endpoint(ip::tcp::v4(), 8888);

		ChatServer chatServer(ioservice, endpoint);

		ioservice.run();
	});

	t.detach();

}

void Server::SlotStopServer()
{

}
