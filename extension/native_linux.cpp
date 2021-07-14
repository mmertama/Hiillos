#include "native.h"
#include <iostream>
#include <cassert>
#include <set>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <Magick++.h>

#include <fstream>


#include <unistd.h>
#include <sys/types.h>

using namespace Native;

std::vector<std::string> Native::openDialog(
    const std::string& /*aTitle*/,
    const std::string& /*aDefaultPathAndFile*/,
    bool /*dirOnly*/,
    bool /*allowMultiple*/,
const FilterType& /*filters*/) {
    assert(false);  // use pfd from gempyre
    return {};
}

std::string Native::saveDialog (
 const std::string& /*aTitle*/,
 const std::string& /*aDefaultPathAndFile*/,
        const FilterType& /*filters*/) {
    assert(false);  // use pfd from gempyre
    return {};
}


static pid_t getPid(Display *d, Window w, Atom p) {
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes;
    unsigned char *prop;
    const int status = XGetWindowProperty(d, w, p,
            /* long_offset */ 0L,
            /* long_length */ 1024L,
            /* delete */ False,
            /* req_type */ XA_CARDINAL,  &actual_type, &actual_format, &nitems, &bytes, &prop);
    if (status != 0)
        return -1;
    if (nitems < 1)
        return -1;
    pid_t value = prop[0]
          | prop[1] << 8
          | prop[2] << 16
          | prop[3] << 24;
    XFree(prop);
    return value;
}

std::vector<Window> traverse(Display* display) {
    auto win = XDefaultRootWindow(display);
    if(!win)
        return {};
    const auto p = getpid();
    std::set<Window> wins({win});
    std::set<Window> toplevel;
    auto it = wins.begin();
    for(; it != wins.end();) {
        for(; it != wins.end(); ++it) {
            Window root_return;
            Window parent_return;
            Window *children_return;
            unsigned int nchildren_return;
            const Status r = XQueryTree(display, *it, &root_return, &parent_return, &children_return, &nchildren_return);
            if(!r)
                continue;

          //  const auto atom = XInternAtom(display, "Hiillos", False);
            const auto atom = XInternAtom(display, "_NET_WM_PID", False);
            for(unsigned i = 0; i < nchildren_return; i++) {
                const auto pid = getPid(display, children_return[i], atom);
                if(pid == p) {
                    toplevel.insert(children_return[i]);
                }
            }
            for(unsigned i = 0; i < nchildren_return; i++) {
                wins.insert(children_return[i]);
            }
        }
    }
    /*
    for(const auto& u : toplevel) {
        XWindowAttributes a;
        XGetWindowAttributes(display, u, &a);
        std::cout << "a" << a.height << std::endl;
    } */
    return std::vector<Window>{toplevel.begin(), toplevel.end()};
}


bool Native::setSize(WindowProvider* prov, int width, int height) {
    const auto display = XOpenDisplay("");
    if(!display)
        return false;
    for(const auto win : traverse(display)) {
        const auto r = XResizeWindow(display, win, width, height);
        if(!r) {
            return false;
        }
        XEvent event = {};
        event.type = Expose;
        event.xexpose.display = display;
        XSendEvent(display, win, False, ExposureMask, &event);
    }
    XFlush(display);
    return true;
}

bool Native::setTitle(WindowProvider* prov, const std::string& name) {
    const auto display = XOpenDisplay("");
    if(!display)
        return false;
    for(const auto win : traverse(display)) {

        auto str = name.c_str();
        const auto r = XStoreName(display, win, str);
        const auto rr = XSetIconName(display, win, str);
        if(!rr || !r) {
            return false;
        }

        XClassHint *class_hint = XAllocClassHint();
        if (class_hint) {
            class_hint->res_name = class_hint->res_class = const_cast<char*>(str);
            XSetClassHint(display, win, class_hint);
            XFree(class_hint);
        }
    }
    XFlush(display);
    return true;
}

/// see in sources about format
bool Native::setAppIcon(WindowProvider* prov, const std::vector<uint8_t>& data, const std::string& type) {
    const auto display = XOpenDisplay("");
    if(!display)
        return false;

    static bool inited = false;
    if(!inited) {
        char buffer[512];
        const char* p = getcwd(buffer, sizeof(buffer));
        Magick::InitializeMagick(p);
        inited = true;
    }

    Magick::Blob blob(data.data(), data.size());
    Magick::Image image;
    try {
        image.magick(type);
        image.read(blob);
    } catch(Magick::Exception& e) {
         std::cerr << "Bad! error" << e.what() << std::endl;
         return false;
    } catch(std::exception& e) {
        std::cerr << "Bad!! error" << e.what() << std::endl;
        return false;
    } catch(...) {
        std::cerr << "Bad!!! error" << std::endl;
        return false;
    }

    auto w = image.columns();
    auto h = image.rows();

    if(w < 48 || h < 48) {
        image.scale(Magick::Geometry(48, 48));
    }

    w = image.columns();
    h = image.rows();

    const auto pixels = image.getPixels(0, 0, w, h);

    auto buffersz = w * h + 2;
    buffersz += buffersz % 64; // padding to 64 bit system
    auto buffer = new u_int32_t[buffersz];
    std::fill_n(buffer, buffersz, 0);

    int bufferPos = -1;

    buffer[++bufferPos] = w;
    buffer[++bufferPos] = h;

    for(auto y = 0U; y < h; ++y){
        for(auto x = 0U; x < w; ++x) {
            auto q = pixels[w * y + x];
            buffer[++bufferPos] = (q.opacity << 24) | (q.red << 16) | (q.green << 8) | q.blue;
        }
    }

    /*
    std::ofstream dump("foo.ppm");
    dump << "P3" << std::endl;
    dump << w << " " << h << std::endl;
    dump << 255 << std::endl;
    for(auto i = 0U; i < w * h; i++) {
        dump << ((buffer[i] >> 16) & 0xFF) << " ";
        dump << ((buffer[i] >> 8) & 0xFF) << " ";
        dump << ((buffer[i]) & 0xFF) << std::endl;
    }
    dump.close();
    */

    Atom icon = XInternAtom(display, "_NET_WM_ICON", False);
    Atom cardinal = XInternAtom(display, "CARDINAL", False);

    int result = 0;
    for(const auto& win : traverse(display)) {

        result |= XChangeProperty(display,
                                     win,
                                     icon,
                                     cardinal,
                                     32,
                                     PropModeReplace,
                                     (const unsigned char*) buffer,
                                     sizeof(buffer));
        XWMHints hints = {};
        hints.flags = IconPixmapHint;
        hints.icon_pixmap = XCreateBitmapFromData(display, win, (const char*) buffer, w, h);
        if(hints.icon_pixmap)
            XSetWMHints(display, win, &hints);
    }

    XFlush(display);

    delete[] buffer;

    return result;
}

std::optional<std::string> Native::getAppName();

void Native::debugPrint(int level, const char* line) {
    std::cerr << level << ':' << line << std::endl;
}
