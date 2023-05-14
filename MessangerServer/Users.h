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
	//�������� ������ �� ��������������� ������
	void getLoginsFromData(std::string& data);

private:
	std::string usersList;
	std::string regDB = "C:\\Netwk\\regDB.txt";
	std::unique_ptr<FileSys> CFileSys;
};

