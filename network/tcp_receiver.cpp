 #include "tcp_receiver.h"
#include <iostream>
#include <boost/bind.hpp>

using std::cout;
using std::endl;


TcpReceiver::TcpReceiver(NetworkChannelReceiver<boost::asio::ip::tcp>* owner,const boost::asio::ip::tcp::endpoint& endpoint, boost::asio::ip::tcp::socket* socket,
												 boost::asio::io_service* io_service, boost::asio::deadline_timer* timer) : endpoint_(endpoint), owner(owner)
{
 
 	if(io_service == 0 )
		cout << "io_service must not be null" << endl;
 if(socket != 0)
 {
	socket_= socket;
 }
 else
 {
	 socket_ = new boost::asio::ip::tcp::socket(*io_service);
 }
 if(!socket_->is_open())
	 socket_->open(endpoint_.protocol());

 if(timer != 0)
	 timer_=timer;
 else
 {
	timer_ = new boost::asio::deadline_timer(*io_service);
 }
}

TcpReceiver::~TcpReceiver()
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


const boost::asio::ip::tcp::socket* TcpReceiver::get_socket() const
{
  return socket_;
}

boost::asio::deadline_timer& TcpReceiver::get_timer() const
{
  return *timer_;
}

const boost::asio::ip::tcp::endpoint& TcpReceiver::get_endpoint() const
{
	return endpoint_;
}

NetworkChannelReceiver< boost::asio::ip::tcp >* TcpReceiver::get_owner() const
{
	return owner;
}

void TcpReceiver::set_endpoint(const boost::asio::ip::tcp::endpoint& val)
{
	endpoint_ = val;
}

void TcpReceiver::set_owner(NetworkChannelReceiver< boost::asio::ip::tcp >* val)
{
	owner = val;
}

void TcpReceiver::set_socket(boost::asio::ip::tcp::socket* val)
{
	socket_ = val;
}

void TcpReceiver::set_timer(boost::asio::deadline_timer* val)
{
	timer_= val;
}


unsigned int TcpReceiver::sync_receive(char* data,unsigned sz, boost::asio::ip::tcp::endpoint& sender_ , unsigned int timeout)
{
	clear(data_,MAX_PACKET_SIZE);
	return socket_->receive(boost::asio::buffer(data,sz));
}

void TcpReceiver::async_receive(unsigned int timeout, boost::asio::ip::tcp::endpoint* endpoint)
{

	timer_->cancel(); // cancel timer countdown
	//asynchronous receive from socket
	boost::asio::ip::tcp::endpoint* sender;
	if(endpoint ==0)
		sender = &endpoint_;
	else
		sender = endpoint;
	socket_->async_receive( boost::asio::buffer(data_,max_length),
																boost::bind(&TcpReceiver::handle_receive_from,this,
																boost::asio::placeholders::error,
																boost::asio::placeholders::bytes_transferred,*sender)
															);
	///start timer countdown for the given timeout!
	timer_->expires_from_now(boost::posix_time::millisec(timeout));
	timer_->async_wait(boost::bind(&TcpReceiver::handle_timeout,this,boost::asio::placeholders::error));
}
void TcpReceiver::handle_receive_from(const boost::system::error_code& error, size_t bytes_recvd, const boost::asio::ip::tcp::endpoint& endpoint )
{
	
	if(!error)
	{
		if(owner != 0)
			owner->notify_receive(data_,bytes_recvd,endpoint);
		else
			cout << "Asynchronous data received but no function to call!" << endl;
	}
	else
	{
		std::cout << "There was an error in tcp receiver:\n" << error.message() << std::endl;
	}
	clear(data_,MAX_PACKET_SIZE);
}
void TcpReceiver::clear(char* bytes, unsigned sz)
{
	for(unsigned int i =0; i< sz;i++)
		bytes[i] =0;

}
void TcpReceiver::handle_timeout(const boost::system::error_code& error )
{
	clear(data_,MAX_PACKET_SIZE);
	if(!error && owner != 0)
		owner->notify_receive_timeout();
	else
	{
		cout << "TcpReceiver Timeout occured " << endl;
	}

}
