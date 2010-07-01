#ifndef TCP_NETWORK_CHANNEL_H
#define TCP_NETWORK_CHANNEL_H
#include "unicast_channel.h"
#include "../interfaces/network_channel.h"
#include "../narukom_messages/Nack.pb.h"
#include "udp_network_channel.h"
#include "connection.hpp"
#include "connection_manager.hpp"
#include <boost/bimap.hpp>
#include "network_message.h"
#include "network_message_buffer.h"
#include "../system/Mutex.h"
#include <queue>

typedef boost::bimap<std::string, boost::asio::ip::tcp::endpoint> hosts_bmt;

class TcpNetworkChannel : public UnicastChannel, NetworkChannelSender<boost::asio::ip::tcp>, NetworkChannelReceiver<boost::asio::ip::tcp>
{
public:
  TcpNetworkChannel( const std::string& channel_name, const std::string& ip_address, short int port, unsigned clean_period = 500 );
  virtual ~TcpNetworkChannel();
  virtual int Execute();
  virtual void receive( std::string* host, Tuple* data );
  virtual void send( const Tuple& t );
  virtual void start();
  virtual void stop();
  virtual void process_messages();
  void process_incoming_packets();
  void write_to_network();
  void read_from_network();
  void async_read_from_network();
  void async_write_to_network();
  void respond_nack( const NackMessage& nack );
  void send_nack( NetworkMessage* msg );
  void cleanup();
  void nack_cleanup();
  virtual void notfiy_send( unsigned int num_of_message, unsigned int num_of_piece, const  boost::asio::ip::basic_endpoint< boost::asio::ip::tcp >& );
  virtual void notify_receive( char* buffer_data, unsigned int sz, const boost::asio::ip::basic_endpoint< boost::asio::ip::tcp >& );
  virtual void notify_receive_timeout();
  virtual void notify_send_timeout( unsigned int num_of_message, unsigned int num_of_piece, const boost::asio::ip::basic_endpoint< boost::asio::ip::tcp >& );
  virtual void process_command( const ChannelCommand& cmd );
private:
  void handle_broken_connections();
  std::list<boost::asio::ip::tcp::endpoint>* get_endpoints_from_string( const std::string& destination );
  bool check_msg_and_deliver( NetworkMessage* msg );
  void handle_accept( const boost::system::error_code& e );
  Mutex* mx;
  boost::posix_time::time_duration cleanup_period;
  boost::posix_time::ptime now;
  boost::asio::io_service* io_service;
  boost::asio::ip::tcp::socket* socket;
  boost::asio::ip::tcp::endpoint endpoint_;
  NetworkMessageBuffer* incoming;
  NetworkMessageBuffer* outgoing;
  network_msg_finger_print index_incoming;

  boost::asio::ip::tcp::acceptor acceptor_;
  connection_manager connection_manager_;
  connection_ptr new_connection_;

  std::vector< boost::tuple<boost::asio::ip::tcp::endpoint, unsigned, unsigned>* > timed_out_packets;
  hosts_bmt hosts;
  boost::posix_time::ptime current_time;
  std::queue<NetworkPacket*> outgoing_queue;
  unsigned receiver_timeout;
  unsigned sender_timeout;
  unsigned outgoing_timeout;
  unsigned incoming_timeout;
  network_buffer_index::iterator next_msg;
};

#endif // UDP_NETWORK_CHANNEL_H
