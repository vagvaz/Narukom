#include <iostream>

#include "pinger.h"
#include "../narukom.h"
#include "../narukom_common.h"
using namespace std;
int main(int argc, char **argv) {
		Narukom n;
		MessageQueue* mq = n.get_message_queue();
		Pinger pinger;
		cout << "add publisher pinger " << endl;
		mq->add_publisher(&pinger);
		cout << "add_subscriber " << endl;
		mq->add_subscriber(&pinger);
		mq->subscribe("motion",&pinger,ON_TOPIC);
		cout << "start thread " << endl;
		pinger.StartThread();
		cout << " service " << endl;
		pinger.start_service();
		cout << "joining " << endl;
		pinger.JoinThread();
		cout << "Joined and exit ... " << endl;
}
