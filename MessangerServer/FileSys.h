#pragma once
#include <fstream>

#define errOpenFile -1

class FileSys
{
public:
	FileSys();
	int32_t readFile(std::string& nameFile, std::string& buff);
	int32_t writeEndFile(std::string& nameFile, std::string& buff);

private:
	bool openFile(std::string& nameFile, int mode);

private:
	std::unique_ptr<std::fstream> file;
	//std::string dBasePath = "C:\\Netwk\\regDB.txt";
};

