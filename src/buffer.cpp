
#include "buffer.h"
#include "utils.h"
#include <cstring>
#include <arpa/inet.h>

#ifdef HAVE_ENDIAN_H
  #include <endian.h>
#endif

/*****************************************************************************
 *
 *****************************************************************************/
Buffer::Buffer(void) : ClassLogger("Buffer") {
    m_buff      = NULL;
    m_cursor    = NULL;
    m_size      = 0;
    m_sizemax   = 0;
    return;
}

/*****************************************************************************
 *
 *****************************************************************************/
Buffer::Buffer(byte * buff, int size) : ClassLogger("Buffer") {
    m_buff      = NULL;
    m_cursor    = NULL;
    m_size      = 0;
    m_sizemax   = 0;
    Set(buff, size);
    return;
}


/*****************************************************************************
 *
 *****************************************************************************/
Buffer::~Buffer(void) {
    delete [] m_buff;
    return;
}

/*****************************************************************************
 *
 *****************************************************************************/
void Buffer::Reset(void) {
    memset(m_buff, 0xFF, m_sizemax);
    m_cursor = m_buff;
    return;
}

/*****************************************************************************
 *
 *****************************************************************************/
void Buffer::Rewind(void) {
    m_cursor = m_buff;
    return;
}

/*****************************************************************************
 *  Auto resize amount here?
 * 128 or 1024 as usual (performance)
 *****************************************************************************/
void Buffer::Resize(int newsize) {
    byte * tmp;
    if (newsize < m_sizemax) return;

    tmp = new byte[newsize];
    memcpy(tmp, m_buff, m_size);
    delete [] m_buff;

    m_buff      = tmp;
    m_sizemax   = newsize;
    m_cursor    = m_buff + m_size;
    return;
}

/*****************************************************************************
 *
 *****************************************************************************/
void Buffer::Append(byte * buff, int size) {
    if (m_size + size >= m_sizemax) 
        Resize(m_size + size + 128);
    memcpy(m_cursor, buff, size);
    m_cursor += size;
    m_size   += size;
    *m_cursor = '\0';   // Auto null
    return;
}



/*****************************************************************************
 *
 *****************************************************************************/
int Buffer::Size(void) {
    return m_size;
}

/*****************************************************************************
 *
 *****************************************************************************/
byte * Buffer::Get(void) {
    return m_buff;
}

/*****************************************************************************
 *
 *****************************************************************************/
void Buffer::Set(byte * buff, int size) {
    Reset();
    Append(buff, size);
    return;
}

/*****************************************************************************
 * Append bin data
 *****************************************************************************/
Buffer & Buffer::operator << (const char * arg) {
    Append((byte *)arg, strlen(arg) + 1);
    return *this;
}

Buffer & Buffer::operator << (uint64_t n) {
#ifdef HAVE_HTOBE64
    n = htobe64(n);
    this->Append((byte*)&n, sizeof(uint64_t));
#else
    uint32_t u, l;
    u = (n & 0xffffffff00000000LLU) >> 32;
    l = n & 0xffffffff;
    this->Append((byte*) &u, sizeof(uint32_t));
    this->Append((byte*) &l, sizeof(uint32_t));
#endif
    return *this;
}

Buffer & Buffer::operator << (int64_t n) {
#ifdef HAVE_HTOBE64
    n = htobe64(n);
    this->Append((byte*)&n, sizeof(int64_t));
#else
    uint32_t u, l;
    u = (n & 0xffffffff00000000LLU) >> 32;
    l = n & 0xffffffff;
    this->Append((byte*) &u, sizeof(int32_t));
    this->Append((byte*) &l, sizeof(int32_t));
#endif
    return *this;
}


Buffer & Buffer::operator << (uint32_t n) {
    n = htonl(n);
    this->Append((byte*)&n, sizeof(uint32_t));
    return *this;
}

Buffer & Buffer::operator << (int32_t n) {
    n = htonl(n);
    this->Append((byte*)&n, sizeof(int32_t));
    return *this;
}


Buffer & Buffer::operator << (uint16_t n) {
    n = htons(n);
    this->Append((byte*)&n, sizeof(uint16_t));
    return *this;
}

Buffer & Buffer::operator << (int16_t n) {
    n = htons(n);
    this->Append((byte*)&n, sizeof(int16_t));
    return *this;
}

Buffer & Buffer::operator << (char n) {
    Append((byte *)&n, sizeof(char));
    return *this;
}
Buffer & Buffer::operator << (float n) {
    Append((byte *)&n, sizeof(float));
    return *this;
}
Buffer & Buffer::operator << (double n) {
    Append((byte *)&n, sizeof(double));
    return *this;
}
/* XXX: Truncate timeval fields to 32-bit when storing for compatability.
 * Deal as appropriate once it's no longer a issue (but before 2038) */
