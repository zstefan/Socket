#include "Network/MySocket.h"


namespace Network
{

void universalBzero(char* buffer, size_t length)
{
    #ifdef __linux__
    bzero(buffer, length);
    #elif _WIN32
    memset(buffer,'\0',length);
    #endif
}

#if _WIN32
    using socketfd_t = SOCKET;
#else
    using socketfd_t = int;
 #endif

struct  MySocket::impl
{
    socketfd_t sockFD;
    MySocket::Domain domain;
    MySocket::Type socketType;
    uint32_t address;
    int port;

};


MySocket::MySocket(Domain domain, Type socketType, int portNum, uint32_t address)
	 : pimpl(new MySocket::impl())
{
    this->pimpl->domain = domain;
    this->pimpl->socketType = socketType;
    this->pimpl->port = portNum;
    this->pimpl->address = address;
}

MySocket::MySocket(Domain domain, int portNum, const char* address)
   	 : pimpl(new MySocket::impl())
{
	this->pimpl->port = portNum; 
    this->pimpl->domain = domain;
    this->pimpl->socketType = MySocket::Type::SOCKET_DGRAM;
    struct hostent *server;

    server = gethostbyname(address);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        return;
    }

    memcpy(&this->pimpl->address, server->h_addr, server->h_length);
}

MySocket::MySocket(Domain domain, Type socketType, int portNum)
	 : pimpl(new MySocket::impl())
{
    this->pimpl->domain = domain;
    this->pimpl->socketType = socketType;
    this->pimpl->port = portNum;
    this->pimpl->address = INADDR_ANY;
}

MySocket::MySocket(Domain domain, Type socketType)
{ 
    this->pimpl->domain = domain;
    this->pimpl->socketType = socketType;
}

MySocket::MySocket(int sock, struct sockaddr_in addr)
{
    this->pimpl->sockFD = sock;
    this->SetParamsFromStruct(addr);
}

MySocket::MySocket(MySocket::Type socketType)
	
{
    this->pimpl->domain = MySocket::Domain::D_INET;
    this->pimpl->socketType = socketType;
    this->pimpl->address = INADDR_ANY;
}

MySocket::MySocket()
    : pimpl(new MySocket::impl())
{
}

MySocket::~MySocket()
{
    close(this->pimpl->sockFD);
}

bool MySocket::CreateSocket()
{
    this->pimpl->sockFD = socket((int)this->pimpl->domain,(int) this->pimpl->socketType, 0);
    if(this->pimpl->sockFD < 0)
    {
        PrintError("ERROR opening socket");
        return false;
    }    
    return true;
}

bool MySocket::Bind()
{
    struct sockaddr_in myStruct = this->GetAddrStruct();
    if (bind(this->pimpl->sockFD, (struct sockaddr *) &myStruct, sizeof(myStruct)) < 0) 
    {
        PrintError("ERROR on binding");
        return false;
    }
    return true;        
}

bool MySocket::SetSockOption(int optName, int optValue)
{
    return SetSockOption(SOL_SOCKET, optName, optValue);
}

bool MySocket::SetSockOption(int level, int optName, int optValue)
{
    std::string s = std::to_string(optValue);
    char const *pchar = s.c_str();
    return SetSockOption(level, optName, pchar, sizeof(optValue));
}

#ifdef __linux__
bool MySocket::SetSockOption(int level, int optName, const void* optValue, socklen_t optLen)
{
    if(setsockopt(this->pimpl->sockFD, level, optName, optValue, optLen) == -1)
    {
        PrintError("ERROR on setting option");
        return false;
    }
    return true;
}
#elif _WIN32
bool MySocket::SetSockOption(int level, int optName, const char* optValue, socklen_t optLen)
{
    if(setsockopt(this->pimpl->sockFD, level, optName, optValue, optLen) == -1)
    {
        PrintError("ERROR on setting option");
        return false;
    }
    return true;
}
#endif
void MySocket::Listen(int queue)
{
    if(queue > 0 && queue < 6)
        listen(this->pimpl->sockFD, queue);
    else
        listen(this->pimpl->sockFD, 5);  
}

void MySocket::Listen()
{
    listen(this->pimpl->sockFD, 1);
}

std::unique_ptr<MySocket> MySocket::Accept()
{
    struct sockaddr_in cliAddr;
    socklen_t clilen = sizeof(cliAddr);
    int newsockFD = accept(this->pimpl->sockFD, (struct sockaddr *) &cliAddr, &clilen);
    if (newsockFD < 0)
    {
        PrintError("ERROR on accept");
        return nullptr;
    }    

    //proveriti da li je potreban neki cast za cliAddr
    return std::unique_ptr<MySocket>(new MySocket(newsockFD, cliAddr));
}

