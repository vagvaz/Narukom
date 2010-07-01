#include "udp_sender.h"
#include <boost/bind.hpp>
#include <iostream>
using std::cout;
using std::endl;

UdpSender::UdpSender(NetworkChannelSender<boost::asio::ip::udp>* owner,const boost::asio::ip::udp::endpoint& endpoint, boost::asio::ip::udp::socket* socket,
                     boost::asio::io_service* io_service, boost::asio::deadline_timer* timer):endpoint_(endpoint), owner(owner)
{

    if (io_service == 0 )
        cout << "io_service must not be null" << endl;

    if (socket == 0 )
    {
        socket_ = new boost::asio::ip::udp::socket(*io_service);
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
UdpSender::~UdpSender()
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




const boost::asio::ip::udp::endpoint& UdpSender::get_endpoint() const
{
    return this->endpoint_;
}
boost::asio::ip::udp::socket* UdpSender::get_socket() const
{
    return socket_;
}
 boost::asio::deadline_timer& UdpSender::get_timer() const
{
    return *timer_;
}
short int UdpSender::get_udp_port() const
{
    return endpoint_.port();
}
NetworkChannelSender<boost::asio::ip::udp>* UdpSender::get_owner() const
{
    return owner;
}

void UdpSender::set_owner(NetworkChannelSender<boost::asio::ip::udp>* val)
{
    owner = val;
}

void UdpSender::set_endpoint(boost::asio::ip::udp::endpoint& val)
{
    endpoint_ = val;
}
void UdpSender::set_socket(boost::asio::ip::udp::socket* socket)
{
    socket_ = socket;
}
void UdpSender::set_timer(boost::asio::deadline_timer* timer)
{
    timer_ = timer;
}

void UdpSender::async_write(const std::string& msg, unsigned int num_of_message, unsigned int num_of_packet,unsigned timeout, boost::asio::ip::udp::endpoint* endpoint)
{
		timer_->cancel();
    boost::asio::ip::udp::endpoint* receiver;
    if (endpoint == 0)
        receiver=&endpoint_;
    else
        receiver=endpoint;

    socket_->async_send_to(boost::asio::buffer(msg),*receiver,
                           boost::bind(&UdpSender::handle_send_to,this,boost::asio::placeholders::error,num_of_message,num_of_packet,*receiver));
    timer_->expires_from_now(boost::posix_time::millisec(timeout));
    timer_->async_wait(boost::bind(&UdpSender::handle_send_timeout,this,boost::asio::placeholders::error,num_of_message,num_of_packet,*receiver));

}
void UdpSender::async_write(const char* msg, unsigned int size, unsigned int num_of_message, unsigned int num_of_packet,unsigned timeout, boost::asio::ip::udp::endpoint* endpoint)
{
    boost::asio::ip::udp::endpoint* receiver;
    if (endpoint == 0)
        receiver=&endpoint_;
    else
        receiver=endpoint;
    socket_->async_send_to(boost::asio::buffer(msg,size),*receiver,
                           boost::bind(&UdpSender::handle_send_to,this,boost::asio::placeholders::error,num_of_message,num_of_packet,*receiver));
    timer_->expires_from_now(boost::posix_time::millisec(timeout));
    timer_->async_wait(boost::bind(&UdpSender::handle_send_timeout,this,boost::asio::placeholders::error,num_of_message,num_of_packet,*receiver));
}
unsigned int UdpSender::sync_write(const char* msg, unsigned int size, boost::asio::ip::udp::endpoint* endpoint)
{
	int result = 0;
	if(endpoint == 0)
		result = socket_->send_to(boost::asio::buffer(msg,size),endpoint_);
	else
		result = socket_->send_to(boost::asio::buffer(msg,size),*endpoint);

	return result;
}
unsigned int UdpSender::sync_write(const std::string& msg, boost::asio::ip::udp::endpoint*  endpoint)
{
	int result = 0;
	cout << "I send to " << msg.size() << endl;
if(endpoint == 0)
{
	result = socket_->send_to(boost::asio::buffer(msg),endpoint_);
	
}
	else
	{
		result = socket_->send_to(boost::asio::buffer(msg),*endpoint);
	
	}
	
	return result;
}
void UdpSender::handle_send_timeout(const boost::system::error_code& error, unsigned int num_of_message, unsigned int num_of_piece, const boost::asio::ip::udp::endpoint& endpoint)
{
	if(!error && owner != 0)
	{
		owner->notify_send_timeout(num_of_message, num_of_piece,endpoint);
	}

}
void UdpSender::handle_send_to(const boost::system::error_code& error, unsigned int num_of_message, unsigned int num_of_piece, const boost::asio::ip::udp::endpoint& endpoint)
{
	if(!error )
	{
		if( owner != 0)
			owner->notfiy_send(num_of_message,num_of_piece,endpoint);
	}
	else
	{
		cout << "UdpSender error: " << error.message() << endl;
	}
}

