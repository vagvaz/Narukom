//
// connection_manager.cpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "connection_manager.hpp"
#include <algorithm>
#include <boost/bind.hpp>

void connection_manager::start(connection_ptr c)
{
  connections_.insert(c);
  c->start();
}

void connection_manager::remove(connection_ptr c)
{
	connection_struct::iterator it = connections_.find(c->get_remote_endpoint());
	if(it != connections_.end())
		connections_.erase(it);
	
}
connection_ptr connection_manager::find(boost::asio::ip::tcp::endpoint remote_endpoint)
{
	connection_ptr result;
	result.reset();
	connection_struct::iterator it = connections_.find(remote_endpoint);
	if(it != connections_.end())
	{
		result = (*it);
	}
	return result;
}

void connection_manager::restart(connection_ptr c)
{
	c->start();
}

void connection_manager::stop(connection_ptr c)
{
//   connections_.erase(c);
  c->stop();
}

void connection_manager::stop_all()
{
  std::for_each(connections_.begin(), connections_.end(),
      boost::bind(&connection::stop, _1));
  connections_.clear();
}

connection_ptr connection_manager::find(connection_ptr p)
{
	return find(p->get_remote_endpoint());
}
connection_struct& connection_manager::get_connections()
{
	return connections_;
}

void connection_manager::add_broken(connection_ptr con ,const boost::system::error_code& e)
{
	broken_connections_.push_back(std::make_pair<connection_ptr,boost::system::error_code>(con,e));
}
std::list< std::pair< connection_ptr, boost::system::error_code > >& connection_manager::get_broken()
{
	return broken_connections_;
}

std::list< connection_ptr >* connection_manager::get_connection_list()
{
	std::list< connection_ptr >* result =  new std::list<connection_ptr>;
	for(connection_struct::iterator it = connections_.begin(); it != connections_.end();it++ )
	{
		result->push_back((*it));
	}
	return result;
}

