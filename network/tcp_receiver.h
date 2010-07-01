/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#ifndef TCP_RECEIVER_H
#define TCP_RECEIVER_H
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "network_common.h"
#include <boost/asio.hpp>
#include <boost/asio/placeholders.hpp>
#include "../interfaces/network_channel.h"
// #include <boost/asio/ip/tcp.hpp>

class TcpReceiver
{
public:
	TcpReceiver(NetworkChannelReceiver<boost::asio::ip::tcp>* owner, const boost::asio::ip::tcp::endpoint& endpoint,
							boost::asio::ip::tcp::socket* socket = 0,boost::asio::io_service* io_service = 0, boost::asio::deadline_timer* timer = 0);
	virtual ~TcpReceiver();

	const boost::asio::ip::tcp::socket* get_socket() const;
  boost::asio::deadline_timer& get_timer() const;
  const boost::asio::ip::tcp::endpoint& get_endpoint() const;
	NetworkChannelReceiver<boost::asio::ip::tcp>* get_owner() const;

	void set_socket(boost::asio::ip::tcp::socket* val);
	void set_timer(boost::asio::deadline_timer* val);
	void set_endpoint(const boost::asio::ip::tcp::endpoint& val);
	void set_owner(NetworkChannelReceiver<boost::asio::ip::tcp>* val);
	
	unsigned sync_receive(char* data,unsigned sz ,boost::asio::ip::tcp::endpoint&, unsigned timeout);
	void async_receive(unsigned timeout =0, boost::asio::ip::tcp::endpoint* endpoint = 0);
	
private:
	void clear(char* bytes, unsigned sz);
	void handle_receive_from(const boost::system::error_code& error, size_t bytes_recvd, const boost::asio::ip::tcp::endpoint& endpoint);
	void handle_timeout(const boost::system::error_code& error);
  boost::asio::ip::tcp::endpoint endpoint_;
  boost::asio::ip::tcp::socket* socket_;
  boost::asio::deadline_timer* timer_;
  enum { max_length = MAX_PACKET_SIZE};
	char data_[max_length];
	NetworkChannelReceiver<boost::asio::ip::tcp>* owner;
};




#endif // UDP_RECEIVER_H
