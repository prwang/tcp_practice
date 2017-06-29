# tcp_practice
程序设计实习_tcp_socket大作业


**注意：此时.pro暂不可用，编译配置以CMakeLists.txt为准**
## 功能特色

* 可扩充的通信协议
* udp打洞，可穿越所有port-restricted型NAT
* 整个程序没有用到任何同步IO操作，保证了界面流畅运行

鉴于期末机考实在太差，恳请老师高抬贵手，**这个作业多加几分**吧，感激不尽！

## TODO lists

* [x] 画UI
* [x] 功能spec
* [ ] 基本程序，详见下列spec

## spec

### 登陆
* [x] 开始时设定右边listbox不可用
* [x] 新用户开始侦听UDP端口
* [x] 新用户向(tcp)服务器发送自己的用户名
* [x] 连接超时：如果connected信号晚于timer的信号，则判定为超时
* [x] 等待服务器发送一个ACK
* [x] 新用户向服务器(udp)发送自己的IP和port 这个操作要定期做
* [x] 新用户等待服务器的udp回复，如果没有（超时），则提示网络错误
* [x] 新用户立刻进行取回操作



### 定期取回

1. [x] 打洞请求（后做）和用户增删（先做）
1. [x] 取回用户数据的时候提供版本号，服务器把该版本以后的所有版本的增删操作发送给用户
1. [x] 收到打洞请求，则向目标用户发送数据包，并通知服务器打洞已完成，服务器加入打洞请求完成列表





### 消息
1. [ ] 就是要用单个控件实现多标签的功能，应该对每个好友维护一个对应于view的model，至少包括：聊天记录，当前状态（消息框是否冻结）
1. [ ] 冻结当前发送窗口，直到收到反馈再发送
1. [ ] 不记录是否有洞，而是通过发送的数据收到反馈没有来判断，
如果没有收到反馈（一定超时内），则认为是UDP不让发，那么再请求服务器协调
//这时候要不要的冻结界面？
1. [ ] 如果有洞，则直接发送数据包
1. [ ] 如果没洞，则向服务器发送打洞请求，下次目标用户取回数据时会得到打洞请求

//每个人最多一个等待的 数据，否则要一次发一大堆就会拥塞

//那么就应该在发现没有洞的时候给把发信息的屏蔽了
        
1. [ ] 客户端被暂时不允许向这个目标用户发送数据{pending}，
1. [ ] 另一方接到数据，更改界面，并且如果现在对面还是{no}， 直接改称yes
        收到打洞包以后，立刻回复消息内容
1. [ ] 收到打洞完成的通知，则发送等待的数据包


### 下线

删号，
处理UI变化

## 未解决的问题

* [ ] read_first这个结构简直难受。。是否是异步所必须的呢？
* [ ] 服务器端的Vector会不断变大，试着等所有客户端都更新到最新版本，
对服务器的储存进行优化（把所有操作都改成add，服务器端维护一个versionOffset)
* [ ] 对该不该发keep-alive包，该不该发互通的包还不清楚（不能探测出NAT的类型以及实际保存的时间）
* [ ] 加密，多媒体，界面美化...




