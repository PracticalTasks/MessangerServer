#include "Server.h"

Server::Server(const uint16_t READ_PORT)
    : sockVec(OPEN_SOCK)
    , CUsers(std::make_unique<Users>())
    , CFileSys(std::make_unique<FileSys>())
{
    //Запускаем сервер
    if (!start_server(READ_PORT))
    {
        std::cout << "Can not running messanger server on the port " << READ_PORT << std::endl;
        return;
    }

    service.run();
}

Server::~Server()
{
}

bool Server::start_server(const uint16_t READ_PORT)
{
    //sock = new ip::tcp::socket(service);
    ep = std::make_unique<boost::asio::ip::tcp::endpoint>(ip::tcp::v4(), READ_PORT);
    serv_accept = std::make_unique<boost::asio::ip::tcp::acceptor>(service, *ep);

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

//Хэндлер для обработки входящих подключений
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
        std::cout << "Access!\n";

        ////Отправляем регистрационные данные
        //std::fstream file;
        //file.open(dBasePath);
        //if (!file.is_open())
        //{
        //    std::cout << "Cannot " << dBasePath << " open file!\n";
        //    return;
        //}

        //std::vector<std::string> regData;
        //std::string currentStr;
        //while (std::getline(file, currentStr))
        //{
        //    regData.append(currentStr);
        //}

        write(*sockVec[idxSockVec].clientSock.get(), buffer(CUsers->getUsersList()));

        //Вызываем асинхронную функцию чтения которая будет ждать входящие данные
        sockVec[idxSockVec].clientSock->async_read_some(buffer(dest_buff, 256), boost::bind(&Server::readData,
            this, idxSockVec, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }
    else
        std::cout << "Error async_accept\n";
}


//Коды запросав для сервера. Сервер отвечает на выполненный запрос кодом 0xEF и на невыполненный 0xFF
// 0x01 - Записать данные нового пользователя в базу данных, 0x02 - просьба авторизовать пользователя 
void Server::readData(uint16_t idxSockVec, const boost::system::error_code& ec, std::size_t bytes_transferred)
{
    if (bytes_transferred > 0)
    {
        std::string userId(dest_buff);
        uint8_t request = userId.at(userId.size() - 1);
        userId.pop_back();

        switch(request)
        {
            case 0x01:
                CFileSys->writeEndFile(regDB, userId);
                //if(CFileSys->writeEndFile(regDB, userId) != errOpenFile)
                //    write(*sockVec[idxSockVec].clientSock.get(), buffer((char*)ANSWER_SUCCESS, 1));
                //else
                //    write(*sockVec[idxSockVec].clientSock.get(), buffer((char*)ANSWER_FAILED, 1));
                break;
            
            case 0x02:
                if (checkUserAuth(userId))
                {
                    sockVec[idxSockVec].isReg = true;
                    char buff[] = { ANSWER_SUCCESS };
                    write(*sockVec[idxSockVec].clientSock.get(), buffer(buff));
                }
                else
                {
                    char buff[] = { ANSWER_FAILED };
                    write(*sockVec[idxSockVec].clientSock.get(), buffer(buff));
                }
                break;

            default:
                break;

        }
    }
    else if (bytes_transferred == 0)
    {
        std::cout
            << "Client with address "
            << sockVec[idxSockVec].clientSock->remote_endpoint().address().to_string()
            << ":" << ntohs(sockVec[idxSockVec].clientSock->remote_endpoint().port())
            << " disconnected" << std::endl;
        //sockVec.erase(sockVec.begin() + idxSockVec);
        --connectCount;
        return;
    }

    //Обнуляем буффер для приёма данных
    for (int i = 0; i < DEST_BUFFSZ; ++i)
        dest_buff[i] = 0;
    
    //Вызываем асинхронную функцию чтения которая будет ждать входящие данные
    sockVec[idxSockVec].clientSock->async_read_some(buffer(dest_buff, 256), boost::bind(&Server::readData,
        this, idxSockVec, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

bool Server::checkUserAuth(std::string data)
{
    std::string userId = data.substr(0, data.find(':'));
    std::string regData;
    CFileSys->readFile(regDB, regData);
    //Проверяем есть ли данные пользователя(логин оканчивается ':')
    int currentIdx = regData.find(userId + ':');
    int nextIdx = 0;

    if (currentIdx != -1)
    {
        //Увеличеваем текущий idx на количество символов в логине + ':'
        currentIdx += userId.size() + 1;
        //После ':' находится пароль и заканчивается '@'
        nextIdx = regData.find('@', currentIdx);
        nextIdx -= currentIdx;

        userId = regData.substr(currentIdx, nextIdx);

        std::string password = data.substr(data.find(':') + 1, (data.size() - 1) - (data.find(':') + 1));
        if (userId == password)
            return true;
    }

    return false;
}


bool Server::loadDBase(std::fstream& file)
{
    file.open(regDB, std::ios::in | std::ios::out | std::ios::app);

    if (!file)
        return false;

    //std::string sig;
    //std::getline(file, sig);
    //if (sig != "Database")
    //{
    //    file.close();
    //    std::cout << "Database is corrupted!\n";
    //    return false;
    //}
    
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


