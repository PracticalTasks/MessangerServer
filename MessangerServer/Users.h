#pragma once
//#include <vector>
#include <string>
#include "FileSys.h"

class Users
{
public:
	Users();
	std::string getUsersList();

private:
	//ѕолучаем логины из регистрационных данных
	void getLoginsFromData(std::string& data);

private:
	std::string usersList;
	std::string regDB = "C:\\Netwk\\regDB.txt";
	std::unique_ptr<FileSys> CFileSys;
};

