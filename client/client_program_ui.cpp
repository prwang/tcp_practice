//
// Created by prwang on 6/30/17.
//
#include "client_program.h"

void ClientProgram::ui_setupFriend()
{ //就是curContact里面的那个Friend_

    //首先若curContact是null，则设置有右边为不可用

    if (!curContact) { ui->enableCtrl->setDisabled(true); return; }
    else ui->enableCtrl->setEnabled(true);

    if (curContact->pendingmsg.size())
        ui->leSendBuffer->setDisabled(true);
    ui->lbInfos->setText(tr("当前好友：%1  IP地址：%2  upd端口：%3")
            .arg(curContact->username).arg(curContact->ipaddr).arg(curContact->port));
    ui->teMessages->setText(curContact->history);
}

void ClientProgram::ui_setall(bool enable)
{
    ui->splitter->setEnabled(enable);
    if (enable)
    {
        ui->lwContacts->setEnabled(true);
        ui_setupFriend();
    }
}

void ClientProgram::on_lwContacts_itemClicked(QListWidgetItem* qw)
{
    Friend_* fr = lst_rev[qw];
    curContact = fr;
    ui_setupFriend();
}




void ClientProgram::ui_add(Friend_ &fr)
{
    if (fr.session == me.session) return;
    lst_rev[fr.disp = new QListWidgetItem(fr.username, ui->lwContacts)] = &fr;
}

void ClientProgram::ui_del(Friend_ &fr)
{
    lst_rev.remove(fr.disp);
    delete fr.disp; fr.disp = nullptr;
}
void ClientProgram::ui_send_error()
{
    errmsg("发送失败，请检查网络");
}
