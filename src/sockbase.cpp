
#include "sockbase.h"
#include "exception.h"

#include <cerrno>
#include <cstddef>
#include <unistd.h>
#include <strings.h>


/*****************************************************************************
 *
 *****************************************************************************/
SockBase::SockBase() 
    : ClassLogger("SockBase"), m_addr() {
    m_sock    = -1;
    m_timeout = 0;
    m_lastupdate = time(NULL);
    return;
}


/*****************************************************************************
 *
 *****************************************************************************/
SockBase::SockBase(Addr & addr) 
    : ClassLogger("SockBase"), m_addr(addr.Address(), addr.Port()) {
    m_sock      = -1;
    m_timeout   = 0;
    m_lastupdate = time(NULL);
    return;
}

/*****************************************************************************
 *
 *****************************************************************************/
SockBase::~SockBase(void) {
    Close();
    return;
}

void SockBase::SetAddr(Addr & addr) { 
    m_addr.SetAddr(addr);  
    return;
}

Addr * SockBase::GetAddr(void) { 
    return &m_addr;        
}

int SockBase::SockId(void) { 
    return m_sock;         
}

void SockBase::SetTimeout(int secs) { 
    m_timeout = secs;      
    if (IsConnected()) {
        struct timeval tv;
        tv.tv_sec  = m_timeout;
        tv.tv_usec = 0;
        /* Socket timeouts removing connections from thread pool */
        if (setsockopt(m_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval)) == -1) {
            LogSysError("::Create setsockopt SO_RCVTIMEO");
            m_sock = -1;
            return;
        }
    }
    return;
}

bool SockBase::IsConnected(void) { 
    return (-1 != m_sock); 
}

time_t SockBase::LastUpdate(void) { 
    return m_lastupdate; 
}

/*****************************************************************************
 *
 *****************************************************************************/
bool SockBase::Create(void) {
    int flag = 1;
    if (m_sock != -1) return false;
    if ((m_sock = socket(PF_INET, SOCK_STREAM, 0)) >= 0) {
        // Reuse addr helps not holding the socket long
        if (setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(int)) == -1) {
            /* 
               EBADF     The argument s is not a valid descriptor.
               EFAULT    The  address  pointed  to by optval is not in a valid part of
                         the process address space.  For getsockopt(), this error  may
                         also  be  returned  if  optlen  is not in a valid part of the
                         process address space.
               EINVAL    optlen invalid in setsockopt().
               ENOPROTOOPT The option is unknown at the level indicated.
               ENOTSOCK  The argument s is a file, not a socket.
             */
            LogSysError("::Create setsockopt SO_REUSEADDR");
            m_sock = -1;
            return false;
        }
        if (m_timeout) { 
            SetTimeout(m_timeout);
            if (m_sock == -1) return false;
        }
    }
    else {
        // errno set 
        /*
            EACCES Permission  to create a socket of the specified type and/or pro-
              tocol is denied.
           EAFNOSUPPORT
                  The implementation does not support the specified  address  fam-
                  ily.
           EINVAL Unknown protocol, or protocol family not available.
           EMFILE Process file table overflow.
           ENFILE The  system  limit  on  the  total number of open files has been
                  reached.
           ENOBUFS or ENOMEM
                  Insufficient memory is available.  The socket cannot be  created
                  until sufficient resources are freed.
           EPROTONOSUPPORT
                  The  protocol  type  or  the specified protocol is not supported
                  within this domain.
         */
        LogSysError("::Create socket");
        return false;
    }
    return true;
}

/*****************************************************************************
 *
 *****************************************************************************/
bool SockBase::Connect(void) {
    if (m_sock == -1) {
        if (!Create()) return false;
    }
    if (connect(m_sock, m_addr.GetSockAddr(), sizeof(sockaddr)) < 0)    {
        /*
           EACCES For Unix domain sockets, which are identified by pathname: Write
              permission is denied on the socket file, or search permission is
              denied for one of the directories in the path prefix.  (See also
              path_resolution(7).)
           EACCES, EPERM
                  The user tried to connect to a broadcast address without  having
                  the  socket  broadcast  flag  enabled  or the connection request
                  failed because of a local firewall rule.
           EADDRINUSE
                  Local address is already in use.
           EAFNOSUPPORT
                  The passed address didn't have the correct address family in its
                  sa_family field.
           EAGAIN No  more free local ports or insufficient entries in the routing
                  cache.  For PF_INET see the net.ipv4.ip_local_port_range  sysctl
                  in ip(7) on how to increase the number of local ports.
           EALREADY
                  The socket is non-blocking and a previous connection attempt has
                  not yet been completed.
           EBADF  The file descriptor is not a valid index in the  descriptor  ta-
                  ble.
           ECONNREFUSED
                  No one listening on the remote address.
           EFAULT The  socket  structure  address  is  outside  the user's address
                  space.
           EINPROGRESS
                  The socket is non-blocking and the  connection  cannot  be  com-
                  pleted  immediately.  It is possible to select(2) or poll(2) for
                  completion by selecting the socket for writing.  After select(2)
                  indicates  writability,  use  getsockopt(2) to read the SO_ERROR
                  option at level SOL_SOCKET to determine whether  connect()  com-
                  pleted   successfully   (SO_ERROR  is  zero)  or  unsuccessfully
                  (SO_ERROR is one of the usual error codes listed here,  explain-
                  ing the reason for the failure).
           EINTR  The system call was interrupted by a signal that was caught.
           EISCONN
                  The socket is already connected.
           ENETUNREACH
                  Network is unreachable.
           ENOTSOCK
                  The file descriptor is not associated with a socket.
           ETIMEDOUT
                  Timeout while attempting connection.  The server may be too busy
                  to accept new connections.  Note that for IP sockets the timeout
                  may be very long when syncookies are enabled on the server.
         */
        LogSysError("::Connect connect");
        return false;
    }
    return true;
}

