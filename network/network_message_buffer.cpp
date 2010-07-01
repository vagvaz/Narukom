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

#include "network_message_buffer.h"
#include <iostream>
using std::cout;
using std::endl;


//network_msg
std::string network_msg::get_host() const
{
	if(msg->get_network_header() != 0)
		return msg->get_network_header()->host();
	return "";
}

unsigned int network_msg::get_message_num() const
{
	return msg->get_message_number();
}

boost::posix_time::ptime network_msg::get_timeout() const
{
	return timeout;
}

network_msg::network_msg ( NetworkMessage* val )
{
	msg =val;
	timeouts = 0;
	timeout = boost::posix_time::microsec_clock::local_time() + boost::posix_time::millisec(msg->get_network_header()->timeout());
}

network_msg::~network_msg()
{
	delete msg;
}
bool network_msg::operator==(network_msg& other) const
{
	return this->get_message_num() == other.get_message_num();
}



/************************/
//NetworkMessageBuffer

NetworkMessageBuffer::NetworkMessageBuffer(unsigned timeout_period )
{
	period = boost::posix_time::millisec(timeout_period);

}
NetworkMessageBuffer::NetworkMessageBuffer ( const NetworkMessageBuffer& other )
{
	network_buffer_index::iterator other_indx = other.get_message_set().begin();
	for(; other_indx != other.get_message_set().end(); other_indx++)
	{
		add_message((*other_indx)->msg);
	}
	
}

const network_buffer_index& NetworkMessageBuffer::get_message_set() const
{
	return network_messages;
}
bool NetworkMessageBuffer::isEmpty() const
{
	return size() == 0;
}

unsigned int NetworkMessageBuffer::get_timeout_period() const
{
	return period.total_milliseconds();
}

void NetworkMessageBuffer::set_timeout_period ( unsigned int val )
{
	period = boost::posix_time::millisec(val);
}

void NetworkMessageBuffer::set_timeout_period ( const boost::posix_time::time_duration& val )
{
	period = val;
}

unsigned int NetworkMessageBuffer::get_maximum_timeouts() const
{
	return maximum_timeouts;
}

void NetworkMessageBuffer::set_maximum_timeouts ( unsigned int val )
{
	maximum_timeouts = val;
}


int NetworkMessageBuffer::size() const
{
	return network_messages.size();
}

NetworkMessage* NetworkMessageBuffer::remove ( std::string host, unsigned message_number )
{
	NetworkMessage* result;
	network_buffer_index::iterator iter =  network_messages.find(boost::make_tuple(host,message_number));
	if(iter != network_messages.end())
	{
		result =  (*iter)->msg;
		network_messages.erase(iter);
	}
	else
		result =  0;
	return result;
}

void NetworkMessageBuffer::add_message ( NetworkMessage* msg )
{
	if(msg != 0 && msg->get_network_header() != 0)
	{
		network_buffer_index::iterator it = network_messages.find(boost::make_tuple(msg->get_network_header()->host(),msg->get_message_number()));
		if(it == network_messages.end())
			network_messages.insert(new network_msg(msg));
		else
		{
			cout << "Network Message Already in buffer replacing " << endl;
			  network_messages.erase(it);
        network_messages.insert(new network_msg(msg));
		}
	}
	else
		cout << "Adding either null message or message without any packet " << endl;
}

NetworkMessage* NetworkMessageBuffer::add_packet ( NetworkPacket* packet )
{
  network_buffer_index::iterator it = network_messages.find ( boost::make_tuple ( packet->header().host(),packet->header().message_num() ) );

  NetworkMessage* msg;
  if ( it == network_messages.end() )
  {

    msg= new NetworkMessage ( packet );
    network_messages.insert ( new network_msg(msg) );

  }
  else
  {
    msg = ( *it )->msg;
    msg->add_packet ( packet );
  }

  return msg;
}

NetworkMessage* NetworkMessageBuffer::get_message ( std::string host, int message_number )
{
network_buffer_index::iterator it = network_messages.find ( boost::make_tuple ( host,message_number ) );
if(it != network_messages.end())
	return (*it)->msg;
return 0;
}


bool NetworkMessageBuffer::cleanup(const boost::posix_time::ptime& current_time)
{
// 	now = boost::posix_time::microsec_clock::local_time();
  if(current_time -now < period)
		return false;
	
	timeout_messages.clear();

    network_buffer_index::nth_index<1>::type& message_index = network_messages.get<1>();
    network_buffer_index::nth_index<1>::type::iterator iter = message_index.begin();
    if (iter != message_index.end())
    {
        while (iter != message_index.end() && (*iter)->get_timeout() < current_time)
        {
            if ((*iter)->timeouts < maximum_timeouts)
            {
                network_msg* nmsg = (*iter);
                nmsg->timeouts++;
                nmsg->timeout = current_time + boost::posix_time::millisec(nmsg->msg->get_network_header()->timeout());
                timeout_messages.push_back(nmsg);
                message_index.replace(iter,nmsg);
                iter = message_index.begin();
            }
            else
            {
                
                network_msg* nmsg = (*iter);
                message_index.erase(iter);
                delete nmsg;
                iter = message_index.begin();
            }
        }
      
    }
    for (std::list<network_msg*>::iterator list_it = timeout_messages.begin(); list_it != timeout_messages.end(); list_it++)
    {
        message_index.insert((*list_it));
    }
		now = current_time;
		if(timeout_messages.size() > 0 )
			return true;
		return false;
}
const std::list< network_msg* >* NetworkMessageBuffer::get_timeout_messages() const
{
	return &timeout_messages;
}

void NetworkMessageBuffer::clear()
{
	network_messages.clear();
}


NetworkMessage* NetworkMessageBuffer::remove_head()
{
	NetworkMessage* result = (*network_messages.begin())->msg;;
	network_messages.erase(network_messages.begin());
	return result;
	
}