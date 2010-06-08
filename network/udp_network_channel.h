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

#ifndef UDP_NETWORK_CHANNEL_H
#define UDP_NETWORK_CHANNEL_H
#include "broadcast_channel.h"
#include "../interfaces/network_channel.h"
#include "udp_receiver.h"
#include "udp_sender.h"

class NetworkMessageBuffer;
class NetworkMessageIndex;

class UdpNetworkChannel : public BroadcastChannel,NetworkChannelSender<boost::asio::ip::udp>, NetworkChannelReceiver<boost::asio::ip::udp>
{
  public:
    UdpNetworkChannel(const std::string& channel_name,const std::string& ip_address, short int port);
    virtual ~UdpNetworkChannel();
		virtual int Execute();
    virtual void receive(std::string* host, Tuple* data);
    virtual void send(const Tuple& t);
    virtual void start();
    virtual void stop();
		virtual void process_messages();
		void write_to_network();
		void read_from_network();
		void async_read_from_network();
		void async_write_to_network();
		void respond_nack();
		void send_nack();
		void cleanup();
    virtual void notfiy_send(unsigned int num_of_message, unsigned int num_of_piece, boost::asio::ip::basic_endpoint< boost::asio::ip::udp > );
    virtual void notify_receive(char* buffer_data, unsigned int sz, boost::asio::ip::basic_endpoint< boost::asio::ip::udp > );
    virtual void notify_receive_timeout();
    virtual void notify_send_timeout(unsigned int num_of_message, unsigned int num_of_piece, boost::asio::ip::basic_endpoint< boost::asio::ip::udp > );
  private:
		boost::posix_time::time_duration cleanup_period;
		boost::posix_time::ptime now;
		boost::asio::io_service* io_service;
		boost::asio::ip::udp::socket* socket;
    boost::asio::ip::udp::endpoint address;
		NetworkMessageBuffer* incoming;
		NetworkMessageBuffer* outgoing;
		NetworkMessageIndex* index_incoming;
		UdpReceiver* receiver;
		UdpSender* sender;
		std::vector< std::pair<unsigned int, unsigned int> > timed_out_packets;
};

#endif // UDP_NETWORK_CHANNEL_H
