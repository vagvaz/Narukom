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

#include "pinger.h"
#include <cstdlib>
#include <ctime>
void Pinger::printGame(int other,int myside)
{
    cout <<       "         Pinger         " << endl;
    cout << "--------------------" << endl;
    if ( other)
    {
        if (other != myside)
        {
            cout << "|O       []          \n" <<    "|        []          " <<endl;
            cout << "         []         |\n" <<    "         []         |"  << endl;
        }
        else
        {
            cout << "|        []        |\n" <<    "|        []          |" << endl;
            cout << "O        []         \n" <<    "         []           "  << endl;
        }
    }
    else
    {
        if (other == myside)
        {
            cout << "|        []        |\n" <<  "|        []        |" <<endl;
            cout << "O        []        \n" <<   "         []         "  << endl;
        }
        else
        {
            cout << "         []         \n" <<    "         []          " << endl;
            cout << "|O       []        |\n" <<    "|        []        |"  << endl;
        }
        cout << "--------------------" << endl;
    }

}
PingMessage* Pinger::play(PongMessage* msg)
{
    PingMessage* result = new  PingMessage;
    srand(time(0));
    int myside = rand() % 2;
    if (myside ==msg->side())
        result->set_successful(true);
    else
        result->set_successful(false);
		printGame(msg->side(),myside);
    return result;
}

void Pinger::start_service()
{
    PingMessage* result = new  PingMessage;
    srand(time(0));

    publish(result);
    //return result;
}
int Pinger::Execute()
{
    if (Subscriber::getBuffer() != 0)
        if (Subscriber::getBuffer()->size() > 0)
            process_messages();

    return 0;
}

void Pinger::process_messages()
{

    MessageBuffer* sub_buf = Subscriber::getBuffer();
    //cout << "Pinger " << endl;
    if (sub_buf == 0)
        cout << "None Unprocessed Buffers" << endl;
    Tuple*  cur = sub_buf->remove_head();


    while (cur != 0 )
    {
        if (cur->get_msg_data()->GetTypeName() ==  "PongMessage"  )
        {
            PongMessage* msg = (PongMessage*)cur->get_msg_data();
            PingMessage* ping_msg = play(msg);
            //cout << "Before publishe Successful  " << ping_msg->successful() <<" " <<  ping_msg->side() << "address: " << ping_msg << endl;
            publish(ping_msg);
        }
        if(cur->get_msg_data()->GetTypeName() == "RawBytes")
				{
						PongMessage* msg = blk->extract_result_from_tuple<PongMessage>(*cur);
						PingMessage* ping_msg = play(msg);
            publish(ping_msg);
				}
        else
        {
            cout << "Unknown Message Type: " << cur->get_msg_data()->GetTypeName() << endl;
        }
        delete cur;
        cur = sub_buf->remove_head();

    }
} 

void Pinger::publish ( google::protobuf::Message* msg )
{
    static int delivered = 0;
    Publisher::publish ( msg,"motion");
    if (++delivered == 40)
    {
        cout << "Stop Pinger" <<endl;
        StopThread();
				cout << "After stop Thread" << endl;
    }
	delete msg;
}

