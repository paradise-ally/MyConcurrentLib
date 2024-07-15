# MyConcurrentLib
This project is a C++ implementation of high-performance server framework-co-programming library , refer to the open source project sylar (https://github.com/sylar-yin/sylar), used to consolidate the basics, exercise hands-on ability

case_1 : 在单一线程下，主协程负责调度任务(主协程即调度协程)和子协程负责执行任务

case_2: 在多线程下，主线程添加任务完成后，可以选择加入其他工作线程或等待其他工作线程完成任务，工作线程在调度协程和任务协程中切换，所有任务完成后join主线程结束

case_3:多线程协程模型 在服务器下的应用 一个简单的echo服务器
io_scheduler继承于之前的scheduler类

case_4:ioscheduler在上个基础上继承于计时模块 增加了计时器功能（时间堆实现）

test:sylar,libco,libevent,epoll测试示例
