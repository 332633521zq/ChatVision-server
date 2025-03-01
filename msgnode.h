#pragma once

#include "ConstValue.h"
#include "logicnode.h"
#include <boost/asio.hpp>
#include <memory>

class MsgNode
{
    friend class Session;

public:
    MsgNode(short max_len)
        : _total_len(max_len + HEAD_TOTAL_LENGTH)
        , _cur_len(0)
    {
        _data = new char[_total_len + 1]();
        _data[_total_len] = '\0';
    }

    ~MsgNode() { delete[] _data; }

    void Clear()
    {
        ::memset(_data, 0, _total_len);
        _cur_len = 0;
    }

    int _cur_len;
    int _total_len;
    char* _data;
};

class RecvNode : public MsgNode
{
    // friend class Session;

public:
    RecvNode(short max_len, short msg_id);
    short GetMsgId() { return _msg_id; }

private:
    short _msg_id;
};

class SendNode : public MsgNode
{
    // friend class Session;

public:
    SendNode(const char* data, short max_len, short msg_id);

    short GetMsgId() { return _msg_id; }

private:
    short _msg_id;
};
