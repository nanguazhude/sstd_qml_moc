
#include <string_view>
using namespace std::string_view_literals;
namespace sstd_private {
extern std::string_view getLongWord ( ) {
    return u8R"==(long word)=="sv;
}

}
