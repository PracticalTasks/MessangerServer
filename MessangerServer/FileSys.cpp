#include "FileSys.h"
#include <string>

FileSys::FileSys() 
	: file(std::make_unique<std::fstream>())
{
}

int32_t FileSys::readFile(std::string& nameFile, std::string& buff)
{
	if (!openFile(nameFile, std::fstream::in))
		return errOpenFile;
	
	std::getline(*file, buff);
	
	return buff.size();
}

int32_t FileSys::writeEndFile(std::string& nameFile, std::string& buff)
{
	if (!openFile(nameFile, std::ios::out | std::ios::app))
		return errOpenFile;

	*file << buff;
	file->close();
	//std::string tmpBuff;
	//std::getline(*file, tmpBuff);
	//tmpBuff.append(buff + '@');
	//buff.clear();
	//buff.append(tmpBuff);

	return buff.size();
}

bool FileSys::openFile(std::string& nameFile, int mode)
{
	file->open(nameFile, mode);
	if (file->is_open())
		return true;

	return false;
}
