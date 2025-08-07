//
// Created by guo on 2025/8/7.
//

#ifndef __SOLAR_TCPSERVER_H__
#define __SOLAR_TCPSERVER_H__

#include <memory>
#include <functional>

#include "Socket.h"
#include "Core/IOManager.h"

namespace solar {
class TcpServer : public std::enable_shared_from_this<TcpServer>{
public:
    typedef std::shared_ptr<TcpServer> ptr;
    TcpServer(IOManager* worker = IOManager::GetThis());
    virtual ~TcpServer();

    virtual bool bind(Address::ptr addr);
    virtual bool bind(const std::vector<Address::ptr>& addrs, std::vector<Address::ptr>& fails);
    virtual bool start();
    virtual bool stop();

    uint64_t getReadTimeout() const { return m_readTimeout; }
    std::string getName() const { return m_name; }
    void setReadTimeout(uint64_t v) { m_readTimeout = v; }
    void setName(const std::string& v) { m_name = v; }

    bool isStop() const { return m_isStop; }
protected:
    virtual void handleClient(Socket::ptr client);
private:
    IOManager* m_worker;
    std::vector<Socket::ptr> m_socks; //< listening socket
    uint64_t m_readTimeout;
    std::string m_name;
    bool m_isStop;
};
}

#endif //__SOLAR_TCPSERVER_H__