#pragma once

#define MAX_LENGTH 1024 * 2

#define HEAD_ID_LENGTH 2
#define HEAD_DATA_LENGTH 2
#define HEAD_TOTAL_LENGTH 4

#define MAX_SENDQUE 1000
#define MAX_RECVQUEUE 10000

enum MSG_IDS {
    MSG_HELLO_WORLD = 1001, //测试消息
    MSG_LOGIN = 1002,       // 用户登陆
    MSG_TEXT_CHAT = 1003
};
