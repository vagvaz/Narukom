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

#ifndef NETWORK_MESSAGE_H
#define NETWORK_MESSAGE_H

#include <list>
#include "../pub_sub/tuple.h"
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/composite_key.hpp>
#include "../narukom_messages/NetworkPacket.pb.h"
using namespace boost::multi_index;
struct packet_num
{
	packet_num(NetworkPacket* pckt);
	~packet_num();
	NetworkPacket* packet;
	unsigned get_packet_num();
};
typedef boost::multi_index_container<
packet_num*,
	indexed_by<
		ordered_unique<
		BOOST_MULTI_INDEX_MEM_FUN(packet_num,unsigned, get_packet_num)
																			>
						>
> packet_sequence;


class NetworkMessage
{
	public:
		NetworkMessage(unsigned message_num,Tuple* tuple);
		NetworkMessage(NetworkPacket* packet);
		NetworkMessage(const NetworkMessage& other_msg);
		virtual ~NetworkMessage();
		unsigned get_message_number() const;
		unsigned get_number_added_packets() const;
		unsigned get_number_of_packets() const;
		void set_message_number(unsigned val);
		void set_number_of_packets( unsigned val);
		unsigned add_packet(NetworkPacket* pckt);
		unsigned remove_packet(unsigned packet_num);
		unsigned remove_packet(const NetworkPacket& pckt);
		const NetworkHeader* get_network_header() const;
		std::string* get_serialized_packet(unsigned packt_num) const;
		std::string* get_serialized_msg() const;
		std::list<unsigned>* get_missing_packets() const;
		std::list<NetworkPacket*>* get_added_packets() const;
// 		unsigned add_packet();
	private:
		void create_packets_from_message(NetworkHeader& header, google::protobuf::Message* msg);
		unsigned int message_number;
		unsigned int added_packets;
		unsigned int number_of_packets;
		packet_sequence packets;
		bool is_header_initiliazed;
		NetworkHeader msg_header;
};

#endif // NETWORK_MESSAGE_H
