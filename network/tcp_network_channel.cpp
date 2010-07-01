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

#include "tcp_network_channel.h"
#include "network_common.h"
#include <iostream>
#include <limits>
using std::cout;
using std::endl;




TcpNetworkChannel::TcpNetworkChannel ( const std::string& channel_name, const std::string& ip_address, short int port ,
                                       unsigned period) : UnicastChannel ( channel_name ),io_service(new boost::asio::io_service),acceptor_(*io_service)

{
    //Network Initialization

    mx = new Mutex;
    io_service = new boost::asio::io_service();
//		boost::asio::ip::address addr(ip_address);
//  address ( boost::asio::ip::address::from_string(ip_address),port)
    endpoint_.address ( boost::asio::ip::address::from_string ( ip_address ) );
    endpoint_.port(port);
    acceptor_.open(endpoint_.protocol());
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint_);
    acceptor_.listen();
    acceptor_.async_accept(new_connection_->socket(),
                           boost::bind(&TcpNetworkChannel::handle_accept, this,
                                       boost::asio::placeholders::error));

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


TcpNetworkChannel::~TcpNetworkChannel()
{

    outgoing->clear();
    incoming->clear();
    delete outgoing;
    delete incoming;
    delete socket;
//     /*delete receiver;
//     delete sender;*/
		acceptor_.close();
    if ( io_service != 0 )
        delete io_service;

    if (mx !=0)
    {
        mx->unlock();
        delete mx;
    }

}

void TcpNetworkChannel::async_read_from_network()
{
	std::list<connection_ptr>* connection_list = connection_manager_.get_connection_list();
	for(std::list<connection_ptr>::iterator it = connection_list->begin(); it != connection_list->end();it++)
	{
		(*it)->start_async_read();
	}
	delete connection_list;
}

void TcpNetworkChannel::async_write_to_network()
{
std::list<connection_ptr>* connection_list = connection_manager_.get_connection_list();
	
	unsigned int remaining = 0;
	std::list<connection_ptr>::iterator it = connection_list->begin();
	while(connection_list->size() > 0)
	{
		remaining = (*it)->async_write_next();
		if(remaining == 0 )
		{
			connection_list->erase(it);
			
			continue;
		}
		it++;
	}
	delete connection_list;
}

void TcpNetworkChannel::process_command(const ChannelCommand& cmd)
{
    //Channel::process_command(cmd);
		if(cmd.command_type() == "connect")
		{
			cout << "Connecting" << endl;
		}
		if(cmd.command_type() == "disconnect")
		{
			cout << "disconnecting" << endl;
		}
}

void TcpNetworkChannel::cleanup()
{
// 	cout << "Cleanup " << endl;
    outgoing->cleanup(current_time);

}

void TcpNetworkChannel::handle_accept(const boost::system::error_code& e)
{
  if (!e)
  {
    connection_manager_.start(new_connection_);
    new_connection_.reset(new connection(*io_service,
          connection_manager_/*, request_handler_*/));
    acceptor_.async_accept(new_connection_->socket(),
        boost::bind(&TcpNetworkChannel::handle_accept, this,
          boost::asio::placeholders::error));
  }
}


void TcpNetworkChannel::nack_cleanup()
{
	cout << "Nack Cleanup not needed in tcp " << endl;
// // 	cout << "NCleanup " << endl;
//     incoming->cleanup(current_time);
//     const std::list<network_msg*>* timeout_msg =  incoming->get_timeout_messages();
//     for (std::list<network_msg*>::const_iterator it = timeout_msg->begin(); it != timeout_msg->end();it++)
//     {
//         send_nack((*it)->msg);
//     }
}
int TcpNetworkChannel::Execute()
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
    async_write_to_network();
    io_service->run();
		process_incoming_packets();
		handle_broken_connections();
    return 0;
}

void TcpNetworkChannel::handle_broken_connections()
{
	std::list< std::pair<connection_ptr, boost::system::error_code > >& broken_list = connection_manager_.get_broken();
	for(std::list< std::pair<connection_ptr, boost::system::error_code > >::iterator it = broken_list.begin(); it != broken_list.end();it++)
	{
		cout << "Problem with con: " << it->first->get_local_endpoint() << " --> " <<  it->first->get_remote_endpoint() << endl;
		cout << "Error Message: " << it->second.message() << endl;
	}
	
}

