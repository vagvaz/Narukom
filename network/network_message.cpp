#include "network_message.h"
#include "network_common.h"
#include <cmath>
#include <iostream>
unsigned int packet_num::get_packet_num()
{
    if ( packet != 0 )
        return packet->header().packet_num();
    return -1;
}
packet_num::packet_num ( NetworkPacket* pckt )
{
    packet = pckt;
}
packet_num::~packet_num()
{
    delete packet;
}

NetworkMessage::NetworkMessage ( unsigned message_num,Tuple* tuple )
{
    is_header_initiliazed = false;
    msg_header.set_host ( tuple->get_host() );
    msg_header.set_message_num ( message_num );
    message_number = message_num;
    msg_header.set_publisher ( tuple->get_publisher() );
    msg_header.set_topic ( tuple->get_topic() );
    msg_header.set_type ( tuple->get_type() );
    msg_header.set_timestamp ( boost::posix_time::to_iso_string ( tuple->get_timestamp() ) );
    msg_header.set_timeout ( tuple->get_envelope().timeout() );
		is_header_initiliazed = true;
    added_packets = 0;
		
    create_packets_from_message ( msg_header,tuple->get_msg_data() );
}
NetworkMessage::NetworkMessage (  NetworkPacket* packet )
{
	  is_header_initiliazed = false;
		msg_header.CopyFrom(packet->header());
		is_header_initiliazed = true;
    message_number = packet->header().message_num();
    number_of_packets = packet->header().number_of_packets();
    add_packet ( packet );
}
NetworkMessage::NetworkMessage ( const NetworkMessage& other_msg )
{
	  is_header_initiliazed = false;
		msg_header.CopyFrom(*(other_msg.get_network_header()));
		is_header_initiliazed = true;
    message_number = other_msg.get_message_number();
    number_of_packets = other_msg.get_number_of_packets();
    std::list<NetworkPacket*>* other_packets = other_msg.get_added_packets();
    for ( std::list<NetworkPacket*>::iterator it; it != other_packets->end(); it++ )
    {
        this->add_packet ( ( *it ) );
    }

}
NetworkMessage::~NetworkMessage()
{
	packets.clear();
}

unsigned int NetworkMessage::add_packet ( NetworkPacket* pckt )
{
    const packet_sequence::nth_index<0>::type& packet_index = packets.get<0>();
    packet_sequence::iterator it = packet_index.find ( pckt->header().packet_num() );
    packet_num* new_pckt = new packet_num(pckt);
    if ( it != packet_index.end() )
        ;
    else
    {
        packets.insert( new_pckt  );
        added_packets++;
    }
    return added_packets;

}
unsigned int NetworkMessage::get_number_added_packets() const
{
    return added_packets;
}
unsigned int NetworkMessage::get_message_number() const
{
    return message_number;
}
std::list< unsigned >* NetworkMessage::get_missing_packets() const
{
    const packet_sequence::nth_index<0>::type& packet_index = packets.get<0>();
    std::list<unsigned>* missing_packets = new std::list<unsigned>();
    unsigned current = 0;
    packet_sequence::iterator it = packet_index.begin();
    for ( current = 0; current < number_of_packets; current++ )
    {
        if ( current == ( *it )->get_packet_num() )
        {
            it++;
            continue;
        }
        else
        {
            missing_packets->push_back ( current );
        }
    }
    return missing_packets;
}
const NetworkHeader* NetworkMessage::get_network_header() const
{
	if(is_header_initiliazed)
		return &msg_header;
	else
		return 0;
}
unsigned int NetworkMessage::get_number_of_packets() const
{
    return added_packets;
}
std::list< NetworkPacket* >* NetworkMessage::get_added_packets() const
{
    const packet_sequence::nth_index<0>::type& packet_index = packets.get<0>();
    std::list<NetworkPacket*>* packet_list = new std::list<NetworkPacket*>();

    for ( packet_sequence::iterator it = packet_index.begin(); it != packet_index.end(); it++ )
        packet_list->push_back ( ( *it )->packet );
    return packet_list;
}

