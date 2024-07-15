#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <event2/event.h>

#define PORT 8888

// 回声处理函数
void echo_read_cb(evutil_socket_t fd, short events, void* arg) {
    char buf[1024];
    int len;
    len = recv(fd, buf, sizeof(buf) - 1, 0);
    if (len <= 0) {
        // 发⽣错误或连接关闭，关闭连接并释放事件资源
        close(fd);
        event_free((struct event*)arg);
        return;
    }
    buf[len] = '\0';
    printf("接收到消息：%s\n", buf);
    // 发送回声消息给客户端
    send(fd, buf, len, 0);
}

// 接受连接回调函数
void accept_conn_cb(evutil_socket_t listener, short event, void* arg) {
    struct event_base* base = (struct event_base*)arg;
    struct sockaddr_storage ss;
    socklen_t slen = sizeof(ss);
    int fd = accept(listener, (struct sockaddr*)&ss, &slen);

    if (fd < 0)
        perror("accept");
    else if (fd > FD_SETSIZE)
        close(fd);
    else {
     // 创建⼀个新的事件结构体
     struct event* ev = event_new(NULL, -1, 0, NULL, NULL);
     // 将新的事件添加到事件循环中
     event_assign(ev, base, fd, EV_READ | EV_PERSIST, echo_read_cb, (void*)ev);
     event_add(ev, NULL);
    }
}

int main() {
    struct event_base* base;
    struct event listener;
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(PORT);
    // 初始化Libevent库
    base = event_base_new();
    // 创建⼀个监听事件
    listener = *event_new(base, -1, EV_READ | EV_PERSIST, accept_conn_cb, (void*)base);
    // 将监听事件绑定到指定的地址和端⼝
    if (event_add(&listener, NULL) == -1) {
        perror("event_add");
        return -1;
    }
    // 开始事件循环
    event_base_dispatch(base);
    return 0;
}