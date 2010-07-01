#include "narukom.h"
//TOBE REMOVED
#include "pub_sub/filters/content_filter.h"
#include "pub_sub/tuple.h"
///////
Narukom::Narukom()
{
  mq = new MessageQueue("/home/vagvaz/topic_tree.xml");
  //   udp_multi = new UdpMulticastChannel();
  // mq->add_subscriber(udp_multi);
  //   mq->subscribe("global",udp_multi,2);
  mq->StartThread();
	multicast_channel = new UdpNetworkChannel("CatalogComChannel","224.0.0.1",9000,500);
	mq->add_publisher(multicast_channel);
	mq->add_subscriber(multicast_channel);
	mq->subscribe("motion",multicast_channel,ON_TOPIC);
	multicast_channel->StartThread();

}
MessageQueue* Narukom::get_message_queue()
{
  if(mq == 0)
    mq = new MessageQueue();
  return mq;
  
  
}