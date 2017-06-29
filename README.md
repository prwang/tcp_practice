# tcp_practice
程序设计实习_tcp_socket大作业


**注意：此时.pro暂不可用，编译配置以CMakeLists.txt为准**


## TODO lists

* [x] 画UI
* [ ] 基本程序，详见下列spec
* [ ] 加密，多媒体，界面美化...

##spec

### 登陆
1. [x] 开始时设定右边listbox不可用
1. [x] 新用户开始侦听UDP端口
2. [x] 新用户向(tcp)服务器发送自己的用户名
3. [x] 连接超时：如果connected信号晚于timer的信号，则判定为超时
3. [x] 等待服务器发送一个ACK
3. [x] 新用户向服务器(udp)发送自己的IP和port
3. [x] 新用户等待服务器的udp回复，如果没有（超时），则提示网络错误
4. [x] 新用户立刻进行取回操作



### 定期取回

1. [ ] 打洞请求（后做）和用户增删（先做），还有打洞请求完成
1. [ ] 取回用户数据的时候提供版本号，服务器把该版本以后的所有版本的增删操作发送给用户
1. [ ] 收到打洞请求，则向目标用户发送数据包，并通知服务器打洞已完成，服务器加入打洞请求完成列表
1. [ ] 收到打洞请求完成，则检查对应正在pending的消息，并把他们全部发送出去





### 消息

1. [ ] 如果有洞，则直接发送数据包
1. [ ] 如果没洞，但是还是会发一个空包过去，这样打洞包就有用了，则向服务器发送打洞请求，下次目标用户取回数据时会得到打洞请求
1. [ ] 客户端被暂时不允许向这个目标用户发送数据{pending}，
1. [ ] 另一方接到数据，更改界面，并且如果现在对面还是{no}， 直接改称yes
        收到打洞包以后，立刻回复消息内容
1. [ ]收到打洞完成的通知，则发送等待的数据包


### 下线