/*****************************************************************************
 *
 *****************************************************************************/
bool SockBase::Close(void) {
    if (m_sock == -1) return true;
    if (close(m_sock) < 0) {
        LogSysError("::Close close");
        /*
           EBADF  fd isn't a valid open file descriptor.
           EINTR  The close() call was interrupted by a signal.
           EIO    An I/O error occurred.
        */
        m_sock = -1; // ???
        return false;
    }
    m_sock = -1;
    return true;
}

/*****************************************************************************
 *
 *****************************************************************************/
bool SockBase::Bind(void) {
    if (m_sock == -1) return false;
    
    if (bind(m_sock,  m_addr.GetSockAddr(), sizeof(sockaddr)) < 0) {
        LogSysError("::Bind bind");
        /* 
           EACCES The address is protected, and the user is not the superuser.
           EADDRINUSE
                  The given address is already in use.
           EBADF  sockfd is not a valid descriptor.
           EINVAL The socket is already bound to an address.
           ENOTSOCK
                  sockfd is a descriptor for a file, not a socket.
           The following errors are specific to UNIX domain (AF_UNIX) sockets:
           EACCES Search  permission  is denied on a component of the path prefix.
                  (See also path_resolution(7).)
           EADDRNOTAVAIL
                  A non-existent interface was requested or the requested  address
                  was not local.
           EFAULT my_addr points outside the user's accessible address space.
           EINVAL The  addrlen is wrong, or the socket was not in the AF_UNIX fam-
                  ily.
           ELOOP  Too many symbolic links were encountered in resolving my_addr.
           ENAMETOOLONG
                  my_addr is too long.
           ENOENT The file does not exist.
           ENOMEM Insufficient kernel memory was available.
           ENOTDIR
                  A component of the path prefix is not a directory.
           EROFS  The socket inode would reside on a read-only file system.
         */
        return false;
    }
    return true;
}

/*****************************************************************************
 *
 *****************************************************************************/
bool SockBase::Listen(void) {
    if (m_sock == -1) return false;
    // 5 defines size of backlog pending connections
    if (listen(m_sock, 5) < 0) {
        LogSysError("::Listen listen");
        /* errno set
           EADDRINUSE
              Another socket is already listening on the same port.
           EBADF  The argument sockfd is not a valid descriptor.
           ENOTSOCK
                  The argument sockfd is not a socket.
           EOPNOTSUPP
                  The socket is not of a type that supports  the  listen()  opera-
                  tion.
        */
        return false;
    }
    return true;
}

/*****************************************************************************
 *
 *****************************************************************************/
bool SockBase::Accept(SockBase * sock) {
    socklen_t addrlen = sizeof(sockaddr);
    sockaddr addr;
    bzero(&addr, sizeof(sockaddr));
    if (m_sock == -1) return false;

    // Poll for timeouts 
    if ((sock->m_sock = accept(m_sock, &addr, &addrlen)) == -1) {
        LogSysError("::Accept accept");
        return false;
    }
    sock->m_addr.SetAddr(&addr);
    return true;
}

/*****************************************************************************
 *
 *****************************************************************************/
int SockBase::Send(const byte * buff, size_t bytes) {
    int sent = 0;
    int rv = 0;
    if (!IsConnected()) {
        Close();
        throw Exception("[Send] Connection closed"); 
    }
    do {
        if ((sent = send(m_sock, (buff + rv), (bytes - rv), MSG_NOSIGNAL)) < 0) {
            switch (errno) {
                case EPIPE:
                    Close();
                    throw Exception("[Send] Connection closed"); 
                case EAGAIN:        // Return on timeout? Nah, this works...
                    Close();
                    throw Exception("[Send] Connection timed out"); 
                case ECONNABORTED:
                    Close();
                    throw Exception("[Send] Connection aborted"); 
                case ECONNRESET:
                    Close();
                    throw Exception("[Send] Connection reset"); 
            }
            Close();
            LogSysError("::Send send");
            throw Exception("[Send] bad file descriptor"); 
        }
        rv += sent;
    } while (rv < (int)bytes);
    return rv;
}

/*****************************************************************************
 *
 *****************************************************************************/
int SockBase::Recv(byte * buff, size_t bytes) {
    int rv = 0;
    if (!IsConnected()) {
        Close();
        throw Exception("[Send] Connection closed"); 
    }
    if ((rv = recv(m_sock, buff, bytes, 0)) <= 0) {
        switch (errno) {
            // Timeout
            case EINTR: // read signal(7)
            case EAGAIN:    
                if (rv == -1) return 0;  
            case ECONNABORTED:
                Close();
                throw Exception("[Recv] Connection aborted", GE_NONE); 
            case ECONNRESET:
                Close();
                throw Exception("[Recv] Connection reset", GE_NONE); 
        }
        Close();
        if (!rv) throw Exception("[Recv] Connection reset by peer", GE_NONE); 
        LogSysError("::Recv recv");
        throw Exception("[Recv] Unhandled error", GE_NONE); 
        return -1;
    }
    return rv;
}

/*****************************************************************************
 *
 *****************************************************************************/
bool SockBase::Setup(void) {
    // Server Socket setup
    if (!Create()) {
        Close();
        return false;
    }
    if (!Bind()) {
        Close();
        return false;
    }
    if (!Listen()) {
        Close();
        return false;
    }
    return true;
}
