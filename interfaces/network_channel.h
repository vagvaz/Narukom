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
#include <boost/asio/ip/host_name.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/basic_endpoint.hpp>
#include <system/Mutex.h>
template<class InternetProtocol>
class NetworkChannelSender
{
	public:
		 unsigned get_next_message_number()
		{
			m_lock.lock();
			current_message++;
			m_lock.unlock();
			return current_message;
		}
		virtual std::string get_hostname() const{ return boost::asio::ip::host_name();}
		virtual void notfiy_send(unsigned num_of_message,unsigned num_of_piece,const boost::asio::ip::basic_endpoint<InternetProtocol>& ) = 0;
		virtual void notify_send_timeout(unsigned num_of_message,unsigned num_of_piece,const boost::asio::ip::basic_endpoint<InternetProtocol>&) = 0;
	private:
		static unsigned current_message;

		Mutex m_lock; 
};
template<class InternetProtocol>
unsigned NetworkChannelSender<InternetProtocol>::current_message = 0;

template<class InternetProtocol>
class NetworkChannelReceiver
{
	public:
		bool get_deliver_in_order() const { return deliver_in_order;}
		void set_deliver_in_order(bool val){deliver_in_order = val;}
		virtual void notify_receive(char* buffer_data, unsigned sz,const  boost::asio::ip::basic_endpoint<InternetProtocol>& ) = 0;
		virtual void notify_receive_timeout() = 0;
	private:
		bool deliver_in_order;
};
#endif // NETWORK_CHANNEL_H
