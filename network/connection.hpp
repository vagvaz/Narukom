//
// connection.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_CONNECTION_HPP
#define HTTP_CONNECTION_HPP

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "network_message.h"
#include <queue>
#include "../system/Mutex.h"

class connection_manager;

/// Represents a single connection from a client.
class connection
  : public boost::enable_shared_from_this<connection>,
    private boost::noncopyable
{
public:
  /// Construct a connection with the given io_service.
  explicit connection(boost::asio::io_service& io_service,
      connection_manager& manager);

  /// Get the socket associated with the connection.
  boost::asio::ip::tcp::socket& socket();

  /// Start the first asynchronous operation for the connection.
  void start(boost::asio::ip::tcp::endpoint& peer);
	void start();

  /// Stop all asynchronous operations associated with the connection.
  void stop();
	void add_message(NetworkMessage* msg);
	unsigned async_write_next();
	std::string* read_next();
	void start_async_read();
	
	
	boost::asio::ip::tcp::endpoint get_local_endpoint() ;
	boost::asio::ip::tcp::endpoint get_remote_endpoint() ;
private:
	///Handle connect completion
	void handle_connect( const boost::system::error_code& e);
	///Handle timeout
	void handle_timeout(const boost::system::error_code& e);
	/// Handle completion of a read operation.

  void handle_read(const boost::system::error_code& e,
      std::size_t bytes_transferred);

  /// Handle completion of a write operation.
  void handle_write(const boost::system::error_code& e,NetworkPacket* pck);

  /// Socket for the connection.
	Mutex mx;
  boost::asio::ip::tcp::socket socket_;
	
  /// The manager for this connection.
  connection_manager& connection_manager_;

  /// Buffer for incoming data.
  std::string* buffer_;

	std::queue<NetworkMessage*> outgoing_messages;
	std::queue<std::string*> incoming_data;
	
};

typedef boost::shared_ptr<connection> connection_ptr;


#endif // HTTP_CONNECTION_HPP