void TcpNetworkChannel::process_incoming_packets()
{
	std::list<connection_ptr>* connection_list = connection_manager_.get_connection_list();
	std::string* raw_packet = 0;
	NetworkPacket* packet = 0;
	std::list<connection_ptr>::iterator it = connection_list->begin();
	while(connection_list->size() > 0)
	{
		raw_packet = (*it)->read_next();
		if(raw_packet == 0 )
		{
			connection_list->erase(it);
			continue;
		}
		
		packet = new NetworkPacket;
    if(!packet->ParseFromString(*raw_packet))
		{
			cout << "Unserialization failed" << endl;
			delete raw_packet;
			raw_packet = 0;
			it++;
			continue;
		}
		else
		{
			cout << " serialization was true " << endl;
		}
		cout << "receive data of size: " << raw_packet->size() << " from " << (*it)->get_remote_endpoint() << " where " << packet->byte_buffer().size() <<  endl;
    if (hosts.right.find((*it)->get_remote_endpoint()) == hosts.right.end())
        hosts.insert(hosts_bmt::value_type(packet->header().host(),(*it)->get_remote_endpoint() )  );
//     if (packet->header().type() == "NackMessage")
//     {
//         NackMessage nack;
//         nack.ParseFromString(packet->byte_buffer().byte_stream());
//         if (nack.receiver() == get_hostname())
// 				{
// 					respond_nack(nack);
// 					return;
// 				}
//     }
    NetworkMessage *msg = incoming->add_packet(packet);
    check_msg_and_deliver(msg);
		delete raw_packet;
		raw_packet = 0;
		it++;
	}
	delete connection_list;
}

void TcpNetworkChannel::notfiy_send ( unsigned int num_of_message, unsigned int num_of_piece,const  boost::asio::ip::basic_endpoint< boost::asio::ip::tcp >& rec )
{

    cout << "Async send of msg: " << num_of_message << " packet: " << num_of_piece << " to " <<  rec << endl;
}

