#include "Users.h"

Users::Users()
    : CFileSys(std::make_unique<FileSys>())
{
    std::string buff;
    int32_t count = CFileSys->readFile(regDB, buff);
    if(count != -1)
        getLoginsFromData(buff);
}

std::string Users::getUsersList()
{
    return usersList;
}

void Users::getLoginsFromData(std::string& data)
{
    int currentIdx = 0;
    int nextIdx = data.find(':');
    while (nextIdx != -1)
    {
        usersList.append(data.substr(currentIdx, (nextIdx - currentIdx)) + '@');
        currentIdx = data.find('@', nextIdx) + 1;
        nextIdx = data.find(':', currentIdx);
    }
}