std::string* NetworkMessage::get_serialized_msg() const
{
    std::string* result = new std::string();
    const packet_sequence::nth_index<0>::type& packet_index = packets.get<0>();

    for ( packet_sequence::iterator it = packet_index.begin(); it != packet_index.end(); it++ )
    {
        NetworkPacket* tmp_packet = ( *it )->packet;
        result->append ( tmp_packet->byte_buffer().byte_stream() );
    }
    return result;
}
std::string* NetworkMessage::get_serialized_packet ( unsigned int packet_num ) const
{
    std::string* result = 0;
    const packet_sequence::nth_index<0>::type& packet_index = packets.get<0>();
    packet_sequence::iterator it = packet_index.find ( packet_num );
    if ( it != packet_index.end() )
    {
        std::cout << "SerializeAsString " << std::endl;
        result =  new std::string();
        NetworkPacket* tmp_packet = ( *it )->packet;
        tmp_packet->SerializeToString ( result );
    }
    return result;

}
unsigned int NetworkMessage::remove_packet ( unsigned int packet_num )
{
    const packet_sequence::nth_index<0>::type& packet_index = packets.get<0>();
    packet_sequence::iterator it = packet_index.find ( packet_num );
    if ( it != packet_index.end() )
    {
        packets.erase(it);
        added_packets--;
        return added_packets;
    }
    return added_packets;
}
unsigned int NetworkMessage::remove_packet ( const NetworkPacket& pckt )
{
    return remove_packet(pckt.header().packet_num());
}
void NetworkMessage::set_message_number ( unsigned int val )
{
    message_number = val;
}
void NetworkMessage::set_number_of_packets ( unsigned int val )
{
    number_of_packets = val;
}

void NetworkMessage::create_packets_from_message ( NetworkHeader& header, google::protobuf::Message* msg )
{
    unsigned int message_size = msg->ByteSize();
    unsigned int header_size = header.ByteSize();
    unsigned int max_header_size = header_size + 9;

//   google::protobuf::uint8* buffer = new google::protobuf::uint8[message_size];
    std::string* buffer = new std::string();
    msg->SerializeToString ( buffer );
    NetworkPacket* tmp_packet = new NetworkPacket();
    std::cout << "empty net packet " << tmp_packet->ByteSize() << " " <<  message_size << std::endl ;
    if ( ( int ) ( DEFAULT_PACKET_SIZE - header_size - message_size ) > 0 )
    {
        number_of_packets = 1;
        header.set_number_of_packets ( 1 );
        header.set_packet_num ( 0 );
        tmp_packet->mutable_header()->CopyFrom ( header );
        tmp_packet->mutable_byte_buffer()->set_byte_stream(*buffer);
        tmp_packet->mutable_byte_buffer()->set_size(buffer->size());
        add_packet ( tmp_packet );
				delete buffer;
        std::cout<< "type: " << msg->GetTypeName() << " msg size: " << msg->ByteSize() << " nheader sz: " << header.ByteSize() << " " << tmp_packet->ByteSize()<< std::endl;
    }
    else
    {
        delete tmp_packet; //unneeded instance as network packets are created inside for
        unsigned total_packets = ( unsigned ) ceil ( message_size / ( DEFAULT_PACKET_SIZE - max_header_size ) );
        number_of_packets = total_packets;
        header.set_number_of_packets ( total_packets );

        std::cout<< "typDDDDe: " << msg->GetTypeName() << " message_size: " << message_size << "DEFAULT_PACKET_SIZE: " << DEFAULT_PACKET_SIZE << "max header: " << max_header_size << "num_packets: " << total_packets << std::endl;
        for ( unsigned i = 0; i < number_of_packets; i++ )
        {
            unsigned packet_size = DEFAULT_PACKET_SIZE - max_header_size;
            tmp_packet = new NetworkPacket;
            header.set_packet_num ( i );
            tmp_packet->mutable_header()->CopyFrom ( header );
            unsigned pos = 0;
            unsigned substr_size = 0;
            if ( i < number_of_packets-1 )
            {
							pos = i*packet_size;
							substr_size = packet_size;
              tmp_packet->mutable_byte_buffer()->mutable_byte_stream()->append(buffer->substr(pos,substr_size));
            }
            else
            {
                pos = i*packet_size;
								substr_size = message_size-(i*packet_size);
                tmp_packet->mutable_byte_buffer()->mutable_byte_stream()->append(buffer->substr(pos,substr_size));
            }
            add_packet ( tmp_packet );
						delete buffer;
        }
    }
}