void TcpNetworkChannel::notify_receive ( char* buffer_data, unsigned int sz, const  boost::asio::ip::basic_endpoint< boost::asio::ip::tcp >& send )
{
    NetworkPacket* packet = new NetworkPacket;

    if (!packet->ParseFromArray(buffer_data,sz))
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
        hosts.insert(hosts_bmt::value_type(packet->header().host(),send )  );
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


void TcpNetworkChannel::notify_receive_timeout()
{

//     cout << "receive timed out " << endl;
    io_service->stop();
}
void TcpNetworkChannel::notify_send_timeout ( unsigned int num_of_message, unsigned int num_of_piece, const  boost::asio::ip::basic_endpoint< boost::asio::ip::tcp >& )
{
//     timed_out_packets.push_back(std::make_pair<unsigned,unsigned>(num_of_message,num_of_piece));
}

std::list< boost::asio::ip::tcp::endpoint >* TcpNetworkChannel::get_endpoints_from_string(const std::string& destination)
{
	std::list< boost::asio::ip::tcp::endpoint >* result_list = new std::list<boost::asio::ip::tcp::endpoint>;
	if(destination == "")
	{
		for(hosts_bmt::right_map::iterator it = hosts.right.begin(); it != hosts.right.end();it++ )
		{
			result_list->push_back(it->first);
		}
	}
	else
	{
		hosts_bmt::left_map::iterator it =  hosts.left.find(destination);
		if(it != hosts.left.end())
			result_list->push_back(it->second);
	}
	return result_list;
}

void TcpNetworkChannel::process_messages()
{
    MessageBuffer* buf = Subscriber::getBuffer();
    if (buf != 0 && buf->size() > 0)
    {
        Tuple* cur = buf->remove_head();
				std::string destination = "";
        while (cur != 0)
        {
            cur->set_host(get_hostname());
            cout << " Tcpnewtorkchannel: " << "msg sz: " << cur->get_msg_data()->ByteSize() << endl;
            NetworkMessage* net_msg = new NetworkMessage( get_next_message_number(),cur);
            outgoing->add_message(net_msg);
						destination = cur->get_envelope().destination();
            delete cur;
            cur = buf->remove_head();
						std::list<boost::asio::ip::tcp::endpoint>* receivers = get_endpoints_from_string(destination);
						if(receivers->size() > 0)
						{
							for(std::list<boost::asio::ip::tcp::endpoint>::iterator iter = receivers->begin(); iter != receivers->end();iter++)
							{
								connection_manager_.find((*iter))->add_message(net_msg);
							}
							
							
						}
						else
						{

						}
						delete receivers;
// 						for(std::list
//             std::list<NetworkPacket*>* packet_list = net_msg->get_added_packets();
// 						
//             for (std::list<NetworkPacket*>::iterator it = packet_list->begin() ; it !=packet_list->end(); it++)
//             {
// 
//                 NetworkPacket* outgoing_packet =  (*it);
// //                 cout << "TcpNetworkChannel: pack sz " << outgoing_packet->ByteSize() << " heeader " << outgoing_packet->header().ByteSize() << endl;
//                 
//                 cout << "Adding to outgoing packet " << outgoing_queue.size() << " "<<outgoing_packet->header().message_num() << " " << outgoing_packet->header().packet_num()  <<endl;
//             }
        }
    }
}
bool TcpNetworkChannel::check_msg_and_deliver(NetworkMessage* msg)
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


void TcpNetworkChannel::read_from_network()
{
//     char* buffer = new char[DEFAULT_PACKET_SIZE];
// //     unsigned read_bytes = receiver->sync_receive(buffer,DEFAULT_PACKET_SIZE,address,sender_timeout);
//     unsigned read_bytes;
//     if (read_bytes > 0 )
//     {
//         NetworkPacket* new_packet = new  NetworkPacket;
//         new_packet->ParseFromArray(buffer,read_bytes);
//         if (new_packet->header().type() == "NackMessage")
//         {
//             NackMessage nack;
//             nack.ParseFromString(new_packet->byte_buffer().byte_stream());
//             if (nack.receiver() == get_hostname())
//             {
//                 respond_nack(nack);
//                 return;
//             }
//         }
//         NetworkMessage* msg  = incoming->add_packet(new_packet);
//         check_msg_and_deliver(msg);
//     }
//     else
//     {
//         cout << "No Data received from: " << endpoint_.address().to_string() << " on port " << endpoint_.port() << endl;
//     }
}

void TcpNetworkChannel::receive ( std::string* host, Tuple* data )
{
    cout << "uneeded receive function " << endl;
}
void TcpNetworkChannel::respond_nack(const NackMessage& nack)
{

}
void TcpNetworkChannel::send_nack(NetworkMessage* msg)
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
    nack_header.set_publisher("TcpNetworkChannel");
    nack_header.set_timeout(sender_timeout*3);
    nack_header.set_topic("system");
    nack_header.set_timestamp("9999-Dec-31");
    nack_header.set_type(nack.GetTypeName());
    std::string data = "";
    nack_header.SerializeToString(&data);
    nack.SerializeToString(&data);
//     sender->sync_write(data,&address);
    delete missing;
}

void TcpNetworkChannel::send ( const Tuple& t )
{
    cout << "unneeded function from tcp_network_channel send( Tuple )" << endl;
}

void TcpNetworkChannel::start()
{
    Channel::start();

}
void TcpNetworkChannel::stop()
{
    Channel::stop();
}
void TcpNetworkChannel::write_to_network()
{
    //first write everything that has timedout
    for (std::vector<boost::tuple<boost::asio::ip::tcp::endpoint,unsigned,unsigned>* >::iterator it = timed_out_packets.begin(); it != timed_out_packets.end(); it++)
    {
//         std::string* ser_packet = outgoing->get_message("localhost",it->first)->get_serialized_packet(it->second);
//         sender->sync_write(*ser_packet,0);
    }
    NetworkPacket* pack = outgoing_queue.front();
    std::string* buffer_data = 0;
    while (outgoing_queue.size() > 0 && pack != 0)
    {
        buffer_data = new std::string();
        cout << " outgoing queue size " << outgoing_queue.size() << " " << pack->header().type() << " tm: " << pack->header().timeout() << endl;
        pack->SerializeToString(buffer_data);
// 			unsigned send_bytes  =  sender->sync_write (*buffer_data,0);
//         unsigned send_bytes=0; //added_compile
//         if (buffer_data->size() != send_bytes)
//             timed_out_packets.push_back(std::make_pair<unsigned, unsigned>(pack->header().message_num(),pack->header().packet_num()));
        if (buffer_data != 0)
        {
            delete buffer_data;

        }
        outgoing_queue.pop();
        //break;
        pack = outgoing_queue.front();
    }
}