Buffer & Buffer::operator << (struct timeval arg) {
    int32_t sec = htonl((int32_t) arg.tv_sec);
    int32_t usec = htonl((int32_t) arg.tv_usec);

    Append((byte*)&sec, sizeof(int32_t));
    Append((byte*)&usec, sizeof(int32_t));
    return *this;
}

Buffer & Buffer::operator << (bool arg) {
    Append((byte *)&arg, sizeof(bool));
    return *this;
}


/*****************************************************************************
 * Extract bin data
 *****************************************************************************/
Buffer & Buffer::operator >> (char ** arg) {
    *arg = nstrdup((char *)m_cursor);
    m_cursor += strlen((char *)m_cursor) + 1;
    return *this;
}

Buffer & Buffer::operator >> (uint64_t &n) {
#ifdef HAVE_BE64TOH
    memcpy(&n, this->m_cursor, sizeof(uint64_t));
    this->m_cursor += sizeof(uint64_t);
    n = be64toh(n);
#else
    uint32_t u, l;
    memcpy(&u, this->m_cursor, sizeof(uint32_t));
    this->m_cursor += sizeof(uint32_t);
    memcpy(&l, this->m_cursor, sizeof(uint32_t));
    this->m_cursor += sizeof(uint32_t);
    n = ((uint64_t) u << 32) | l;
#endif
    return *this;
}

Buffer & Buffer::operator >> (int64_t &n) {
#ifdef HAVE_BE64TOH
    memcpy(&n, this->m_cursor, sizeof(int64_t));
    this->m_cursor += sizeof(int64_t);
    n = be64toh(n);
#else
    uint32_t u, l;
    memcpy(&u, this->m_cursor, sizeof(int32_t));
    this->m_cursor += sizeof(int32_t);
    memcpy(&l, this->m_cursor, sizeof(int32_t));
    this->m_cursor += sizeof(int32_t);
    n = ((int64_t) u << 32) | l;
#endif
    return *this;
}

Buffer & Buffer::operator >> (uint32_t &n) {
    memcpy(&n, m_cursor, sizeof(uint32_t));
    this->m_cursor += sizeof(uint32_t);
    n = ntohl(n);
    return *this;
}

Buffer & Buffer::operator >> (int32_t &n) {
    memcpy(&n, m_cursor, sizeof(int32_t));
    this->m_cursor += sizeof(int32_t);
    n = ntohl(n);
    return *this;
}

Buffer & Buffer::operator >> (uint16_t &n) {
    memcpy(&n, this->m_cursor, sizeof(uint16_t));
    this->m_cursor += sizeof(uint16_t);
    n = ntohs(n);
    return *this;
}

Buffer & Buffer::operator >> (int16_t &n) {
    memcpy(&n, this->m_cursor, sizeof(int16_t));
    this->m_cursor += sizeof(int16_t);
    n = ntohs(n);
    return *this;
}

Buffer & Buffer::operator >> (char & arg) {
    memcpy(&arg, m_cursor, sizeof(char));
    m_cursor += sizeof(char);
    return *this;
}
Buffer & Buffer::operator >> (float & arg) {
    memcpy(&arg, m_cursor, sizeof(float));
    m_cursor += sizeof(float);
    return *this;
}
Buffer & Buffer::operator >> (double & arg) {
    memcpy(&arg, m_cursor, sizeof(double));
    m_cursor += sizeof(double);
    return *this;
}
Buffer & Buffer::operator >> (bool & arg) {
    memcpy(&arg, m_cursor, sizeof(bool));
    m_cursor += sizeof(bool);
    return *this;
}
Buffer & Buffer::operator >> (struct timeval & arg) {
    int32_t sec, usec;
    memcpy(&sec, m_cursor, sizeof(int32_t));
    arg.tv_sec = ntohl(sec);
    m_cursor += sizeof(int32_t);

    memcpy(&usec, m_cursor, sizeof(int32_t));
    arg.tv_usec = ntohl(usec);
    m_cursor += sizeof(int32_t);

    return *this;
}

#ifdef NEED_TIME_T_FUNCS
Buffer & Buffer::operator >> (time_t & arg) {
    /* Pass as 64bit over wire, but truncate as needed */
#if SIZEOF_TIME_T == 8
    *this >> (int64_t) arg;
#else
    int64_t sec;
    *this >> sec;
    arg = (int32_t) sec;
#endif
    return *this;
}
#endif

