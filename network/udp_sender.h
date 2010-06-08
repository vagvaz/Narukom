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

#ifndef UDP_SENSER_H
#define UDP_SENSER_H
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/placeholders.hpp>

#include "../interfaces/network_channel.h"
//typedef void (*notify_send_func_ptr)(unsigned num_of_package, unsigned num_of_message);
//typedef void (*notify_timeout_func_ptr)();
class UdpSender
{
	public:
		UdpSender(NetworkChannelSender<boost::asio::ip::udp>* owner,const boost::asio::ip::udp::endpoint& endpoint,boost::asio::ip::udp::socket* socket,
							boost::asio::io_service* io_service,boost::asio::deadline_timer* timer);
		virtual ~UdpSender();
		const boost::asio::ip::udp::endpoint& get_endpoint() const;
		void set_endpoint(boost::asio::ip::udp::endpoint& val);
		void set_socket(boost::asio::ip::udp::socket* socket);
		void set_timer(boost::asio::deadline_timer* time);
		void  set_owner(NetworkChannelSender<boost::asio::ip::udp>* val);
		boost::asio::ip::udp::socket* get_socket() const;
		boost::asio::deadline_timer& get_timer() const;
		short get_udp_port() const;
		NetworkChannelSender<boost::asio::ip::udp>* get_owner() const;

		unsigned sync_write(const char* msg,unsigned int size, boost::asio::ip::udp::endpoint* endpoint = 0);
		unsigned sync_write(const std::string& msg, boost::asio::ip::udp::endpoint* endpoint= 0);
		void async_write(const std::string& msg,unsigned num_of_message,unsigned num_of_packet,unsigned timeout = 50,
										boost::asio::ip::udp::endpoint* endpoint = 0);
		void async_write(const char* msg, unsigned int size,unsigned num_of_message,unsigned num_of_packet,unsigned timeout = 50,
										boost::asio::ip::udp::endpoint* endpoint = 0);
		void handle_send_to(const boost::system::error_code& error,unsigned int num_of_message,
												unsigned int num_of_piece,const boost::asio::ip::udp::endpoint& endpoint);
		void handle_send_timeout(const boost::system::error_code& error,unsigned int num_of_packet,
														unsigned int message_num,const boost::asio::ip::udp::endpoint& endpoint);
  private:
    boost::asio::ip::udp::endpoint endpoint_;
    boost::asio::ip::udp::socket* socket_;
    boost::asio::deadline_timer* timer_;
    short udp_port;
		NetworkChannelSender<boost::asio::ip::udp>* owner;
};

#endif // UDP_SENSER_H