#include <thread>
#include <regex>
#include <cassert>
#include <iostream>
#include <memory>
#include <gempyre_utils.h>

#include "extension.h"
#include "native.h"

#include "nlohmann/json.hpp"
using Json = nlohmann::json;


int startWs(const std::string& address, int port, const std::string& protocol, ExtPtr&);
std::vector<uint8_t> base64Decode(const std::string& data);

ExtPtr
Extension::startExtension(const std::string& url, const std::function<void()>& onExit, const OnMain& onMain, WindowProvider* prov) {
    const std::regex re(R"((?:([^\:]*)\:\/\/)?(?:([^\:\@]*)(?:\:([^\@]*))?\@)?(?:([^\/\:]*)\.(?=[^\.\/\:]*\.[^\.\/\:]*))?([^\.\/\:]*)(?:\.([^\/\.\:]*))?(?:\:([0-9]*))?(\/[^\?#]*(?=.*?\/)\/)?([^\?#]*)?(?:\?([^#]*))?(?:#(.*))?)");
    std::smatch m;
    if(!std::regex_match(url, m, re)) {
        std::cerr << "Invalid URL: " << url << std::endl;
        std::abort();
    }

    if(m.size() < 8) {
        return nullptr;
    }

    const std::string host = m[5];
    const int port = stoi(m[7]);

    return startExtension(host, port, onExit, onMain, prov);
}

ExtPtr
Extension::startExtension(const std::string& host, int port, const std::function<void()>& onExit, const OnMain& onMain, WindowProvider* prov) {
    auto e = std::make_shared<Extension>(prov);
   // e->m_wait.lock();
    auto extension = std::thread([port, host, onExit, e]() mutable {
        startWs(host, port, "gempyre", e);
     //   e->m_wait.unlock();
        std::cout << "extension thread completed" << std::endl;
        onExit();
    });
    e->m_OnMain = onMain;
    e->m_thread = std::move(extension);
    return e;
}

void Extension::join() {
    if(m_exit)
        m_exit();
    m_thread.join();
}

void Extension::logError(const std::string& msg) {
    m_sendMessage(Json::object({
                      {"type", "log"},
                      {"level", "error"},
                      {"msg", msg}
                  }).dump());
}

int Extension::parse_json(const char* str) {
    const auto json = Json::parse(str, nullptr, false);
    if(json.is_discarded()) {
        logError("Not Json");
        return -5;
    }
    if(json.empty()) {
        logError("Json empty");
        return -1;
    }
    if(!json.contains("type")) {
        logError("Unknown json");
        return -2;
    }
    if(json["type"] == "exit_request") {
        return 1;
    }
    if(json["type"] != "extension") {
        return -3; //ignore
    }

    const auto callid = json["extension_call"].get<std::string>();
    const auto xparams = json["extension_parameters"].get<std::string>();
    const auto params = Json::parse(xparams);
    const auto id = json["extension_id"].get<std::string>();

    if(callid == "openFile") {
        const auto caption = params["caption"].get<std::string>();
        const auto dir = params["dir"].get<std::string>();
        const auto filters = params["filter"].get<Native::FilterType>();
        m_OnMain([this, caption, dir, filters, id]() {
            const auto files = Native::openDialog(caption, dir, false, false, filters);
            if(!files.empty()) {
                if(!m_sendMessage(Json::object({
                                               {"type", "extension_response"},
                                               {"extension_call", "openFileResponse"},
                                               {"extension_id", id},
                                               {"openFileResponse", files.at(0)}}).dump()))
                   logError("Cannot return a file name");
            }
        });
    }
    else if(callid == "openFiles") {
        const auto caption = params["caption"].get<std::string>();
        const auto dir = params["dir"].get<std::string>();
        const auto filters = params["filter"].get<Native::FilterType>();
        m_OnMain([this, caption, dir, filters, id]() {
            const auto files = Native::openDialog(caption, dir, false, true, filters);
            const auto array = Json(files);
            if(!files.empty()) {
                if(!m_sendMessage(Json::object({
                                                                    {"type", "extension_response"},
                                                                    {"extension_call", "openFilesResponse"},
                                                                    {"extension_id", id},
                                                                    {"openFilesResponse", array}}).dump()))
                    logError("Cannot return file names");
            }
        });
    }
    else if(callid == "openDir") {
        const auto caption = params["caption"].get<std::string>();
        const auto dir = params["dir"].get<std::string>();
        m_OnMain([this, caption, dir, id](){
            const auto dirs = Native::openDialog(caption, dir, true, false, {});
            if(!dirs.empty()) {
                if(!m_sendMessage(Json::object({
                                                                {"type", "extension_response"},
                                                                {"extension_call", "openDirResponse"},
                                                                {"extension_id", id},
                                                                {"openDirResponse", dirs.at(0)}}).dump()))
                    logError("Cannot return a dir name");
            }
        });
    }
    else if(callid == "saveFile") {
        const auto caption = params["caption"].get<std::string>();
        const auto dir = params["dir"].get<std::string>();
        const auto filters = params["filter"].get<Native::FilterType>();
        m_OnMain([this, caption, dir, filters, id]() {
            const auto file = Native::saveDialog(caption, dir, filters);
            if(!file.empty()) {
                if(!m_sendMessage(Json::object({
                                                                {"type", "extension_response"},
                                                                {"extension_call", "saveFileResponse"},
                                                                {"extension_id", id},
                                                                {"saveFileResponse", file}}).dump()))
                     logError("Cannot return a save file name");
            }
        });
    }
    else if(callid == "setAppIcon") {
         const auto data = params["image_data"].get<std::string>();
         const auto type = params["type"].get<std::string>();
         const auto bytes = base64Decode(data);
         m_OnMain([this, bytes, type]() {
             if(!Native::setAppIcon(m_prov, bytes, type)) {
                 logError("Cannot set app icon");
             }
        });
    }
    else if(callid == "resize") {
         const auto width = params["width"].get<int>();
         const auto height = params["height"].get<int>();
         m_OnMain([this, width, height]() {
             if(!Native::setSize(m_prov, width, height))
                 logError("Cannot resize");
        });
    }
    else if(callid == "setTitle") {
        const auto name= params["title"].get<std::string>();
         m_OnMain([this, name]() {
             if(!Native::setTitle(m_prov, name))
                 logError("Cannot set title");
        });
    }
    else if (callid == "ui_info") {
        const auto url = params["url"].get<std::string>();
        const auto p = params["params"].get<std::string>();
      //  const auto plist = GempyreUtils::split<std::vector<std::string>>(p, ' ');
        std::cout << url << " - " << p << std::endl;
      /*  m_info.url = url;
        m_info.width = plist.size() > 0 ? GempyreUtils::to<int>(plist[0]) : 0;
        m_info.height = plist.size() > 1 ? GempyreUtils::to<int>(plist[1]) : 0;
        m_info.title = plist.size() > 2 ? plist[2] : "";
        m_valid_info = true;
        m_wait.unlock();*/
    }
    else {
        logError("Not supported:" + callid);
        return -4;
    }

    return 0;
}

std::string Extension::toString(const std::map<std::string, std::string>& map) {
    return Json(map).dump();
}

/*
Extension::Info Extension::waitClient() {
    if(!m_valid_info)
        m_wait.lock(); //wait until fail or connected
    return m_info;
}
*/
