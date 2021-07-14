#ifndef EXTENSION_H
#define EXTENSION_H

#include <thread>
#include <memory>
#include <map>
#include <vector>
#include <atomic>
#include <functional>

class Extension;
using ExtPtr = std::shared_ptr<Extension>;

/*
class Platform {
public:
    virtual void exit() = 0;
    virtual void setTitle(const std::string& title) = 0;
    virtual void setSize(int width, int height) = 0;
};
*/

class WindowProvider;

class Extension {
public:
    struct Info {
       std::string url;
       int width;
       int height;
       std::string title;
    };

    using ExtFunction = const std::function<void ()>;
    using OnMain = std::function<void(const std::function<void ()>&)>;

public:
    static ExtPtr startExtension(const std::string& address,
                                                     const ExtFunction& ext,
                                                     const OnMain& onMain,
                                 WindowProvider* prov);
    static ExtPtr startExtension(const std::string& url, int port,
                                                     const ExtFunction& ext,
                                                     const OnMain& onMain,
                                 WindowProvider* prov);
    void join();
    int parse_json(const char* str);
    static std::string toString(const std::map<std::string, std::string>& obj);
    Extension(WindowProvider* prov) : m_prov{prov} {}
private:
    void logError(const std::string& msg);
private:
    friend int startWs(const std::string& address, int port, const std::string& protocol, ExtPtr&);
    WindowProvider* m_prov;
    std::thread m_thread;
    std::function<void()> m_exit = nullptr;
    std::function<bool (const std::string&)> m_sendMessage{nullptr};
    OnMain m_OnMain{nullptr};
    //std::mutex m_wait;
    //Info m_info;
    //std::atomic_bool m_valid_info{false};
};


#endif // EXTENSION_H
