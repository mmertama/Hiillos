// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "hiillos_app.h"

#include <string>
#include <thread>
#include <iostream>

#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"
#include "hiillos_handler.h"
#include "extension/extension.h"
#include "extension/native.h"

#include <gempyre_utils.h>

namespace {

// When using the Views framework this object provides the delegate
// implementation for the CefWindow that hosts the Views-based browser.
class SimpleWindowDelegate : public CefWindowDelegate {
 public:
  explicit SimpleWindowDelegate(CefRefPtr<CefBrowserView> browser_view)
      : browser_view_(browser_view) {}

  void OnWindowCreated(CefRefPtr<CefWindow> window) OVERRIDE {
    // Add the browser view and show the window.
    window->AddChildView(browser_view_);
    window->Show();

    // Give keyboard focus to the browser view.
    browser_view_->RequestFocus();
  }

  void OnWindowDestroyed(CefRefPtr<CefWindow> window) OVERRIDE {
    browser_view_ = nullptr;
  }

  bool CanClose(CefRefPtr<CefWindow> window) OVERRIDE {
    // Allow the window to close if the browser says it's OK.
    CefRefPtr<CefBrowser> browser = browser_view_->GetBrowser();
    if (browser)
      return browser->GetHost()->TryCloseBrowser();
    return true;
  }

  CefSize GetPreferredSize(CefRefPtr<CefView> view) OVERRIDE {
    return CefSize(800, 600);
  }

 private:
  CefRefPtr<CefBrowserView> browser_view_;

  IMPLEMENT_REFCOUNTING(SimpleWindowDelegate);
  DISALLOW_COPY_AND_ASSIGN(SimpleWindowDelegate);
};

class SimpleBrowserViewDelegate : public CefBrowserViewDelegate {
 public:
  SimpleBrowserViewDelegate() {}

  bool OnPopupBrowserViewCreated(CefRefPtr<CefBrowserView> browser_view,
                                 CefRefPtr<CefBrowserView> popup_browser_view,
                                 bool is_devtools) OVERRIDE {
    // Create a new top-level Window for the popup. It will show itself after
    // creation.
    CefWindow::CreateTopLevelWindow(
        new SimpleWindowDelegate(popup_browser_view));

    // We created the Window.
    return true;
  }

 private:
  IMPLEMENT_REFCOUNTING(SimpleBrowserViewDelegate);
  DISALLOW_COPY_AND_ASSIGN(SimpleBrowserViewDelegate);
};

}  // namespace

HiillosApp::HiillosApp(const OnMain& onMain) : m_onMain(onMain) {}

HiillosApp::~HiillosApp() {
    std::cout << "Waiting for extension" << std::endl;
    if(m_extension) {
        m_extension->join();
    }
    std::cout << "Extension done" << std::endl;
}

void HiillosApp::OnBeforeCommandLineProcessing(const CefString &process_type, CefRefPtr<CefCommandLine> command_line) {
#ifdef OS_APPLE
    if(process_type.empty()) {
        command_line->AppendSwitch("use-mock-keychain");
    }
#endif
}

