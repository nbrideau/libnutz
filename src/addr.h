#ifndef _ADDR_H_INCLUDED
#define _ADDR_H_INCLUDED

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <netinet/in.h>
#include "logger.h"
#include "buffer.h"

/**
 @class Addr 
    Encapsulated internet addresses 

    TODO: IPV6?
*/
class Addr : ClassLogger {
    public:
        Addr(void);
        Addr(int sock);
        Addr(Addr & addr);
        Addr(sockaddr * addr);
        Addr(const char *  ip, unsigned short port);
        Addr(unsigned long ip, unsigned short port);

        bool operator==(Addr & addr); /**< Comparison */

    public:
        const char *    Ip(void);
        unsigned long   Address(void);
        unsigned short  Port(void);
        sockaddr *      GetSockAddr(void);
        

        void SetAddr(sockaddr * addr);
        void SetAddr(Addr   & addr);

    private:
        char        m_ip[INET_ADDRSTRLEN];       /**< Ip address string */
        sockaddr_in m_addr;                      /**< Address structure */
};
 
#endif
