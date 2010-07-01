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

#include "udp_network_channel.h"
#include "network_common.h"
#include <iostream>
#include <limits>
using std::cout;
using std::endl;

void network_msg_finger_print::deliver ( const std::string& host, unsigned int msg )
{
    std::map<std::string,std::list<unsigned>* >::iterator iter =  delivered_messages.find ( host );
    if ( iter != delivered_messages.end() )
    {
        std::list<unsigned>* host_list = iter->second;
        if ( msg <= host_list->front() )
            return;
        if ( msg - host_list->front() == 1 )
        {
            fix_front ( *host_list,msg,false );
            return;
        }
        if ( msg > host_list->back() )
        {
            host_list->push_back ( msg );
            return;
        }
        std::list<unsigned>::iterator list_it = host_list->begin();
        while ( list_it != host_list->end() && ( *list_it ) < msg )
        {
            list_it++;
        }
        host_list->insert ( list_it,msg );
        return;
    }
    else
    {
        std::list< unsigned >* new_list = new std::list<unsigned>();
        new_list->push_back ( msg );
    }
}
std::list< unsigned int >* network_msg_finger_print::deliver_in_order ( const std::string& host, unsigned int msg )
{

    std::map<std::string,std::list<unsigned>* >::iterator iter =  delivered_messages.find ( host );
    if ( iter != delivered_messages.end() )
    {
        std::list<unsigned>* host_list = iter->second;
        if ( msg <= host_list->front() )
            return new std::list<unsigned>();
        if ( msg - host_list->front() == 1 )
        {
            return fix_front ( *host_list,msg,true );
        }
        if ( msg > host_list->back() )
        {
            host_list->push_back ( msg );
            return new std::list<unsigned>();
        }
        std::list<unsigned>::iterator list_it = host_list->begin();
        while ( list_it != host_list->end() && ( *list_it ) < msg )
        {
            list_it++;
        }
        host_list->insert ( list_it,msg );
        return new std::list<unsigned>();
    }
    else
    {
        std::list< unsigned >* new_list = new std::list<unsigned>();
        new_list->push_back ( msg );
        return new std::list<unsigned> ( new_list->begin(),new_list->end() );
    }

    return new std::list<unsigned>();
}
bool network_msg_finger_print::is_delivered ( const std::string& host, unsigned int msg )
{
    std::map<std::string,std::list<unsigned>* >::iterator iter =  delivered_messages.find ( host );
    if ( iter == delivered_messages.end() )
        return false;
    else
    {
        std::list<unsigned>* host_list = iter->second;
        std::list<unsigned>::iterator list_it = host_list->begin();
        while ( list_it != host_list->end() )
        {
            if ( ( *list_it ) == msg )
                return true;
            list_it++;
        }
        return false;

    }
}
std::list< unsigned int >* network_msg_finger_print::fix_front ( std::list<unsigned>& alist, unsigned int msg_num, bool in_order )
{
    std::list<unsigned>* result;
    std::list<unsigned>::iterator list_it =alist.begin();
    if ( in_order )
    {
        result = new std::list<unsigned>();
        alist.pop_front();
        alist.push_front ( msg_num );

        unsigned int i = 0;
        while ( list_it != alist.end() && ( *list_it ) - msg_num == i )
        {
            result->push_back ( ( *list_it ) );
            i++;
            list_it++;
        }
        alist.erase ( alist.begin(),list_it-- );
        alist.push_front ( msg_num + i-1 );
        return result;
    }
    else
    {
        alist.pop_front();
        alist.push_front ( msg_num );

        unsigned int i = 0;
        while ( list_it != alist.end() && ( *list_it ) - msg_num == i )
        {
            i++;
            list_it++;
        }
        alist.erase ( alist.begin(),list_it-- );
        alist.push_front ( msg_num + i-1 );
        return 0;
    }
}


UdpNetworkChannel::UdpNetworkChannel ( const std::string& channel_name, const std::string& ip_address, short int port ,
																			 unsigned period) : BroadcastChannel ( channel_name ),
																			 NetworkChannelSender<boost::asio::ip::udp>(), NetworkChannelReceiver<boost::asio::ip::udp>()
{
    //Network Initialization
    mx = new Mutex;
    io_service = new boost::asio::io_service();
//		boost::asio::ip::address addr(ip_address);
//  address ( boost::asio::ip::address::from_string(ip_address),port)
     address.address ( boost::asio::ip::address::from_string ( ip_address ) );
		 
		 address.port(port);
		
    socket = new boost::asio::ip::udp::socket ( *io_service,address );
    receiver = new UdpReceiver ( this,address,socket,io_service,0 );
    sender   = new UdpSender ( this,address,socket,io_service,0 );
    socket->set_option ( boost::asio::ip::multicast::join_group ( address.address() ) );
    socket->set_option ( boost::asio::ip::multicast::enable_loopback ( false ) );

		sender_timeout = 50;
    receiver_timeout = 50;;
    outgoing_timeout = 300;
    incoming_timeout = 300;
    outgoing = new NetworkMessageBuffer (outgoing_timeout );
    incoming = new NetworkMessageBuffer ( incoming_timeout );
    outgoing->set_maximum_timeouts(3);
    incoming->set_maximum_timeouts(3);

    cleanup_period = boost::posix_time::millisec(period);
    current_time = boost::posix_time::microsec_clock::local_time();
}


