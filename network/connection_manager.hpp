//
// connection_manager.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_CONNECTION_MANAGER_HPP
#define HTTP_CONNECTION_MANAGER_HPP

#include <set>
#include <boost/noncopyable.hpp>
#include "connection.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/composite_key.hpp>

using namespace boost::multi_index;

typedef multi_index_container<
	connection_ptr,
	indexed_by<
		ordered_unique<BOOST_MULTI_INDEX_MEM_FUN(connection,boost::asio::ip::tcp::endpoint,get_remote_endpoint)
		>
	>
> connection_struct;


/// Manages open connections so that they may be cleanly stopped when the server
/// needs to shut down.
class connection_manager
  : private boost::noncopyable
{
public:
  /// Add the specified connection to the manager and start it.
  void start(connection_ptr c);

  /// Stop the specified connection.
  void stop(connection_ptr c);
	connection_ptr find(boost::asio::ip::tcp::endpoint remote_endpoint);
	connection_ptr find(connection_ptr p);
	void remove(connection_ptr c);
	void restart(connection_ptr c);
  /// Stop all connections.
  void stop_all();
	connection_struct& get_connections();
	std::list<connection_ptr>* get_connection_list();
	void add_broken(connection_ptr,const boost::system::error_code& e);
	std::list<std::pair<connection_ptr,boost::system::error_code> >& get_broken();

private:
  /// The managed connections.
  connection_struct connections_;
	std::list<std::pair<connection_ptr,boost::system::error_code> > broken_connections_;
};

#endif // HTTP_CONNECTION_MANAGER_HPP