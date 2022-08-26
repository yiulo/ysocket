# ysocket<br>
c/c++ socket 简化<br>

使用方法：<br>
(客户端)<br>
```
Ysocket4 sock4; // 创建一个socket结构体
Yaddr4 addr4 = sock4.create({ 127,0,0,1 }, 8888); // 创建发送目标，并返回一个ipv4的地址结构体
char* data[1400]; // 创建一个数据
sock4.send(data, sizeof(data)); // 发送数据
sock4.receive(data, sizeof(data)); // 接收数据
```
(服务端) (无阻塞)<br>
```
Ysocket4 sock4; // 创建一个socket结构体
Yaddr4 addr4 = sock4.create(8888); // 创建一个接收端口，并返回一个ipv4的地址结构体
char* data[1400]; // 创建一个数据
int ret = sock4.receive(data, sizeof(data), &addr4); // 接收数据，并返回数据长度，并赋值Yaddr4结构体来源地址信息
if (ret > 0)
  sock4.send(data, sizeof(data), &addr4); // 向addr4所记录的地址发送数据
```
