#include <algorithm>
#include <iostream>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <format>
#include <thread>

using namespace std;

int sock = -1;
string uid; // bemfa 私钥
string topic; // TCP 设备主题
string mac; // WOL 目标设备 MAC 地址
string interface; // 发送广播的网卡接口名称
string LOG_LEVEL = "INFO";
const string LOG_PREFIX = "[WOL] ";

jthread receiverThread;
jthread heartbeatThread;

bool connectToServer() {
    addrinfo hints{}, *res; // hints 传入信息，res 返回结果
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    const int status = getaddrinfo("bemfa.com", "8344", &hints, &res);
    if (status != 0) {
        cerr << LOG_PREFIX << "getaddrinfo error: " << gai_strerror(status) << endl;
        return false;
    }

    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if (connect(sock, res->ai_addr, res->ai_addrlen) != 0) {
        cerr << LOG_PREFIX << format("连接失败: {} (错误码: {}）", strerror(errno),errno) << endl;
        close(sock);
        freeaddrinfo(res);
        return false;
    }
    cout << LOG_PREFIX << "连接成功" << endl;
    freeaddrinfo(res);

    // 设置超时时间
    timeval timeout{
        timeout.tv_sec = 5
    };
    // timeval timeout;
    // timeout.tv_sec = 5; // 秒
    // timeout.tv_usec = 0; // 微秒
    // 使用这种初始化方式可能会导致结构体里面成员的数值是随机的
    // 而在 C++中，{}会执行值初始化，将里面成员都自动初始化为 0
    setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO, &timeout, sizeof(timeout));
    return true;
}

bool sendMsg(string msg) {
    const string m = format("{}\r\n", msg);
    return send(sock, m.c_str(), m.size(), 0) >= 0;
}

bool subscribeTopic() {
    const string msg = format("cmd=1&uid={}&topic={}\r\n", uid, topic);
    if (!sendMsg(msg)) {
        cerr << "发送订阅消息失败" << endl;
        return false;
    }
    return true;
}

void heartbeat(const stop_token &stopToken) {
    while (!stopToken.stop_requested()) {
        const string msg = "ping";
        sendMsg(msg);
        this_thread::sleep_for(chrono::seconds(60));
    }
}

void messageReceiver(const stop_token &stopToken);

int fail = -1;

void messageReceiver(const stop_token &stopToken) {
    // 通过超时时间为 5 秒，避免在 read 阻塞造成重连事件不触发
    while (!stopToken.stop_requested()) {
        char buffer[1024] = {0};
        const ssize_t size = read(sock, buffer, 1024);
        if (size > 0) {
            fail = 0;
            const string s(buffer, size - 2); // 很奇怪，有\r\n就读不出来，因此去掉后面两个字节
            if (LOG_LEVEL == "DEBUG") {
                cout << LOG_PREFIX << "Receive Message: " << s << endl;
            }
            int cmdCode = stoi(format("{}", s[4]));
            if (cmdCode == 2) {
                cout << LOG_PREFIX << "Wake On Lan" << endl;
                // 通过 ether-wake 发送 WOL 开机指令
                system(format("/usr/sbin/ether-wake -b -i {} {}", interface, mac).c_str());
            }
        } else if (size < 0 && errno != EAGAIN) {
            fail++;
        }
        if (fail > 3) {
            close(sock);
            sock = -1;
            heartbeatThread.request_stop();
            receiverThread.request_stop();
            cerr << "连接已断开，尝试重新连接" << endl;
            break;
        }
    }
}

int main() {
    LOG_LEVEL = getenv("LOG_LEVEL") ?: "INFO";
    uid = getenv("UID") ?: "";
    topic = getenv("TOPIC") ?: "";
    mac = getenv("MAC") ?: "";
    interface = getenv("INTERFACE") ?: "";
    if (ranges::any_of(
        initializer_list<string>{uid, topic, mac, interface},
        [](const string &s) {
            return s.empty();
        })) {
        cerr << LOG_PREFIX << "有环境变量参数为空" << endl;
        exit(1);
    }

    while (true) {
        if (sock == -1 && connectToServer()) {
            fail = 0;
            subscribeTopic();
            // 启动线程
            receiverThread = jthread(messageReceiver);
            heartbeatThread = jthread(heartbeat);
        }
        this_thread::sleep_for(chrono::seconds(60));
    }

    return 0;
}