UdpNetworkChannel::~UdpNetworkChannel()
{

    outgoing->clear();
    incoming->clear();
    delete outgoing;
    delete incoming;
    delete socket;
    delete receiver;
    delete sender;
    if ( io_service != 0 )
        delete io_service;
		    
		if(mx !=0)
		{
			mx->unlock();
			delete mx;
		}

}

void UdpNetworkChannel::async_read_from_network()
{
    receiver->async_receive ( receiver_timeout );
}

void UdpNetworkChannel::async_write_to_network()
{
		NetworkPacket* packet = outgoing_queue.front();
		outgoing_queue.pop();
    std::string* buffer_data = new std::string();
		packet->SerializeToString(buffer_data);
    sender->async_write ( *buffer_data,packet->header().message_num(),packet->header().packet_num(),sender_timeout );
}
void UdpNetworkChannel::cleanup()
{
// 	cout << "Cleanup " << endl;
    outgoing->cleanup(current_time);
		
}

void UdpNetworkChannel::nack_cleanup()
{
// 	cout << "NCleanup " << endl;
		incoming->cleanup(current_time);
    const std::list<network_msg*>* timeout_msg =  incoming->get_timeout_messages();
    for (std::list<network_msg*>::const_iterator it = timeout_msg->begin(); it != timeout_msg->end();it++)
    {
        send_nack((*it)->msg);
    }
}
int UdpNetworkChannel::Execute()
{
// 	cout << "Execute " << endl;
		io_service->reset();
    boost::posix_time::ptime tmp_current_time = boost::posix_time::microsec_clock::local_time();
		
    if (tmp_current_time - current_time > cleanup_period)
    {
        current_time = tmp_current_time;
        cleanup();
        nack_cleanup();
    }
    process_messages();
    async_read_from_network();
    write_to_network();
    io_service->run();
    return 0;
}

void UdpNetworkChannel::notfiy_send ( unsigned int num_of_message, unsigned int num_of_piece,const  boost::asio::ip::basic_endpoint< boost::asio::ip::udp >& rec )
{
    cout << "Async send of msg: " << num_of_message << " packet: " << num_of_piece << " to " <<  rec << endl;
}

void UdpNetworkChannel::notify_receive ( char* buffer_data, unsigned int sz, const  boost::asio::ip::basic_endpoint< boost::asio::ip::udp >& send )
{
    NetworkPacket* packet = new NetworkPacket;
    
    if(!packet->ParseFromArray(buffer_data,sz))
		{
			cout << "Unserialization failed" << endl;
			return;
		}
		else
		{
			cout << " serialization was true " << endl;
		}
		cout << "receive data of size: " << sz << " from " << send << " where " << packet->byte_buffer().size() <<  endl;
    if (hosts.right.find(send) == hosts.right.end())
        hosts.insert(hosts_bm::value_type(packet->header().host(),send )  );
    if (packet->header().type() == "NackMessage")
    {
        NackMessage nack;
        nack.ParseFromString(packet->byte_buffer().byte_stream());
        if (nack.receiver() == get_hostname())
				{
					respond_nack(nack);
					return;
				}
    }
    NetworkMessage *msg = incoming->add_packet(packet);
    check_msg_and_deliver(msg);

    async_read_from_network();
}


void UdpNetworkChannel::notify_receive_timeout()
{
		
//     cout << "receive timed out " << endl;
		io_service->stop();
}
void UdpNetworkChannel::notify_send_timeout ( unsigned int num_of_message, unsigned int num_of_piece, const  boost::asio::ip::basic_endpoint< boost::asio::ip::udp >& )
{
    timed_out_packets.push_back(std::make_pair<unsigned,unsigned>(num_of_message,num_of_piece));
}
void UdpNetworkChannel::process_messages()
{
    MessageBuffer* buf = Subscriber::getBuffer();
    if (buf != 0 && buf->size() > 0)
    {
        Tuple* cur = buf->remove_head();
        while (cur != 0)
        {
            cur->set_host(get_hostname());
						cout << " Udpnewtorkchannel: " << "msg sz: " << cur->get_msg_data()->ByteSize() << endl;
            NetworkMessage* net_msg = new NetworkMessage( get_next_message_number(),cur);
            outgoing->add_message(net_msg);
            delete cur;
            cur = buf->remove_head();
						std::list<NetworkPacket*>* packet_list = net_msg->get_added_packets();
						for(std::list<NetworkPacket*>::iterator it = packet_list->begin() ; it !=packet_list->end(); it++)
						{
							
							NetworkPacket* outgoing_packet =  (*it);
							cout << "UdpNetworkChannel: pack sz " << outgoing_packet->ByteSize() << " heeader " << outgoing_packet->header().ByteSize() << endl;
							outgoing_queue.push(outgoing_packet);
							cout << "Adding to outgoing packet " << outgoing_queue.size() << " "<<outgoing_packet->header().message_num() << " " << outgoing_packet->header().packet_num()  <<endl;
						}
        }
    }
}
bool UdpNetworkChannel::check_msg_and_deliver(NetworkMessage* msg)
{
    if (msg != 0)
    {
        if (msg->get_number_added_packets() == msg->get_number_of_packets())
        {
            if (index_incoming.is_delivered(msg->get_network_header()->host(),msg->get_message_number() ) )
                return false;//do nothing
            else
            {
                std::string* serialized_data = msg->get_serialized_msg();
                RawBytes*  serialized_msg = new RawBytes;
                serialized_msg->mutable_byte_stream()->append(*serialized_data);
                const NetworkHeader* header = msg->get_network_header();
                Tuple* t = new Tuple(serialized_msg,header->host(),header->publisher(),header->topic(),"",header->timeout());
								t->set_sec_publisher(Publisher::getName());
                Publisher::publish(t);

                incoming->remove(t->get_host(),msg->get_message_number());
                delete t;
                delete msg;
                delete serialized_data;
                delete serialized_msg;
                return true;
            }

        }
    }
    else
    {
        cout << "null message in check msg and deliver" << endl;
        return false;
    }

    return false;
}


