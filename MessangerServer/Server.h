#pragma once
#include <iostream>
#include <fstream>

#include <boost/bind.hpp>
#include <boost/asio.hpp>

using namespace boost::asio;

#define DEST_BUFFSZ 256
const uint16_t OPEN_SOCK = 10;

struct sockAndReg
{
	std::unique_ptr<ip::tcp::socket> clientSock;
	bool isReg = false;
};

class Server
{
public:
	Server(const uint16_t READ_PORT);
	~Server();

private:
	bool start_server(const uint16_t READ_PORT);
	inline bool cmp_chartostr(const char* buff, const std::string& cmd, const int lenBuff);
	void waiting_request(const boost::system::error_code& ec, std::size_t bytes_transferred);
	void accepted_connection(uint16_t idxSockVec, const boost::system::error_code& ec);

	bool loadDBase(std::fstream& file);
	bool load_file(std::string const& file_path);
	bool send_file(const std::vector<char> buff_bin);
	void insert_sizefile_tobuff(std::vector<char>& buff, int32_t val);

	void logIn(uint16_t idxSockVec, const boost::system::error_code& ec, std::size_t bytes_transferred);

private:
	io_service service;
	//ip::tcp::socket* sock = nullptr;
	ip::tcp::endpoint* ep = nullptr;
	ip::tcp::acceptor* serv_accept = nullptr;

	std::vector<sockAndReg> sockVec;
	uint16_t connectCount = 0;

	char dest_buff[DEST_BUFFSZ]{};
	std::string dBasePath = "C:\\Netwk\\DBase.txt";
	//const std::string CMD_EXT = "exit";
	const uint16_t FILEBUFF_SZ = 4096;



};