#include "udp_receiver.h"
#include <iostream>
#include <boost/bind.hpp>

using std::cout;
using std::endl;


UdpReceiver::UdpReceiver(NetworkChannelReceiver<boost::asio::ip::udp>* owner,const boost::asio::ip::udp::endpoint& endpoint, boost::asio::ip::udp::socket* socket,
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
	 socket_ = new boost::asio::ip::udp::socket(*io_service);
 }
 if(!socket_->is_open())
	 socket_->open(endpoint_.protocol());
 socket_->bind(endpoint_);
 if(timer != 0)
	 timer_=timer;
 else
 {
	timer_ = new boost::asio::deadline_timer(*io_service);
 }
}

UdpReceiver::~UdpReceiver()
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


const boost::asio::ip::udp::socket* UdpReceiver::get_socket() const
{
  return socket_;
}

boost::asio::deadline_timer& UdpReceiver::get_timer() const
{
  return *timer_;
}

const boost::asio::ip::udp::endpoint& UdpReceiver::get_endpoint() const
{
	return endpoint_;
}

NetworkChannelReceiver< boost::asio::ip::udp >* UdpReceiver::get_owner() const
{
	return owner;
}

void UdpReceiver::set_endpoint(const boost::asio::ip::udp::endpoint& val)
{
	endpoint_ = val;
}

void UdpReceiver::set_owner(NetworkChannelReceiver< boost::asio::ip::udp >* val)
{
	owner = val;
}

void UdpReceiver::set_socket(boost::asio::ip::udp::socket* val)
{
	socket_ = val;
}

void UdpReceiver::set_timer(boost::asio::deadline_timer* val)
{
	timer_= val;
}


unsigned int UdpReceiver::sync_receive(char* data,unsigned sz, boost::asio::ip::udp::endpoint& sender_ , unsigned int timeout)
{
	return socket_->receive_from(boost::asio::buffer(data,sz),sender_);
}

void UdpReceiver::async_receive(unsigned int timeout, boost::asio::ip::udp::endpoint* endpoint)
{
	timer_->cancel(); // cancel timer countdown
	//asynchronous receive from socket
	boost::asio::ip::udp::endpoint* sender;
	if(endpoint ==0)
		sender = &endpoint_;
	else
		sender = endpoint;
	socket_->async_receive_from(  boost::asio::buffer(data_,max_length), *sender,
																boost::bind(&UdpReceiver::handle_receive_from,this,
																boost::asio::placeholders::error,
																boost::asio::placeholders::bytes_transferred,*sender)
															);
	///start timer countdown for the given timeout!
	timer_->expires_from_now(boost::posix_time::millisec(timeout));
	timer_->async_wait(boost::bind(&UdpReceiver::handle_timeout,this,boost::asio::placeholders::error));
}
void UdpReceiver::handle_receive_from(const boost::system::error_code& error, size_t bytes_recvd, const boost::asio::ip::udp::endpoint& endpoint )
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
		std::cout << "There was an error in udp receiver:\n" << error.message() << std::endl;
	}
}

void UdpReceiver::handle_timeout(const boost::system::error_code& error )
{
	if(!error && owner != 0)
		owner->notify_receive_timeout();
	else
	{
		cout << "UdpReceiver Timeout occured " << endl;
	}

}
