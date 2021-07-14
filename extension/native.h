#include <map>
#include <string>
#include <vector>
#include <optional>

class WindowProvider;

namespace Native {

    using FilterType = std::map<std::string, std::vector<std::string>>;
    
    std::vector<std::string> openDialog(
        const std::string& aTitle,
        const std::string& aDefaultPathAndFile,
        bool dirOnly,
        bool allowMultiple,
        const FilterType& filters);

    std::string saveDialog (
     const std::string& aTitle ,
     const std::string& aDefaultPathAndFile,
     const FilterType& filters);

    /// see in sources about format
    bool setAppIcon(WindowProvider* prov, const std::vector<uint8_t>& data, const std::string& type);

    std::optional<std::string> getAppName();

    bool setSize(WindowProvider* prov, int width, int height);
    bool setTitle(WindowProvider* prov, const std::string& name);

    void debugPrint(int level, const char* line);       
}

