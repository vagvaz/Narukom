#include <iostream>

#include "ponger.h"
#include "../narukom.h"
#include "../narukom_common.h"
using namespace std;
int main(int argc, char **argv) {
		Narukom n;
		MessageQueue* mq = n.get_message_queue();
		Ponger ponger("Ponger");
		cout << "add publisher ponger " << endl;
		mq->add_publisher(&ponger);
		cout << "add_subscriber " << endl;
		mq->add_subscriber(&ponger);
		mq->subscribe("motion",&ponger,ON_TOPIC);
		cout << "start thread " << endl;
		ponger.StartThread();

		cout << "joining " << endl;
		ponger.JoinThread();
		cout << "Joined and exit ... " << endl;
}