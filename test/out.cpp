
#include <string_view>
using namespace std::string_view_literals;

#if defined(_DEBUG)
namespace sstd_private {

extern std::string_view getLongWord ( ) {
    return u8R"==(long word

/*begin:import*/
import xxxx_the_debug 1.0
/*end:import*/

/*begin:debug*/
debug information
/*end:debug*/

)=="sv;
}

}
#else
namespace sstd_private {

extern std::string_view getLongWord ( ) {
    return u8R"==(long word

/*begin:import*/
import xxxx 1.0
/*end:import*/

/*remove debug information*/
/*remove debug information*/
/*remove debug information*/

)=="sv;
}

}
#endif
