#pragma once

#include "nlohmann/json.hpp"
#include "relationalbroker.h"

class MsgBroker : public RelationalBroker
{
public:
    MsgBroker();
    static void StoreChatMsg(nlohmann::json msg);
};
