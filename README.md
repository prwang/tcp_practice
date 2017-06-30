# tcp_practice
程序设计实习_tcp_socket大作业


**注意：此时.pro暂不可用，编译配置以CMakeLists.txt为准**
## 功能特色

* 实现了一定规模的，可扩充的通信协议
* udp打洞，目的是可穿越所有port-restricted型NAT
* 整个程序没有用到任何同步IO操作，保证了界面流畅运行

鉴于期末机考实在太差，恳请老师高抬贵手，**这个作业多加几分**吧，感激不尽！

## 基本功能spec

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

* [x] 打洞请求（后做）和用户增删（先做）
* [x] 取回用户数据的时候提供版本号，服务器把该版本以后的所有版本的增删操作发送给用户
* [x] 收到打洞请求，则向目标用户发送数据包，并通知服务器打洞已完成，服务器加入打洞请求完成列表



* [ ] 确认一下服务器端打洞已经写完了
* [x] 把服务器端像用户端那样，把三个表整到一起去



### 消息

需不需要开打洞包的判断依据(暂时)就是短时间内有没有发送成功

1. [x] 就是要用单个控件实现多标签的功能，应该对每个好友维护一个对应于view的model，至少包括：聊天记录，当前状态（消息框是否冻结）
    注意：特判如果是自己则不显示
    
1. [ ] 冻结当前发送窗口，直到收到反馈再发送
1. [x] 不记录是否有洞，而是通过发送的数据收到反馈没有来判断，
如果没有收到反馈（一定超时内），则认为是UDP不让发，那么再请求服务器协调
//这时候要不要的冻结界面？
一旦有pendingMSG就不响应发送事件
1. [x] 如果有洞，则直接发送数据包
1. [x] 如果没洞，则向服务器发送打洞请求，下次目标用户取回数据时会得到打洞请求

//每个人最多一个等待的 数据，否则要一次发一大堆就会拥塞

//那么就应该在发现没有洞的时候给把发信息的屏蔽了
        
1. [ ] 客户端被暂时不允许向这个目标用户发送数据，在pendingmsg还有的情况下
1. [x] 另一方接到数据，更改界面
        收到打洞包以后，立刻回复消息内容
1. [x] 收到打洞完成的通知，则发送等待的数据包


### 下线

* [ ] 就是删号
* [ ] 所有要找iterator的地方，需要评估一下
哪些是一定得不为end()，哪些有可能因为先下线而有可能是end()

### UI

* [ ] 好友列表的显示
* [ ] 聊天记录的显示
* [ ] 文本框的显示

## 测试

涉及多IP的考虑使用libvirtd + kvm虚拟机
程序放在只读的squashfs镜像里面

###  本地环回网络 
* [ ] 基础功能

### 扁平局域网  
* [ ] 练习架设虚拟网络，
* [ ] 基础功能
* [ ] 设置一定的延时和丢包率，考察错误处理

### 复杂网络拓扑 
* [ ] 测试打洞功能
* [ ] 设置多虚拟NAT
* [ ] 设置防火墙


## 改进方向 

* [ ] read_first这个结构简直难受。。是否是异步所必须的呢？
* [ ] 服务器端的Vector会不断变大，试着等所有客户端都更新到最新版本，
对服务器的储存进行优化（把所有操作都改成add，服务器端维护一个versionOffset)
* [ ] 对该不该发keep-alive包，该不该发互通的包还不清楚
（不能探测出NAT的类型以及实际保存的时间）实际上有很好的算法
( Christopher Daniel Widmer, NAT Traversal Techniques and UDP，2015，pp.64-)
* [ ] QT的异步IO性能不高，考虑服务器整个用```boosKeep-Alive Interval Optimizationt.asio```重写
* [ ] 加密，长消息=文件=TCP连接，实时多媒体，界面美化...




