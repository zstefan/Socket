#pragma once

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <memory>

#ifdef __linux__ 
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <fcntl.h>
#elif _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
    #error Unsupported OS
#endif

#define DEFAULT_PORT 15717

namespace Network
{

class MySocket
{
    public:
    /**
    * @brief This is the socket domain types
    *
    */
    enum struct Domain {
    D_UNSPEC,
    D_LOCAL = 1,
    D_UNIX = 1,
    D_FILE = 1,
    D_INET = 2,
    D_AX25,
    D_IPX,
    D_APPLETALK,
    D_NETROM,
    D_BRIDGE,
    D_ATMPVC,
    D_X25,
    D_INET6,
    D_ROSE,
    D_DECnet,
    D_NETBEUI,
    D_SECURITY,
    D_KEY,
    D_NETLINK,
    D_ROUTE,
    D_PACKET,
    D_ASH,
    D_ECONET,
    D_ATMSVC,
    D_RDS,
    D_SNA,
    D_IRDA,
    D_PPPOX,
    D_WANPIPE,
    D_LLC,
    D_IB,
    D_MPLS,
    D_CAN,
    D_TIPC,
    D_BLUETOOTH,
    D_IUCV,
    D_RXRPC,
    D_ISDN,
    D_PHONET,
    D_IEEE802154,
    D_CAIF,
    D_ALG,
    D_NFC,
    D_VSOCK,
    D_KCM,
    D_QIPCRTR,
    D_SMC,
    D_MAX,
  };

    /**
    * @brief This is the socket type definition
    *
    */
    enum struct Type { SOCK_STREAM = 1, SOCK_DGRAM = 2, SOCK_RAW = 3 };
    
    private:
        // int sockFD;
        // int family;
        // int socketType;
        // uint32_t address;
        // int port;
        struct impl;
        std::unique_ptr<impl> pimpl;
    public:
        /**
         * @brief Constructor that should be used by server side to initialize all params
         *
         * @param domain Address domain
         * @param socketType Socket type
         * @param portNum Port number
         * @param address Server address
         */
        MySocket(Domain domain, Type socketType, int portNum, uint32_t address);

        /**
         * @brief Constructor used to create a server representation as remote point in SOCK_DGRAM communication on client side
         *
         * @param family Server address domain
         * @param portNum Server port number
         * @param address Server address
         */
        MySocket(Domain domain, int portNum, const char* address);

        /**
         * @brief Constructor that should be used by server side with default address of the machine on which the server is running
         *
         * @param family Address domain
         * @param socketType Socket type
         * @param portNum Port number
         */
        MySocket(Domain domain, Type socketType, int portNum);

        /**
         * @brief Constructor that should be used by client side
         *
         * @param family Address domain
         * @param socketType Socket type
         */
        MySocket(Domain domain, Type socketType);

        /**
         * @brief Constructor that should be used by server side. Other attributes have default values.
         *
         * @param socketType Socket type
         */
        MySocket(Type socketType);

        /**
         * @brief Constructor that should be used by server side. All attributes have default values.
         *
         */
        MySocket();
        ~MySocket();


        MySocket(const MySocket &) = delete;
        MySocket &operator=(const MySocket &) = delete;
        MySocket(MySocket &&) = delete;
        MySocket &operator=(const MySocket &&) = delete;


        /**
         * @brief This is the function for creating socket
         *
         * @return  true if socket was successfully created, otherwise return false
         */
        bool CreateSocket();

        /**
         * @brief This is the function for binding previously created socket
         *
         * @return  true if socket was successfully binded, otherwise return false
         */
        bool Bind();

        /**
         * @brief This is the function for setting socket options at socket level
         *
         * @param optName Specifies a single option to set
         * @param optValue The value of specified option 
         *
         * @return  true if option is successfully set, otherwise return false
         */
        bool SetSockOption(int optName, int optValue);
        
        /**
         * @brief This is the function for setting socket options
         *
         * @param level Specifies the protocol level at which the option resides
         * @param optName Specifies a single option to set
         * @param optValue The value of specified option 
         *
         * @return  true if option is successfully set, otherwise return false
         */
        bool SetSockOption(int level, int optName, int optValue);

        /**
         * @brief This is the function for setting socket options
         *
         * @param level Specifies the protocol level at which the option resides
         * @param optName Specifies a single option to set
         * @param optValue The value of specified option 
         * @param optLen Size of optValue in bytes
         *
         * @return  true if option is successfully set, otherwise return false
         */
        bool SetSockOption(int level, int optName, const void* optValue, socklen_t optLen);

        /**
         * @brief This is the function that allows listening on the socket for connections
         *
         * @param queue The number of connections that can be waiting while the process is handling a particular connection.
         * Value should be less or equal to 5
         */
        void Listen(int queue);

        /**
         * @brief This is the function that allows listening on the socket for connections. Default is 1 connection
         */
        void Listen();

        /**
         * @brief This is the function for accepting incoming connections
         *
         * @return  Pointer to client socket if operation was successfly, otherwise return nullptr
         */
        std::unique_ptr<MySocket> Accept();
        
        /**
         * @brief This is the function for establish a connection to the server
         *
         * @param family Address domain of server socket
         * @param portNum Port number on which the server will accept connection
         * @param address The address of the host to which you want to connect 
         *
         * @return  true if connection was established, otherwise return false
         */
        bool Connect(int family, int portNum, uint32_t address);

        /**
         * @brief This is the function for establish a connection to the server
         *
         * @param family Address domain of server socket
         * @param portNum Port number on which the server will accept a connection
         * @param address The address of the host with which you want to establish a connection   
         *
         * @return  true if connection was established, otherwise return false
         */
        bool Connect(int family, int portNum, const char* address);

