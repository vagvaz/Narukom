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

#include "ponger.h"
#include "pingpong.pb.h"
#include <cstdlib>
#include <time.h>
using std::cerr;
void Ponger::printGame(int other,int myside)
{
    cout <<      "         Ponger         " << endl;
    cout << "--------------------" << endl;
// 	"--------------------"
// 	"|        []        O\n"
// 	"|        []         \n"
// 	"         []        |\n"
// 	"         []        |"
// 	"|        []       O|"
// 	"|        []        |\n"
// 	"         []         \n"
// 	"         []         "
    if ( other)
    {
        if (other == myside)
        {
            cout << "|        []        O\n" <<    "|        []         \n" ;//<<endl;
            cout << "         []        |\n" <<    "         []         |"  << endl;
        }
        else
        {
            cout << "|        []       O|\n" <<    "|        []        |\n" ;//<< endl;
            cout << "         []         \n" <<    "         []          "  << endl;
        }
    }
    else
    {
        if (other == myside)
        {
            cout << "         []        |\n" <<    "         []        |\n" ;//<<endl;
            cout << "|        []        O\n" <<    "|        []         "  << endl;
        }
        else
        {
            cout << "         []         \n" <<    "         []          \n";// << endl;
            cout << "|        []       O|\n" <<    "|        []        |"  << endl;
        }
        cout << "--------------------" << endl;
    }

}
PongMessage* Ponger::play(PingMessage* msg)
{
    PongMessage* result = new  PongMessage;
    srand(time(0));

    int myside = rand() % 2;
 
    if (myside ==msg->side())
        result->set_successful(true);
    else
        result->set_successful(false);
    printGame(msg->side(),myside);
    return result;
}
int Ponger::Execute()
{
    if (Subscriber::getBuffer()!=0)
        if (Subscriber::getBuffer() > 0)
            process_messages();
    return 0;
}

void Ponger::process_messages()
{

	
    MessageBuffer* sub_buf = Subscriber::getBuffer();

    if (sub_buf == 0)
        cout << "None Unprocessed Buffers" << endl;
    Tuple*  cur = sub_buf->remove_head();


    while (cur != 0 )
    {
        if (cur->get_msg_data()->GetTypeName() ==  "PingMessage" || cur->get_msg_data()->GetTypeName() == "RawBytes"  )
        {
					if(cur->get_msg_data()->GetTypeName() == "PingMessage")
					{
            PingMessage* msg = (PingMessage*)cur->get_msg_data();
            PongMessage* pong_msg = play(msg);
            publish(pong_msg);
					}
					else
					{
						PingMessage* msg = blk->extract_result_from_tuple<PingMessage>(*cur);
						PongMessage* pong_msg = play(msg);
            publish(pong_msg);
					}
        }
        else
        {
            cout << "Unknown Message Type: " << cur->get_msg_data()->GetTypeName() << endl;
        }
        delete cur;
        cur = sub_buf->remove_head();

    }

}

void Ponger::publish ( google::protobuf::Message* msg )
{
    static int delivered = 0;
    Publisher::publish ( msg,"motion" );
    if (++delivered== 40)
    {
        cout << "Stop Ponger" <<endl;
        StopThread();
				cout << "After stop Thread" << endl;
    }
    delete msg;
}

