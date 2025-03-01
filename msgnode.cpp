#include "msgnode.h"
#include <iostream>

// MsgNode::MsgNode() {}

RecvNode::RecvNode(short max_len, short msg_id)
    : MsgNode(max_len)
    , _msg_id(msg_id)
{}

SendNode::SendNode(const char *data, short max_len, short msg_id)
    : MsgNode(max_len)
    , _msg_id(msg_id)
{
    // 将 msg_id 和 max_len 转换为网络字节序（大端序）
    short msg_id_network = boost::asio::detail::socket_ops::host_to_network_short(msg_id);
    short max_len_network = boost::asio::detail::socket_ops::host_to_network_short(max_len);

    memcpy(_data, &msg_id_network, HEAD_ID_LENGTH);
    memcpy(_data + HEAD_ID_LENGTH, &max_len_network, HEAD_DATA_LENGTH);
    memcpy(_data + HEAD_ID_LENGTH + HEAD_DATA_LENGTH, data, max_len);

    // // 输出调试信息
    // std::cout << "SendNode msg_id: " << msg_id << std::endl;
    // std::cout << "SendNode max_len: " << max_len << std::endl;
    // std::cout << "SendNode data: " << data << std::endl;

    // // 以十六进制输出 _data 的内容
    // std::cout << "SendNode _data (hex): ";
    // for (int i = 0; i < HEAD_ID_LENGTH + HEAD_DATA_LENGTH + max_len; ++i) {
    //     std::cout << std::hex << (int) (unsigned char) _data[i] << " ";
    // }
    std::cout << std::endl;
}