bool MySocket::Connect(int family, int portNum, uint32_t address)
{
    struct sockaddr_in servAddr;
    servAddr.sin_family = family;
    servAddr.sin_port = htons(portNum);
    servAddr.sin_addr.s_addr = address;

    if (connect(this->pimpl->sockFD, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) 
    {
        PrintError("ERROR connecting"); 
        return false;
    }    
    return true;
}

bool MySocket::Connect(int family, int portNum, const char* address)
{
    struct hostent *server;

    server = gethostbyname(address);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        return false;
    }

    //struct sockaddr_in serv_addr;
    //inet_pton(family, address.c_str(), &serv_addr.sin_addr);

    uint32_t add;
    memcpy(&add, server->h_addr, server->h_length);

    return Connect(family, portNum, add);
}

int MySocket::Recieve(int sock, char *buf, size_t bufSize, int flags)
{
    // #ifdef __linux__
    // bzero(buf, bufSize);
    // #elif _WIN32
    // memset(buf,'\0',bufSize);
    // #endif
    universalBzero(buf, bufSize);
    int n = recv(sock, buf, bufSize, flags);
    if (n < 0) 
        PrintError("ERROR receiving from socket");

    return n;
}

int MySocket::Recieve(char *buf, size_t bufSize, int flags)
{
    return Recieve(this->pimpl->sockFD, buf, bufSize, flags);
}

int MySocket::RecieveFrom(char *buf, size_t bufSize, int flags, MySocket& fromSock)
{
    // #ifdef __linux__
    // bzero(buf, bufSize);
    // #elif _WIN32
    // memset(buf,'\0',bufSize);
    // #endif
    universalBzero(buf,bufSize);
    socklen_t fromlen = sizeof(struct sockaddr_in);
    struct sockaddr_in from;

    int n = recvfrom(this->pimpl->sockFD, buf, bufSize, flags, (struct sockaddr *)&from, &fromlen);
    if (n < 0) 
        PrintError("ERROR receiving from socket");
    else
    {
        fromSock.SetParamsFromStruct(from);
    }
    
    return n;
}


int MySocket::Read(int sock, char *buf, size_t bufSize)
{
    // #ifdef __linux__
    // bzero(buf, bufSize);
    // #elif _WIN32
    // memset(buf,'\0',bufSize);
    // #endif
    universalBzero(buf,bufSize);
    int n = read(sock, buf, bufSize);
    if (n < 0) 
        PrintError("ERROR reading from socket");

    return n;
}

int MySocket::Read(char *buf, size_t bufSize)
{
    return Read(this->pimpl->sockFD, buf, bufSize);
}

int MySocket::Send(int sock, const char *buf, int bufLen, int flags)
{
    int n = send(sock, buf, bufLen, flags);
    if (n < 0) 
        PrintError("ERROR sending to socket");

    return n;
}

int MySocket::Send(const char *buf, int bufLen, int flags)
{
    return Send(this->pimpl->sockFD, buf, bufLen, flags);
}

int MySocket::SendTo(const char* buf, int bufLen, int flags, const MySocket& destSock)
{
    struct sockaddr_in destAddr = destSock.GetAddrStruct();
    socklen_t destLen = sizeof(destAddr);
    int n = sendto(this->pimpl->sockFD, buf, bufLen, flags, (const struct sockaddr *) &destAddr, destLen);
    if (n  < 0)
        PrintError("ERROR sending to socket");
    return n;
}

int MySocket::Write(int sock, const char *buf, int bufLen)
{
    int n = write(sock, buf, bufLen);
    if (n < 0) 
        PrintError("ERROR writing to socket");

    return n;
}

int MySocket::Write(const char *buf, int bufLen)
{
    return Write(this->pimpl->sockFD, buf, bufLen);
}

void MySocket::SetParamsFromStruct(struct sockaddr_in newAddr)
{
    this->pimpl->domain =static_cast<MySocket::Domain>(newAddr.sin_family);
    this->pimpl->address = newAddr.sin_addr.s_addr;
    this->pimpl->port = ntohs(newAddr.sin_port);
}

sockaddr_in MySocket::GetAddrStruct() const
{
    struct sockaddr_in retValue;
    // //bzero((char *) &retValue, sizeof(retValue));
    // #ifdef __linux__
    // bzero((char *) &retValue, sizeof(retValue));
    // #elif _WIN32
    // memset((char *) &retValue,'\0',sizeof(retValue));
    // #endif
    universalBzero((char*)&retValue, sizeof(retValue));
    retValue.sin_family = (int)this->pimpl->domain;
    retValue.sin_addr.s_addr = this->pimpl->address;
    retValue.sin_port = htons(this->pimpl->port);

    return retValue;
}

void MySocket::PrintError(const char *msg)
{
    perror(msg);
}


int MySocket::GetFD() const { return this->pimpl->sockFD; };

int MySocket::GetPort() const { return this->pimpl->port; };

MySocket::Domain MySocket::GetDomain() const { return this->pimpl->domain; };

uint32_t MySocket::GetAddress() const { return this->pimpl->address; };

}