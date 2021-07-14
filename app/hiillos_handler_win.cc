// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "hiillos_handler.h"

#include <windows.h>
#include <string>

#include "include/cef_browser.h"

void HiillosHandler::PlatformOnCreated(CefRefPtr<CefBrowser> browser) {
    CefWindowHandle hwnd = browser->GetHost()->GetWindowHandle();
    m_windows.push_back((long) hwnd);
}

void HiillosHandler::PlatformTitleChange(CefRefPtr<CefBrowser> browser,
                           const CefString& title) {
  CefWindowHandle hwnd = browser->GetHost()->GetWindowHandle();
  if (hwnd)
    SetWindowText(hwnd, std::wstring(title).c_str());
}
