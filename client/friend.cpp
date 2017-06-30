//
// Created by prwang on 6/30/17.
//

#include "friend.h"
#include "client_program.h"

void Friend_::sendmsg()
{
    ++try_time;
    guard->stop();
    ClientProgram *cp = qobject_cast<ClientProgram *>(parent());
    cp->with_client->writeDatagram(
            compose_obj(opcd::MSG_REALMSG, cp->me.session, pendingmsg),
            QHostAddress(ipaddr), port);
    guard->start(timeout);
}

void Friend_::send_timeout()
{
    ClientProgram *cp = qobject_cast<ClientProgram *>(parent());
    if (try_time < 2)
    {


    } else cp->ui_send_error();
}

void Friend_::ok()
{
    guard->stop();
    ClientProgram *cp = qobject_cast<ClientProgram *>(parent());
    addmsg("****", pendingmsg, "\n");
    cp->ui_setupFriend();
    pendingmsg.resize(0);
}

