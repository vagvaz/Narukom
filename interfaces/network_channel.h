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

#ifndef NETWORK_CHANNEL_H
#define NETWORK_CHANNEL_H
#include <string>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/basic_endpoint.hpp>
template<class InternetProtocol>
class NetworkChannelSender
{
	public:
		
		
		virtual void notfiy_send(unsigned num_of_message,unsigned num_of_piece,boost::asio::ip::basic_endpoint<InternetProtocol>) = 0;
		virtual void notify_send_timeout(unsigned num_of_message,unsigned num_of_piece,boost::asio::ip::basic_endpoint<InternetProtocol>) = 0;
};


template<class InternetProtocol>
class NetworkChannelReceiver
{
	public:

		virtual void notify_receive(char* buffer_data, unsigned sz, boost::asio::ip::basic_endpoint<InternetProtocol> ) = 0;
		virtual void notify_receive_timeout() = 0;
};
#endif // NETWORK_CHANNEL_H
