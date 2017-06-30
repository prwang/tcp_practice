//
// Created by prwang on 6/30/17.
//

#include "friend.h"
#include "client_program.h"

void Friend_::sendmsg()
{
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
    if (try_time < 1)
    {
        ++try_time;
        guard->start(timeout); //stop在上面的sendmsg里面
        cp->with_client->writeDatagram(
                compose_obj(opcd::RQ_NEEDPUNC, cp->me.session, session),
                cp->curServer, server_udp);

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

