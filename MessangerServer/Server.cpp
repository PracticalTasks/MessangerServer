#include "Server.h"

Server::Server(const uint16_t READ_PORT)
    : sockVec(OPEN_SOCK)
{
    if (!start_server(READ_PORT))
    {
        std::cout << "Can not running echo tcp server on the port " << READ_PORT << std::endl;
        return;
    }

    service.run();
}

Server::~Server()
{
    delete serv_accept;
    delete ep;
    //delete sock;
}

bool Server::start_server(const uint16_t READ_PORT)
{
    //sock = new ip::tcp::socket(service);
    ep = new boost::asio::ip::tcp::endpoint(ip::tcp::v4(), READ_PORT);
    serv_accept = new boost::asio::ip::tcp::acceptor(service, *ep);

    serv_accept->listen();

    for (int i{}; i < OPEN_SOCK; ++i)
    {
        sockVec[i].clientSock = std::make_unique<ip::tcp::socket>(service);
        serv_accept->async_accept(*sockVec[i].clientSock.get(), boost::bind(&Server::accepted_connection,
            this, i, boost::asio::placeholders::error));
    }

    std::cout << "Running ftp server on the port " << READ_PORT << "..." << std::endl;

    return true;
}

void Server::accepted_connection(uint16_t idxSockVec, const boost::system::error_code& ec)
{
    if (!ec)
    {
        ++connectCount;
        std::cout
            << "Client with address "
            << sockVec[idxSockVec].clientSock->remote_endpoint().address().to_string()
            << ":" << ntohs(sockVec[idxSockVec].clientSock->remote_endpoint().port())
            << std::endl;

        sockVec[idxSockVec].clientSock->async_read_some(buffer(dest_buff, 256), boost::bind(&Server::logIn,
            this, idxSockVec, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }
    else
        std::cout << "Error async_accept\n";
}

void Server::logIn(uint16_t idxSockVec, const boost::system::error_code& ec, std::size_t bytes_transferred)
{
    if (bytes_transferred > 0)
    {
        //if (cmp_chartostr(dest_buff, CMD_EXT, bytes_transferred))
        //{
        //    std::cout << "Echo server has been stopped ...\n";
        //    return;
        //}

        std::string userId(dest_buff);
        size_t idxSeparatItem = userId.find('@');
        size_t idxSeparatNext = 0;

        if (idxSeparatItem == std::string::npos)
            return;

        std::string userLogin(userId.substr(0, idxSeparatItem));
        std::string userPassword(userId.substr(idxSeparatItem+1));

        std::fstream file;
        if (!loadDBase(file))
        {
            std::cout << "Error loading database\n";
            return;
        }

        //const uint16_t SZ = 99;
        //char buff[SZ]{};
        //file.getline(buff, SZ, '@');
        //std::string userString = buff;

        std::string userString;
        while (!file.eof())
        {
            std::getline(file, userString);
            int itemPos = file.tellg();
            idxSeparatItem = userString.find('@');
            if (idxSeparatItem == std::string::npos)
                return;

            //userString.substr(userString[0], idxSeparatItem);

            if ((userLogin == userString.substr(0, idxSeparatItem)))
            {
                idxSeparatNext = userString.find('@', idxSeparatItem + 1);
                if (idxSeparatNext == std::string::npos)
                    return;

                if (userPassword == userString.substr(idxSeparatItem + 1, idxSeparatNext - (idxSeparatItem + 1)))
                {
                    sockVec[idxSockVec].isReg = true;
                    int itemPos = file.tellg();
                    file.seekg(itemPos - 3);
                    itemPos = file.tellg();
                    //Устанавливаем флаг онлайн
                    file << '1';
                    file.close();
                    std::cout << "Access\n";
                    break;
                }
            }
        }

    }
    else if (bytes_transferred == 0)
    {
        std::cout
            << "Client with address "
            << sockVec[idxSockVec].clientSock->remote_endpoint().address().to_string()
            << ":" << ntohs(sockVec[idxSockVec].clientSock->remote_endpoint().port())
            << " disconnected" << std::endl;
        --connectCount;
    }
}

bool Server::loadDBase(std::fstream& file)
{
    file.open(dBasePath, std::ios::in | std::ios::out);

    if (!file)
        return false;

    std::string sig;
    std::getline(file, sig);
    if (sig != "Database")
    {
        file.close();
        std::cout << "Database is corrupted!\n";
        return false;
    }
    
    return true;
}

bool Server::load_file(std::string const& file_path)
{
    std::vector<char> buff_bin(FILEBUFF_SZ);
    //Должна быть папка "C:\Netwk" в которой сервер будет искать файл 
    std::ifstream file_stream(file_path, std::ifstream::binary);

    if (!file_stream)
        return false;

    //get length of file:
    file_stream.seekg(0, file_stream.end);
    int32_t length = file_stream.tellg();
    file_stream.seekg(0, file_stream.beg);

    std::cout << "Sending file " << file_path << "...\n";

    //Заносить в первые четыре байта длину файла в big endian
    insert_sizefile_tobuff(buff_bin, length);
    file_stream.read(buff_bin.data() + 4, FILEBUFF_SZ - 4);

    if (!send_file(buff_bin))
        return false;

    while (file_stream)
    {
        file_stream.read(buff_bin.data(), FILEBUFF_SZ);
        if (!send_file(buff_bin))
            return false;
    }

    std::cout << "Done\n";
    //sock->async_read_some(buffer(dest_buff, 256), boost::bind(&Server::waiting_request,
    //    this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

    return true;
}

void Server::waiting_request(const boost::system::error_code& ec, std::size_t bytes_transferred)
{

    if (bytes_transferred > 0)
    {
        //if (cmp_chartostr(dest_buff, CMD_EXT, bytes_transferred))
        //{
        //    std::cout << "Echo server has been stopped ...\n";
        //    return;
        //}

        std::string userId(dest_buff);
        

        //if (!load_file(userId))
        //{
        //    std::cout << "Error load file " << userId;
        //}
    }
    else if (bytes_transferred == 0)
        std::cout << "Disconnected\n";

}

void Server::insert_sizefile_tobuff(std::vector<char>& buff, int32_t val)
{
    uint16_t hw_val{}; //старшее слово
    uint16_t lw_val{}; //младшее слово

    hw_val = val >> 16;
    lw_val = val & 0xFFFF;

    auto it = buff.begin();
    it[0] = lw_val & 0xFF;
    it[1] = lw_val >> 8;
    it[2] = hw_val & 0xFF;
    it[3] = hw_val >> 8;
}

bool Server::send_file(const std::vector<char> buff_bin)
{
    uint32_t packet_size = 0;
    int transmit_cnt = 0;
    int size = buff_bin.size();
    std::vector<char> send_buff;
    //write(*sock, buffer(buff_bin));

    return true;
}

//Метод для сравнения символьной строки с string
inline bool Server::cmp_chartostr(const char* buff, const std::string& cmd, const int lenBuff)
{
    if (cmd.size() != lenBuff)
        return false;

    for (int i{}; i < lenBuff; ++i)
    {
        if (tolower(buff[i]) != cmd[i])
            return false;
    }

    return true;
}


