
#include <Windows.h>
#include < Shobjidl_core.h>

#include <cassert>
#include <gempyre_utils.h>
#include "window_provider.h"
#include "native.h"

using namespace Native;

using WideString = std::unique_ptr<wchar_t[]>;

static auto getWide(const std::string& narrow) {
    const auto bytes = narrow.size() + 1;
    auto p = WideString(new wchar_t[bytes]);
    size_t len;
    const auto rv = mbstowcs_s(&len, p.get(), bytes, narrow.c_str(), bytes);
    assert(rv == 0);
    return std::move(p); 
}

static auto getWindowsFilter(const FilterType& filter) {
    std::vector<COMDLG_FILTERSPEC> filters;
    std::vector<std::tuple<WideString, WideString>> memory_buffer;
    for(const auto& [name, filter_list] : filter) {
        auto wide_name = getWide(name);
        std::string str;
        for(const auto& spec : filter_list) {
            if(!str.empty())
                str.append(";");
            str.append(spec);    
        }
        auto wide_spec = getWide(str);
        filters.push_back({wide_name.get(), wide_spec.get()});
        std::tuple<WideString, WideString> mem{std::move(wide_name), std::move(wide_spec)};
        memory_buffer.push_back(std::move(mem));
    }
    return std::move(std::make_tuple(filters, std::move(memory_buffer)));
}


template<class T>
void deleter(T *item) {
    item->Release();
}


auto getString(const std::string& str) {
    const auto wide = getWide(str);
    IShellItem *item;
    HRESULT hr = SHCreateItemFromParsingName(wide.get(), NULL, IID_PPV_ARGS(&item));
    (void) hr;
    assert(SUCCEEDED(hr));
    std::unique_ptr<IShellItem, decltype(&deleter<IShellItem>)> ptr(item, &deleter);
    return std::move(ptr);
}

std::vector<std::string> Native::openDialog(
    const std::string& title,
    const std::string& defaultPathAndFile,
    bool dirOnly,
    bool allowMultiple,
    const FilterType& filters) {

    const auto&& [filter, _] = getWindowsFilter(filters);
    const auto filename = GempyreUtils::baseName(defaultPathAndFile);
    const auto dirname = GempyreUtils::pathPop(defaultPathAndFile);

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | 
        COINIT_DISABLE_OLE1DDE);
    assert(SUCCEEDED(hr));
    IFileOpenDialog* fod;
    hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, 
                IID_IFileOpenDialog, reinterpret_cast<void**>(&fod));
    std::unique_ptr<IFileOpenDialog, decltype(&deleter<IFileOpenDialog>)> fileOpen(fod, &deleter);
    assert(SUCCEEDED(hr));

    FILEOPENDIALOGOPTIONS opt;
    hr = fileOpen->GetOptions(&opt);
    assert(SUCCEEDED(hr));
    if(dirOnly)
       opt |= FOS_PICKFOLDERS;
    if(allowMultiple)
       opt |= FOS_ALLOWMULTISELECT;
    fileOpen->SetOptions(opt);

    const auto wide_title = getWide(title);
    fileOpen->SetTitle(wide_title.get());
    
    const auto wide_dir = getString(dirname);
    fileOpen->SetFolder(wide_dir.get());

    const auto wide_file = getWide(filename);
    fileOpen->SetFileName(wide_file.get());

    if(!filter.empty()) {
        const auto sz = (UINT) filter.size();
        fileOpen->SetFileTypes(sz, filter.data());
    }

    hr = fileOpen->Show(NULL);
    std::vector<std::string> name_list;
    if(!SUCCEEDED(hr))
        return name_list;
    // Get the file name from the dialog box.

    IShellItemArray *ro = nullptr;
    hr = fileOpen->GetResults(&ro);
    std::unique_ptr<IShellItemArray, decltype(&deleter<IShellItemArray>)> results(ro, &deleter);
    assert(SUCCEEDED(hr));
    DWORD count;
    hr = results->GetCount(&count);
    assert(SUCCEEDED(hr));
    for(DWORD i = 0; i < count; ++i) {
        PWSTR filePath;
        IShellItem* ii;
        hr =  results->GetItemAt(i, &ii);
        std::unique_ptr<IShellItem, decltype(&deleter<IShellItem>)> item(ii, &deleter);
        hr = item->GetDisplayName(SIGDN_FILESYSPATH, &filePath);
        assert(SUCCEEDED(hr));
        char narrow[1024];
        size_t len;
        wcstombs_s(&len, narrow, sizeof(narrow), filePath, _TRUNCATE);
        name_list.push_back(narrow);
        CoTaskMemFree(filePath);
    }
    CoUninitialize();
	return name_list;
}