void UdpNetworkChannel::read_from_network()
{
    char* buffer = new char[DEFAULT_PACKET_SIZE];
    unsigned read_bytes = receiver->sync_receive(buffer,DEFAULT_PACKET_SIZE,address,sender_timeout);
    if (read_bytes > 0 )
    {
        NetworkPacket* new_packet = new  NetworkPacket;
        new_packet->ParseFromArray(buffer,read_bytes);
        if (new_packet->header().type() == "NackMessage")
        {
            NackMessage nack;
            nack.ParseFromString(new_packet->byte_buffer().byte_stream());
            if (nack.receiver() == get_hostname())
						{
							respond_nack(nack);
							return;
						}
        }
        NetworkMessage* msg  = incoming->add_packet(new_packet);
        check_msg_and_deliver(msg);
    }
    else
    {
        cout << "No Data received from: " << address.address().to_string() << " on port " << address.port() << endl;
    }
}

void UdpNetworkChannel::receive ( std::string* host, Tuple* data )
{
    cout << "uneeded receive function " << endl;
}
void UdpNetworkChannel::respond_nack(const NackMessage& nack)
{

}
void UdpNetworkChannel::send_nack(NetworkMessage* msg)
{
    std::list<unsigned>* missing = msg->get_missing_packets();
    NackMessage nack;
		nack.set_receiver(msg->get_network_header()->host());
    nack.set_message_num(msg->get_message_number());
    for (std::list<unsigned>::iterator it = missing->begin(); it != missing->end(); it++)
        nack.add_missing_packets((*it));
    NetworkHeader nack_header;
    nack_header.set_host(get_hostname());
    nack_header.set_message_num(0);
    nack_header.set_number_of_packets(1);
    nack_header.set_packet_num(0);
    nack_header.set_publisher("UdpNetworkChannel");
    nack_header.set_timeout(sender_timeout*3);
    nack_header.set_topic("system");
    nack_header.set_timestamp("9999-Dec-31");
    nack_header.set_type(nack.GetTypeName());
    std::string data = "";
    nack_header.SerializeToString(&data);
    nack.SerializeToString(&data);
    sender->sync_write(data,&address);
    delete missing;
}

void UdpNetworkChannel::send ( const Tuple& t )
{
    cout << "unneeded function from udp_network_channel send( Tuple )" << endl;
}

void UdpNetworkChannel::start()
{
    Channel::start();

}
void UdpNetworkChannel::stop()
{
    Channel::stop();
}
void UdpNetworkChannel::write_to_network()
{
    //first write everything that has timedout
    for (std::vector<std::pair<unsigned,unsigned> >::iterator it = timed_out_packets.begin(); it != timed_out_packets.end(); it++)
    {
        std::string* ser_packet = outgoing->get_message("localhost",it->first)->get_serialized_packet(it->second);
        sender->sync_write(*ser_packet,0);
    }
    NetworkPacket* pack = outgoing_queue.front();
		std::string* buffer_data = 0;
    while(outgoing_queue.size() > 0 && pack != 0)
    {
			buffer_data = new std::string();
			cout << " outgoing queue size " << outgoing_queue.size() << " " << pack->header().type() << " tm: " << pack->header().timeout() << endl;
			pack->SerializeToString(buffer_data);
			unsigned send_bytes  =  sender->sync_write (*buffer_data,0);
			if (buffer_data->size() != send_bytes)
				timed_out_packets.push_back(std::make_pair<unsigned, unsigned>(pack->header().message_num(),pack->header().packet_num()));
			if(buffer_data != 0)
			{
				delete buffer_data;
				
			}
				outgoing_queue.pop();
			//break;
				pack = outgoing_queue.front();
      }
}

