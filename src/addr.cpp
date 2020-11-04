#include "addr.h"

#include <netdb.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>

/* Shorthand notes from beejs guide to network programming

    struct sockaddr {
        unsigned short    sa_family;    // address family, AF_xxx
        char              sa_data[14];  // 14 bytes of protocol address
    };

    struct sockaddr_in {
        short int          sin_family;  // Address family
        unsigned short int sin_port;    // Port number
        struct in_addr     sin_addr;    // Internet address
        unsigned char      sin_zero[8]; // Same size as struct sockaddr
    };
 
    htons() host to network short
    htonl() host to network long
    ntohs() network to host short
    ntohl() network to host long

    ina.sin_addr.s_addr = inet_addr("10.12.110.57");
    m_addr.sin_family = AF_INET;         // host byte order
    m_addr.sin_port = htons(MYPORT);     // short, network byte order
    inet_aton("10.12.110.57", &(m_addr.sin_addr));
    memset(m_addr.sin_zero, '\0', sizeof m_addr.sin_zero);
    printf("%s", inet_ntoa(ina.sin_addr));

    int getpeername(int sockfd, struct sockaddr *addr, int *addrlen);
    int gethostname(char *hostname, size_t size);
    struct hostent *gethostbyname(const char *name);
    struct hostent {
        char    * h_name;
        char   ** h_aliases;
        int       h_addrtype;
        int       h_length;
        char   ** h_addr_list;
    };
    #define h_addr h_addr_list[0]
 */

/*****************************************************************************
 *
 *****************************************************************************/
Addr::Addr(void) 
: ClassLogger("Addr") {
    m_ip[0] = '\0';
    m_addr.sin_family = AF_INET;       
    bzero(&m_addr, sizeof(sockaddr_in));
    return;
}

/*****************************************************************************
 * Get peername from a socket
 *****************************************************************************/
Addr::Addr(Addr & addr) 
: ClassLogger("Addr") {
    m_ip[0] = '\0';
    memcpy(&m_addr, addr.GetSockAddr(), sizeof(sockaddr));
    inet_ntop(AF_INET, &(m_addr.sin_addr), m_ip, INET_ADDRSTRLEN);
    return;
}


/*****************************************************************************
 * Get peername from a socket
 *****************************************************************************/
Addr::Addr(sockaddr * addr) 
: ClassLogger("Addr") {
    m_ip[0] = '\0';
    memcpy(&m_addr, addr, sizeof(sockaddr_in));
    inet_ntop(AF_INET, &(m_addr.sin_addr), m_ip, INET_ADDRSTRLEN);
    return;
}

/*****************************************************************************
 * Get peername from a socket
 *****************************************************************************/
Addr::Addr(int sock) 
: ClassLogger("Addr") {
    m_ip[0] = '\0';
    socklen_t len = sizeof(sockaddr_in);
    bzero(&m_addr, sizeof(sockaddr_in));
    if (getpeername(sock, (sockaddr *)&m_addr, &len)) {
        LogSysError("Addr::Addr getpeername");
    }
    inet_ntop(AF_INET, &(m_addr.sin_addr), m_ip, INET_ADDRSTRLEN);
    return;
}

/*****************************************************************************
 * Build by ip
 *****************************************************************************/
Addr::Addr(const char *  ip, unsigned short port) 
: ClassLogger("Addr") {
    m_ip[0] = '\0';
    bzero(&m_addr, sizeof(sockaddr_in));
    m_addr.sin_family = AF_INET;
    m_addr.sin_port   = htons(port);
    inet_pton(AF_INET, ip, &(m_addr.sin_addr));
    inet_ntop(AF_INET, &(m_addr.sin_addr), m_ip, INET_ADDRSTRLEN);
    return;
}

/*****************************************************************************
 * Build by address
 *****************************************************************************/
Addr::Addr(unsigned long ip, unsigned short port) 
: ClassLogger("Addr") {
    m_ip[0] = '\0';
    bzero(&m_addr, sizeof(sockaddr_in));
    m_addr.sin_family       = AF_INET;       
    m_addr.sin_port         = htons(port);
    m_addr.sin_addr.s_addr  = htonl(ip);
    inet_ntop(AF_INET, &(m_addr.sin_addr), m_ip, INET_ADDRSTRLEN);
    return;
}


/*****************************************************************************
 * Comparison operator
 *****************************************************************************/
bool Addr::operator==(Addr & addr) {
    if (m_addr.sin_port == addr.m_addr.sin_port
        && m_addr.sin_addr.s_addr == addr.m_addr.sin_addr.s_addr)
       return true;
    return false;
}

/*****************************************************************************
 *
 *****************************************************************************/
void Addr::SetAddr(sockaddr * addr) {
    memcpy(&m_addr, addr, sizeof(sockaddr_in));
    inet_ntop(AF_INET, &(m_addr.sin_addr), m_ip, INET_ADDRSTRLEN);
    return;
}

/*****************************************************************************
 *
 *****************************************************************************/
void Addr::SetAddr(Addr & addr) {
    memcpy(&m_addr, addr.GetSockAddr(), sizeof(sockaddr));
    inet_ntop(AF_INET, &(m_addr.sin_addr), m_ip, INET_ADDRSTRLEN);
    return;
}


/*****************************************************************************
 * Return the IP 
 *****************************************************************************/
const char * Addr::Ip(void) {
    return m_ip;
}

/*****************************************************************************
 * Return the Address
 *****************************************************************************/
unsigned long Addr::Address(void) {
    return ntohl(m_addr.sin_addr.s_addr);
}

/*****************************************************************************
 * Return the port
 *****************************************************************************/
unsigned short Addr::Port(void) {
    return ntohs(m_addr.sin_port);
}

sockaddr * Addr::GetSockAddr(void) { 
     return (struct sockaddr *)&m_addr; 
}