std::string Native::saveDialog (
const std::string& title ,
const std::string& defaultPathAndFile,
const FilterType& filters) {
    const auto&& [filter, _] = getWindowsFilter(filters);
    const auto filename = GempyreUtils::baseName(defaultPathAndFile);
    const auto dirname = GempyreUtils::pathPop(defaultPathAndFile);
    std::vector<std::string> name_list;
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | 
        COINIT_DISABLE_OLE1DDE);
    assert(SUCCEEDED(hr));
    IFileSaveDialog* fod;
    hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, 
                IID_IFileSaveDialog, reinterpret_cast<void**>(&fod));
    std::unique_ptr<IFileSaveDialog, decltype(&deleter<IFileSaveDialog>)> fileOpen(fod, &deleter);
    assert(SUCCEEDED(hr));

    const auto wide_title = getWide(title);
    fileOpen->SetTitle(wide_title.get());
    
    const auto wide_dir = getString(dirname);
    fileOpen->SetFolder(wide_dir.get());

    const auto wide_file = getWide(filename);
    fileOpen->SetFileName(wide_file.get());

    if(!filter.empty()) {
        const auto sz = (UINT) filter.size();
        fileOpen->SetFileTypes(sz, filter.data());
    }

    hr = fileOpen->Show(NULL);
    std::string save_name;
    if(!SUCCEEDED(hr))
        return save_name;

    // Get the file name from the dialog box.

    IShellItem *ro = nullptr;
    hr = fileOpen->GetResult(&ro);
    std::unique_ptr<IShellItem, decltype(&deleter<IShellItem>)> result(ro, &deleter);
    if(!SUCCEEDED(hr))
        return save_name;

    PWSTR filePath;
    hr = result->GetDisplayName(SIGDN_FILESYSPATH, &filePath);
    assert(SUCCEEDED(hr));

    char narrow[1024];
    size_t len;
    wcstombs_s(&len, narrow, wcslen(filePath), filePath, _TRUNCATE);
    name_list.push_back(narrow);
    CoTaskMemFree(filePath);
    CoUninitialize();
    save_name = narrow;
	return save_name;
}

/// Accepts only the ICO format, therefore add conversion 
/// into gempyre toolchain using PIL. 
bool Native::setAppIcon(WindowProvider* prov, const std::vector<uint8_t>& data, const std::string& type) {
    (void) type;
    //TODO PNG to ICO conversion
    auto ico_data = (PBYTE) data.data();
    int offset = LookupIconIdFromDirectoryEx(ico_data, TRUE, 0, 0, LR_DEFAULTCOLOR);
    if(offset <= 0)
        return false;
    auto icon = CreateIconFromResourceEx(ico_data + offset,
        (DWORD) data.size(),
        TRUE, 0x00030000, 0, 0, LR_DEFAULTSIZE);
    if(!icon)
        return false;

    GUITHREADINFO info{};
    info.cbSize = sizeof(info); 
    if(!GetGUIThreadInfo(0, &info))
        return false;
       
    std::vector<HWND> wins;
    wins.push_back(info.hwndActive);
    wins.push_back(info.hwndFocus);
    wins.push_back(info.hwndCapture);
    wins.push_back(info.hwndMenuOwner);
    wins.push_back(info.hwndMoveSize);
    wins.push_back(info.hwndCaret);

    for(const auto win : wins) {
        //Change both icons to the same icon handle.
        SendMessage(win , WM_SETICON, ICON_SMALL, (LPARAM) icon);
        SendMessage(win , WM_SETICON, ICON_BIG, (LPARAM) icon);

        //This will ensure that the application icon gets changed too.
        SendMessage(GetWindow(win , GW_OWNER), WM_SETICON, ICON_SMALL, (LPARAM) icon);
        SendMessage(GetWindow(win , GW_OWNER), WM_SETICON, ICON_BIG, (LPARAM) icon);

    }
    return true;
}

bool Native::setSizeWindowProvide(WindowProvider* prov, int width, int height) {
    GUITHREADINFO info{};
    info.cbSize = sizeof(info);
    if(!GetGUIThreadInfo(0, &info))
        return false;

    constexpr int MIN = 128;

    if(width < MIN || height < MIN)
        return false:;

    std::vector<HWND> wins;
    wins.push_back(info.hwndActive);
    wins.push_back(info.hwndFocus);
    wins.push_back(info.hwndCapture);
    wins.push_back(info.hwndMenuOwner);

    for(const auto win : wins) {
        RECT rect;
        if(GetWindowRect(win, &rect) && (rect.right -  rect.left) >= MIN && (rect.right -  rect.left) >= MIN)
            MoveWindow(rect.left, rect.height, width, height, TRUE);
    }
    return true;
}

bool Native::setTitle(WindowProvider* prov; const std::string& name) {
        for(const auto win : prov->windows()) {
            SetWindowTextA((HWND) win, name.c_str());
}


std::optional<std::string> Native::getAppName() {
    wchar_t name[1024];                 
    if(!GetModuleFileName(NULL, name, sizeof(name) * 2))
        return std::nullopt; 
    char buf[2048];           
    if(!WideCharToMultiByte(CP_ACP,0 , name, -1, buf, sizeof(buf), NULL, NULL))
        return std::nullopt;
    return std::make_optional(std::string(buf));
    } 



 void Native::debugPrint(int level, const char* line) {
    std::stringstream ss;
    ss << "WS(" << level << "):" << line << std::endl;
    OutputDebugStringA(ss.str().c_str());
}
