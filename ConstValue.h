#pragma once

constexpr int MAX_LENGTH = 1024 * 2;

#define HEAD_ID_LENGTH 2
#define HEAD_DATA_LENGTH 2
#define HEAD_TOTAL_LENGTH 4

#define MAX_SENDQUE 1000
#define MAX_RECVQUEUE 10000

/*
*   1001: 测试消息      ❌
*   1002: 用户登陆      ❌
*   1003: 用户的基本信息   ✅ 
*   1004: 文字聊天消息    ✅          
*   1005: 用户请求与目标聊天的请求被拒绝(拉黑/被拉黑/陌生人但已经发送过一条消息，还没得到回应)  **signal
*   1006: 回复的文字聊天消息     ❌
*   1007: 关注    ✅   **signal
*   1008: 取消关注  ✅   **signal
*   1009: 拉黑    ❌
*   1010: 取消拉黑      ❌
*   1011: 视频通话请求 
*   1020: 同意视频通话    **signal
*   1012: 拒绝视频通话    **signal
*   1013: 语音通话请求    **signal
*   1021: 同意语音通话    **signal
*   1014: 拒绝语音通话    **signal
*   1015: 用户的关注列表   ✅
*   1016: 用户的粉丝列表   ✅
*   1017: 用户的黑名单    ✅
*   1018: 对方不在线     **signal
*   1019: 随机推送可能想认识的用户
*   1022: 聊过天的用户基本信息
*/

enum MSG_IDS {
    MSG_HELLO_WORLD = 1001,
    MSG_LOGIN = 1002,
    MSG_USER_INFO = 1003,
    MSG_TEXT_CHAT = 1004,
    MSG_TEXT_CHAT_REFUSED = 1005,
    MSG_ANWSER_TEXT = 1006,
    MSG_FOLLOWING = 1007,
    MSG_CANCEL_FOLLOW = 1008,
    MSG_BLOCK = 1009,
    MSG_CANCEL_BLOCK = 1010,
    MSG_VIDEO_CHAT = 1011,
    MSG_VIDEO_CHAT_REFUSED = 1012,
    MSG_AUDIO_CHAT = 1013,
    MSG_AUDIO_CHAT_REFUSED = 1014,
    MSG_GET_FOLLOWINGS = 1015,
    MSG_GET_FOLLOWERS = 1016,
    MSG_GET_BLACKLIST = 1017,
    MSG_NOT_ONLINE = 1018,
    MSG_RANDOM_PUSH = 1019,
    MSG_AGREE_VIDEO = 1020,
    MSG_AGREE_AUDIO = 1021,
    MSG_CHATTED_USER = 1022
};

enum Relation_IDS : unsigned int {
    RELATION_BLOCKED = 0,
    RELATION_BLOCK = 1,
    RELATION_STRANGER = 2,
    RELATION_FOLLOWER = 3,
    RELATION_FOLLOWING = 4,
    RELATION_INTERACT = 5
};
