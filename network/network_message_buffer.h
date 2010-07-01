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

#ifndef NETWORK_MESSAGE_BUFFER_H
#define NETWORK_MESSAGE_BUFFER_H
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/deadline_timer.hpp>
#include "network_message.h"
#include "../system/Mutex.h"


struct network_msg
{
  network_msg ( NetworkMessage* val );
  virtual ~network_msg();
  NetworkMessage* msg;
  std::string get_host() const;
  unsigned get_message_num()const;
  boost::posix_time::ptime get_timeout() const;
	bool operator==( network_msg& other) const;
  boost::posix_time::ptime timeout;
  unsigned timeouts;
};
using namespace boost::multi_index;
struct network_key : composite_key<
      network_msg*,
      BOOST_MULTI_INDEX_CONST_MEM_FUN ( network_msg,std::string, get_host ),
      BOOST_MULTI_INDEX_CONST_MEM_FUN ( network_msg,unsigned, get_message_num ),
      BOOST_MULTI_INDEX_CONST_MEM_FUN ( network_msg,boost::posix_time::ptime, get_timeout )
      > {};

typedef multi_index_container<
network_msg*,
indexed_by<
ordered_unique< network_key
#if defined(BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
,composite_key_result_less<standard_key::result_type>
#endif
>,
ordered_non_unique
<
BOOST_MULTI_INDEX_CONST_MEM_FUN ( network_msg,boost::posix_time::ptime, get_timeout )
>
>
> network_buffer_index;

class NetworkMessageBuffer
{
public:
	explicit
  NetworkMessageBuffer( unsigned timeout_period = 300);
  NetworkMessageBuffer ( const NetworkMessageBuffer& other );
  int size() const;
  bool isEmpty() const;
  void clear();
  NetworkMessage* add_packet ( NetworkPacket* packet );
  void add_message ( NetworkMessage* msg );
  NetworkMessage* get_message ( std::string host, int message_number );
  NetworkMessage* remove ( std::string host, unsigned message_number );
  NetworkMessage* remove_head();
	const std::list<network_msg*>* get_timeout_messages() const;
	unsigned get_maximum_timeouts()const;
	void set_maximum_timeouts(unsigned val);
	unsigned get_timeout_period() const;
	void set_timeout_period(unsigned val);
	void set_timeout_period( const boost::posix_time::time_duration& val);
  const network_buffer_index& get_message_set() const;
	bool cleanup ( const boost::posix_time::ptime& current_time );
private:
  
  network_buffer_index network_messages;
	unsigned maximum_timeouts;
	Mutex* mx;
  boost::posix_time::time_duration period;
	boost::posix_time::ptime now;
	
	
	std::list<network_msg*> timeout_messages;
};

#endif // NETWORK_MESSAGE_BUFFER_H
