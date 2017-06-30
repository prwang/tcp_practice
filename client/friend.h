//
// Created by prwang on 6/30/17.
//
#include "../shared.h"
#ifndef TCP_PRACTICE_FRIEND_H
#define TCP_PRACTICE_FRIEND_H
class ClientProgram;
struct Friend_ :  QObject, Userdata
{
    Q_OBJECT
            QString history;
    QString pendingmsg; //特别地，长度为0，表示是没有正在等待的消息的
    QTimer *guard;
    int try_time;
    QListWidgetItem* disp;
    friend class ClientProgram;
public:
    explicit Friend_(const Userdata& bs, QObject* parent)
            :Userdata(bs),
                                                                 history{},
                                                                 pendingmsg{}, guard(new QTimer(this)),
                                                                 QObject(parent), try_time(0) ,disp(nullptr) { }
    void sendmsg();
    void ok();
    void addmsg() {}
    template<class T1, class ...Tss>
    void addmsg(const T1& t1, const Tss& ... t2)
    {
        history.append(t1);
        addmsg(t2...);
    }

private slots:
    void send_timeout();

};
#endif //TCP_PRACTICE_FRIEND_H
