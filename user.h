#pragma once

#include <iostream>
#include <vector>

class User
{
public:
    User();
    void PostToFollowList(unsigned int object_id);
    void RemoveFromFollowList(unsigned int object_id);
    void PostToFollowerList(unsigned int object_id);
    void RemoveFromFollowerList(unsigned int object_id);

private:
    unsigned int _uid;
    std::vector<unsigned int> _follow_list;
    std::vector<unsigned int> _follower_list;
};