void HiillosApp::OnContextInitialized() {
    CEF_REQUIRE_UI_THREAD();

    CefRefPtr<CefCommandLine> command_line = CefCommandLine::GetGlobalCommandLine();

    std::vector<CefString> arguments;
    command_line->GetArguments(arguments);

    const std::string app = command_line->GetSwitchValue("gempyre-app").ToString();

    if(app.empty()) { //if no url given this OS star
#ifdef OS_WIN
    const auto ao = Native::getAppName(); // cef GetProgram was not working
    if(!ao) {
        std::cerr << "Cannot get current appname" << std::endl;
        std::exit(-1);
    }
    const auto app_name = *ao;
#else
    const auto app_name = command_line->GetProgram().ToString();
#endif

   #ifdef OS_APPLE
         const auto path = GempyreUtils::pushPath(GempyreUtils::pathPop(app_name, 2), "gempyre");
   #else
         const auto path = GempyreUtils::pathPop(app_name, 1);
   #endif
         const auto dir = GempyreUtils::directory(path);
         if(dir.empty()) {
             std::cerr << "Cannot find files from: " << path << " of " << app_name << std::endl;
             std::exit(-1);
             return;
         }

    #ifdef OS_APPLE
         const auto exe = std::find_if(dir.begin(), dir.end(), [&path](const auto& d) {
             return GempyreUtils::isExecutable(GempyreUtils::pushPath(path, d));
         });
    #else
        const auto exe = std::find_if(dir.begin(), dir.end(), [&path](const auto& d) {
            const auto name = GempyreUtils::pushPath(path, d);
            return (d.rfind("gempyre_", 0) == 0) && GempyreUtils::isExecutable(name);
         });
    #endif
         if(exe == dir.end() || exe->empty()) {
             std::cerr << "Cannot find binary at " << path << " of " << app_name << std::endl;
             for(const auto& d : dir) {
                  std::cerr << d << std::endl;
             }
             std::exit(-1);
             return;
         }

         const auto full_name = GempyreUtils::pushPath(path, *exe);

         const auto params = GempyreUtils::join(arguments, " ");
         const auto all_params = "--gempyre-app=\"" + app_name + "\" " + params;
         std::cerr << "CMDLINE " << full_name << " " << all_params << std::endl;
         const auto r = GempyreUtils::execute(full_name, all_params);
         if(r != 0) {
             std::cerr << "Error \'" << r << "\' when executing: " << full_name << " " << all_params << std::endl;
             std::exit(-1);
             return;
         }
         std::exit(0); //we go
    } else {
        const auto app_name = command_line->GetProgram().ToString();
        if(app != app_name) {
            std::cerr << "Error  application mismatch\'" << app << "\', when executing:" << app_name << std::endl;
          //  std::exit(-1);
          //  return;
        } 


        // normal start

        // Create the browser using the Views framework if "--use-views" is specified
        // via the command-line. Otherwise, create the browser using the native
        // platform framework.
        const bool use_views = command_line->HasSwitch("gempyre-use-views");

        int width = -1;
        int height = -1;
        std::string title;

        std::vector<CefString> args;
        command_line->GetArguments(args);

        const auto url = args[0].ToString();

        if(args.size() > 1) {
            width = std::stoi(args[1].ToString());
        }
        if(args.size() > 2) {
            height = std::stoi(args[2].ToString());
        }

        if(args.size() > 3) {
            title = args[3].ToString();
        }

        const std::string width_string = command_line->GetSwitchValue("gempyre-width").ToString();
        const std::string height_string = command_line->GetSwitchValue("gempyre-height").ToString();
        const std::string title_string = command_line->GetSwitchValue("gempyre-title").ToString();
        if(!width_string.empty()) {
            width = std::stoi(width_string.c_str());
        }
        if(!height_string.empty()) {
            height = std::stoi(height_string.c_str());
        }

        if(!title_string.empty()) {
            title = title_string;
        }

        CefRefPtr<HiillosHandler> handler(new HiillosHandler(use_views, nullptr));

        // Specify CEF browser settings here.
        CefBrowserSettings browser_settings;

        const auto exitFunction = [this]() {
            m_onMain([]() {
                // Shut down CEF.
                HiillosHandler* handler = HiillosHandler::GetInstance();
                if (handler && !handler->IsClosing())
                  handler->CloseAllBrowsers(false);
                //CefQuitMessageLoop();
            });
          };


        m_extension = Extension::startExtension(url, exitFunction, m_onMain, handler.get());

        if (use_views) { // OSX we have have to use views framework to change the app icon?
        // Create the BrowserView.
            CefRefPtr<CefBrowserView> browser_view = CefBrowserView::CreateBrowserView(
                handler, url, browser_settings, nullptr, nullptr,
                new SimpleBrowserViewDelegate());

        // Create the Window. It will show itself after creation.
            const auto window = CefWindow::CreateTopLevelWindow(new SimpleWindowDelegate(browser_view));
            if(width > 0 && height > 0)
                window->SetSize({width, height});
            if(!title.empty())
                window->SetTitle(title);
        } else {
        // Information used when creating the native window.
            CefWindowInfo window_info;

    #if defined(OS_WIN)
        // On Windows we need to specify certain flags that will be passed to
        // CreateWindowEx().
            window_info.SetAsPopup(NULL, "cefsimple");
    #endif

            if(width > 1)
                window_info.width = width;
            if(height > 1)
                window_info.height = height;
            if(!title.empty()) {
                cef_string_t cef_window_name = {};
                cef_string_utf8_to_utf16(title.c_str(), title.size(), &cef_window_name);
                window_info.window_name = cef_window_name;
            }
            // Create the first browser window.
            CefBrowserHost::CreateBrowser(window_info, handler, url, browser_settings,
                                      nullptr, nullptr);
        }
    }
}



CefRefPtr<CefClient> HiillosApp::GetDefaultClient() {
  // Called when a new browser window is created via the Chrome runtime UI.
  return HiillosHandler::GetInstance();
}
