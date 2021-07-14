// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_
#define CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_

class Extension;
#include "include/cef_app.h"
#include <functional>
#include <memory>


using OnMain = std::function<void(const std::function<void()>&)>;

// Implement application-level callbacks for the browser process.
class HiillosApp : public CefApp, public CefBrowserProcessHandler {
 public:
  HiillosApp(const OnMain& onMain);
  ~HiillosApp();

  // CefApp methods:
  CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
    return this;
  }

  void OnBeforeCommandLineProcessing(const CefString &process_type, CefRefPtr<CefCommandLine> command_line) override;

  // CefBrowserProcessHandler methods:
  void OnContextInitialized() override;
  CefRefPtr<CefClient> GetDefaultClient() override;

 private:
  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(HiillosApp);
  std::shared_ptr<Extension> m_extension;
  OnMain m_onMain;
};

#endif  // CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_