        /**
         * @brief This is the function that recieves data in a buffer from specified socket. It should be used for SOCK_STREAM sockets by server side
         *
         * @param sock The file descriptor of the socket from which the data will be received
         * @param buf Points to a buffer where the message should be stored
         * @param bufSize Specifies the length in bytes of the buffer pointed to by the buf argument
         * @param flags Specifies the type of message reception, if it is 0 function is equivalent to Read
         *
         * @return The number of bytes received or -1 if an error occurred
         */
        int Recieve(int sock, char *buf, size_t bufSize, int flags);

        /**
         * @brief This is the function that recieves data in a buffer. It should be used for SOCK_STREAM sockets by client side
         *
         * @param buf Points to a buffer where the message should be stored
         * @param bufSize Specifies the length in bytes of the buffer pointed to by the buffer argument
         * @param flags Specifies the type of message reception, if it is 0 function is equivalent to Read
         *
         * @return The number of bytes received or -1 if an error occurred
         */
        int Recieve(char *buf, size_t bufSize, int flags);

        /**
         * @brief This is the function that recieves data in a buffer. It should be used for SOCK_DGRAM sockets
         *
         * @param buf Points to a buffer where the message should be stored
         * @param bufSize Specifies the length in bytes of the buffer pointed to by the buffer argument
         * @param flags Specifies the type of message reception
         * @param fromSock Reference to MySocket object in which sender data will be written
         *
         * @return The number of bytes received or -1 if an error occurred
         */
        int RecieveFrom(char *buf, size_t bufSize, int flags, MySocket& fromSock);

        /**
         * @brief This is the function that reads data in a buffer from specified socket. It should be used for SOCK_STREAM sockets by server side
         *
         * @param sock The file descriptor of the socket from which the data will be read
         * @param buf Points to a buffer where the message should be stored
         * @param bufSize Specifies the length in bytes of the buffer pointed to by the buf argument
         *
         * @return The number of bytes received or -1 if an error occurred
         */
        int Read(int sock, char *buf, size_t bufSize);

        /**
         * @brief This is the function that reads data in a buffer. It should be used for SOCK_STREAM sockets by client side
         *
         * @param buf Points to a buffer where the message should be read
         * @param bufSize Specifies the length in bytes of the buffer pointed to by the buffer argument
         *
         * @return The number of bytes received or -1 if an error occurred
         */
        int Read(char *buf, size_t bufSize);

        /**
         * @brief This is the function that sends data from a buffer to specified socket. It should be used for SOCK_STREAM sockets by server side
         *
         * @param sock The file descriptor of the socket to which the data will be sent
         * @param buf Points to a buffer where the message is stored
         * @param bufLen Specifies the length in bytes of the buffer pointed to by the buf argument
         * @param flags Specifies the type of message transmission, if it is 0 function is equivalent to Write
         *
         * @return The number of bytes sent or -1 if an error occurred
         */
        int Send(int sock, const char *buf, int bufLen, int flags);

        /**
         * @brief This is the function that sends data from a buffer to specified socket. It should be used for SOCK_STREAM sockets by client side
         *
         * @param buf Points to a buffer where the message is stored
         * @param bufLen Specifies the length in bytes of the buffer pointed to by the buf argument
         * @param flags Specifies the type of message transmission, if it is 0 function is equivalent to Write
         *
         * @return The number of bytes sent or -1 if an error occurred
         */
        int Send(const char *buf, int bufLen, int flags);

        /**
         * @brief This is the function that sends data from a buffer to specified socket. It should be used for SOCK_DGRAM sockets
         * @param buf Points to a buffer where the message is stored
         * @param bufLen Specifies the length in bytes of the buffer pointed to by the buf argument
         * @param flags Specifies the type of message transmission
         * @param destSock Reference to MySocket object which contains data of reciever
         *
         * @return The number of bytes sent or -1 if an error occurred
         */
        int SendTo(const char* buf, int bufLen, int flags, const MySocket& destSock);

        /**
         * @brief This is the function that writes data from a buffer to specified socket. It should be used for SOCK_STREAM sockets by server side
         *
         * @param sock The file descriptor of the socket to which the data will be written
         * @param buf Points to a buffer where the message is stored
         * @param bufLen Specifies the length in bytes of the buffer pointed to by the buf argument
         *
         * @return The number of bytes sent or -1 if an error occurred
         */
        int Write(int sock, const char *buf, int bufLen);

        /**
         * @brief This is the function that writes data from a buffer to specified socket. It should be used for SOCK_STREAM sockets by client side
         *
         * @param buf Points to a buffer where the message is stored
         * @param bufLen Specifies the length in bytes of the buffer pointed to by the buf argument
         *
         * @return The number of bytes sent or -1 if an error occurred
         */ 
        int Write(const char *buf, int bufLen);
        
        /**
         * @brief This is the function that returns a file descriptor of socket
         */
        int GetFD() const; 

        /**
         * @brief This is the function that returns a port number of socket
         */
        int GetPort() const;

        /**
         * @brief This is the function that returns a address domain of socket
         */
        MySocket::Domain GetDomain() const;

        /**
         * @brief This is the function that returns an address of socket as unsigned int
         */
        uint32_t GetAddress() const;
    
    private:
        MySocket(int sock, struct sockaddr_in addr);
        sockaddr_in GetAddrStruct() const;
        void SetParamsFromStruct(struct sockaddr_in newAddr);
        void PrintError(const char *msg);
        
};

}