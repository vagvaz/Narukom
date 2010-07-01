#include "tcp_sender.h"
#include <boost/bind.hpp>
#include <iostream>
using std::cout;
using std::endl;

TcpSender::TcpSender(NetworkChannelSender<boost::asio::ip::tcp>* owner,const boost::asio::ip::tcp::endpoint& endpoint, boost::asio::ip::tcp::socket* socket,
                     boost::asio::io_service* io_service, boost::asio::deadline_timer* timer):endpoint_(endpoint), owner(owner)
{

    if (io_service == 0 )
        cout << "io_service must not be null" << endl;

    if (socket == 0 )
    {
        socket_ = new boost::asio::ip::tcp::socket(*io_service);
    }
    else
    {
        socket_=socket;
    }
    if (!socket_->is_open())
    {
        socket_->open(endpoint_.protocol());
    }
    if (timer != 0 )
        timer_ = timer;
    else
        timer_ = new boost::asio::deadline_timer(*io_service);

}
TcpSender::~TcpSender()
{
	if(socket_ != 0)
	{
		socket_->cancel();
		socket_->close();
		delete socket_;
	}
	if(timer_ != 0)
	{
		delete timer_;
	}
}




const boost::asio::ip::tcp::endpoint& TcpSender::get_endpoint() const
{
    return this->endpoint_;
}
boost::asio::ip::tcp::socket* TcpSender::get_socket() const
{
    return socket_;
}
 boost::asio::deadline_timer& TcpSender::get_timer() const
{
    return *timer_;
}
short int TcpSender::get_tcp_port() const
{
    return endpoint_.port();
}
NetworkChannelSender<boost::asio::ip::tcp>* TcpSender::get_owner() const
{
    return owner;
}

void TcpSender::set_owner(NetworkChannelSender<boost::asio::ip::tcp>* val)
{
    owner = val;
}

void TcpSender::set_endpoint(boost::asio::ip::tcp::endpoint& val)
{
    endpoint_ = val;
}
void TcpSender::set_socket(boost::asio::ip::tcp::socket* socket)
{
    socket_ = socket;
}
void TcpSender::set_timer(boost::asio::deadline_timer* timer)
{
    timer_ = timer;
}

void TcpSender::async_write(const std::string& msg, unsigned int num_of_message, unsigned int num_of_packet,unsigned timeout, boost::asio::ip::tcp::endpoint* endpoint)
{
		timer_->cancel();
    boost::asio::ip::tcp::endpoint* receiver;
    if (endpoint == 0)
        receiver=&endpoint_;
    else
        receiver=endpoint;

    socket_->async_send(boost::asio::buffer(msg),
                           boost::bind(&TcpSender::handle_send_to,this,boost::asio::placeholders::error,num_of_message,num_of_packet,*receiver));
    timer_->expires_from_now(boost::posix_time::millisec(timeout));
    timer_->async_wait(boost::bind(&TcpSender::handle_send_timeout,this,boost::asio::placeholders::error,num_of_message,num_of_packet,*receiver));

}
void TcpSender::async_write(const char* msg, unsigned int size, unsigned int num_of_message, unsigned int num_of_packet,unsigned timeout, boost::asio::ip::tcp::endpoint* endpoint)
{
    boost::asio::ip::tcp::endpoint* receiver;
    if (endpoint == 0)
        receiver=&endpoint_;
    else
        receiver=endpoint;
    socket_->async_write_some(boost::asio::buffer(msg,size),
                           boost::bind(&TcpSender::handle_send_to,this,boost::asio::placeholders::error,num_of_message,num_of_packet,*receiver));
    timer_->expires_from_now(boost::posix_time::millisec(timeout));
    timer_->async_wait(boost::bind(&TcpSender::handle_send_timeout,this,boost::asio::placeholders::error,num_of_message,num_of_packet,*receiver));
}
unsigned int TcpSender::sync_write(const char* msg, unsigned int size, boost::asio::ip::tcp::endpoint* endpoint)
{
	int result = 0;
	if(endpoint == 0)
		result = socket_->send(boost::asio::buffer(msg,size));
	else
		result = socket_->send(boost::asio::buffer(msg,size));

	return result;
}
unsigned int TcpSender::sync_write(const std::string& msg, boost::asio::ip::tcp::endpoint*  endpoint)
{
	int result = 0;
	cout << "I send to " << msg.size() << endl;
if(endpoint == 0)
{
	result = socket_->send(boost::asio::buffer(msg));
	
}
	else
	{
		result = socket_->send(boost::asio::buffer(msg));
	
	}
	
	return result;
}
void TcpSender::handle_send_timeout(const boost::system::error_code& error, unsigned int num_of_message, unsigned int num_of_piece, const boost::asio::ip::tcp::endpoint& endpoint)
{
	if(!error && owner != 0)
	{
		owner->notify_send_timeout(num_of_message, num_of_piece,endpoint);
	}

}
void TcpSender::handle_send_to(const boost::system::error_code& error, unsigned int num_of_message, unsigned int num_of_piece, const boost::asio::ip::tcp::endpoint& endpoint)
{
	if(!error )
	{
		if( owner != 0)
			owner->notfiy_send(num_of_message,num_of_piece,endpoint);
	}
	else
	{
		cout << "TcpSender error: " << error.message() << endl;
	}
}

