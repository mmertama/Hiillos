#ifndef WINDOW_PROVIDER_H
#define WINDOW_PROVIDER_H

#include <vector>

using WIN_HANDLE = WINDOWS_HANDLE_TYPE;

class WindowProvider {
  public:
    virtual std::vector<WIN_HANDLE> windows() const = 0;
};

#endif // WINDOW_PROVIDER_H
