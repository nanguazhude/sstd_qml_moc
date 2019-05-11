
#include <array>

inline constexpr const static std::array globalUtf8Bom{ '\xEF','\xBB','\xBF' };

#include <fstream>
#include <iostream>

template<typename T>
inline void removeUtf8Bom(T & arg) {
    std::array varTmpBuffer{ '1','2','3' };
    arg.read(varTmpBuffer.data(), varTmpBuffer.size());
    if (arg.gcount() < 3) {
    } else {
        if (globalUtf8Bom == varTmpBuffer) {
            return;
        }
    }
    arg.clear();
    arg.seekg(0);
    return;
}

#include <string_view>
using namespace std::string_view_literals;

#include <list>
#include <vector>

#if __has_include(<filesystem>)

#include <filesystem>
namespace fs = std::filesystem;

#else

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#endif

template<typename T>
inline decltype(auto) streamFileName(const T & arg) {
    if constexpr (std::is_constructible_v< std::ifstream, const T & >) {
        return arg;
    } else if constexpr (std::is_constructible_v<std::ifstream, const std::wstring &>) {
        return arg.wstring();
    } else {
        return arg.string();
    }
}

#include <regex>
#include <string>
#include <optional>

using namespace std::string_literals;

inline int getMaxEquals(const std::string_view arg){

    int varAns   = 0;
    int varCount = 0;
    {
        auto varPos = arg.begin();
        const auto varEnd = arg.end();
        for( ;varPos!=varEnd;++varPos ){
            if(*varPos=='='){
                ++varCount ;
            }else{
                if(varCount>varAns){
                    varAns=varCount;
                }
                for( ++varPos;varPos!=varEnd;++varPos ){
                    if( *varPos=='=' ){
                        varCount = 1;
                        ++varPos;
                        break;
                    }
                }
            }
        }
    }

    if(varCount>varAns){
        varAns=varCount;
    }

    return varAns;
}

class Concept{
public:
    const fs::path inputFileName;
    const fs::path outputFileName;

    std::optional< std::ifstream > inputFile;
    std::optional< std::ofstream > outputFile;
private:
    std::string thisNamespace;
    std::string thisFunctionName;
public:
    inline Concept(std::string_view argNameSpace,
            std::string_view argFunctionName,
            fs::path argInputFileName,
                   std::string_view argoutputFileName) :
        inputFileName{std::move(argInputFileName)},
        outputFileName{argoutputFileName}{
        inputFile.emplace( streamFileName(inputFileName) , std::ios::binary );
        if( !inputFile->is_open() ){
            throw -2;
        }
        removeUtf8Bom(*inputFile);
        outputFile.emplace( streamFileName(outputFileName) , std::ios::binary );
        if( !outputFile->is_open() ){
            throw -3;
        }
        outputFile->sync_with_stdio(false);
        inputFile->sync_with_stdio(false);
        outputFile->write( globalUtf8Bom.data(),globalUtf8Bom.size() );

        thisFunctionName = argFunctionName;
        thisNamespace = argNameSpace;

    }

    inline ~Concept(){
        if( inputFile ){
            inputFile->close();
        }
        if( outputFile ){
            outputFile->close();
        }
    }

private:

    enum : std::size_t {
        normal_line = 0,
        begin_type = 1,
        end_type = 2,
        begin_import_type,
        end_import_type
    };

    class Line{
    public:
        std::string line;
        std::size_t type = normal_line;
    };

    std::list< Line > inputFileData;
    bool isAllNormal{false};
    int equalsCount{1};
    std::string equals;

    void readAll(){

        std::string varLine;

        const static std::regex globalRegexDebugBegin{ "(?:" "\xef" "\xbb" "\xbf" ")?" "\\s*/\\*begin:debug\\*/\\s*" , std::regex::icase };
        const static std::regex globalRegexDebugEnd{ u8R"(\s*/\*end:debug\*/\s*)", std::regex::icase };
        const static std::regex globalRegexImportBegin{ "(?:" "\xef" "\xbb" "\xbf" ")?" "\\s*/\\*begin:import\\*/\\s*" , std::regex::icase };
        const static std::regex globalRegexImportEnd{ u8R"(\s*/\*end:import\*/\s*)", std::regex::icase };
        bool hasDebugData = false;

        const auto & varRegexDebugBegin  = globalRegexDebugBegin;
        const auto & varRegexDebugEnd    = globalRegexDebugEnd;
        const auto & varRegexImportBegin = globalRegexImportBegin;
        const auto & varRegexImportEnd   = globalRegexImportEnd;

        while( inputFile->good() ){
            std::getline(*inputFile,varLine);
            auto & varTheLine = inputFileData.emplace_back();
            if (varLine.empty() == false) {
                if (std::regex_match(varLine, varRegexDebugBegin)) {
                    varTheLine.type = begin_type;
                    hasDebugData = true;
                } else if (std::regex_match(varLine, varRegexDebugEnd)) {
                    varTheLine.type = end_type;
                    hasDebugData = true;
                } else if (std::regex_match(varLine, varRegexImportEnd)) {
                    varTheLine.type = end_import_type;
                    hasDebugData = true;
                } else if (std::regex_match(varLine, varRegexImportBegin)) {
                    varTheLine.type = begin_import_type;
                    hasDebugData = true;
                }
            }
            equalsCount = std::max(equalsCount, getMaxEquals( varLine ));
            varTheLine.line = std::move( varLine );
        }

        isAllNormal = !hasDebugData;
        equals = std::string( static_cast<std::size_t>( 1 + equalsCount) ,'=');

    }

    template<bool isRelease>
    void write(){

        const static std::regex globalRegexTheDebug{ u8R"(_the_debug)", std::regex::icase };
        const auto & varRegexTheDebug = globalRegexTheDebug;

        int varDebugCount = 0;
        int varImportCount = 0;

        auto & varOutStream = * outputFile;

        for (const auto & varLine : inputFileData ) {

            const auto varOldDebugCount = varDebugCount;

            switch (varLine.type) {
            case begin_type:++varDebugCount; break;
            case end_type:--varDebugCount; break;
            case begin_import_type:++varImportCount; break;
            case end_import_type:--varImportCount; break;
            }

            if constexpr(!isRelease){
                varOutStream << varLine.line << '\n';
            }else{
                if ((0 < varDebugCount) || (0 < varOldDebugCount)) {
                    varOutStream << u8"/*remove debug information*/"sv << '\n';
                } else if ( varImportCount > 0 ) {
                    varOutStream << std::regex_replace(varLine.line, varRegexTheDebug, ""s) << '\n';
                } else {
                    varOutStream << varLine.line << '\n';
                }
            }

        }

    }

    void printStart() {
        printNamespaceBegin();
        (* outputFile ) << "extern std::string_view "sv << thisFunctionName ;
        (* outputFile ) << " ( ) {\n    return "sv;
        (* outputFile ) << "u8R\"("sv  << equals;
    }

    void printEnd() {
        (* outputFile ) << ")"sv << equals <<"\"sv"sv   ;
        (* outputFile ) << ";\n}\n"sv;
        printNamespaceEnd();
    }

    void printNamespaceBegin(){
        if( thisNamespace.empty() ){
            return;
        }
        (* outputFile ) << "namespace "sv;
        (* outputFile ) << thisNamespace;
        (* outputFile ) << " {\n"sv;
    }

    void printNamespaceEnd(){
        if( thisNamespace.empty() ){
            return;
        }
        (* outputFile ) << "\n}\n"sv;
    }


public:

    void run(){

        readAll();

        class Lock{
            Concept * const thisp;
        public:
            inline Lock(Concept * arg) : thisp{arg} {
                thisp->printStart();
            }
            inline ~Lock(){
                thisp->printEnd();
            }
        };

        (* outputFile ) << "\n"sv;
        (* outputFile ) << "#include <string_view>\n"sv;
        (* outputFile ) << "using namespace std::string_view_literals;\n"sv;

        if( isAllNormal ){
            Lock varLock{this} ;
            for( const auto & varLine : inputFileData ){
                (* outputFile ) << varLine.line ;
            }
            return;
        }

        (* outputFile ) << "#if defined(_DEBUG)\n"sv ;

        {
            Lock varLock{this} ;
            write<false>();
        }

        (* outputFile ) << "#else\n"sv;

        {
            Lock varLock{this} ;
            write<true>();
        }

        (* outputFile ) << "#endif\n"sv;

    }

};

int main(int argc,char ** argv) try {

    if( argc < 3 ){
        return -1;
    }

    std::string varFileName;
    std::string varFunctionName;
    std::string varNameSpace;

    const fs::path varInput{ argv[1] };
    {
        std::ifstream varReadFile{ streamFileName(varInput) , std::ios::binary };
        if( !varReadFile.is_open() ){
            return -7;
        }
        varReadFile.sync_with_stdio(false);
        removeUtf8Bom(varReadFile);
        static const std::regex globalNameSpaceRegex{ u8R"===(\s*namespace\s*:\s*(\S+)\s*)===" };
        static const std::regex globalFileNameRegex{ u8R"===(\s*fileName\s*:\s*(\S+)\s*)===" };
        static const std::regex globalFunctionNameRegex{ u8R"===(\s*functionName\s*:\s*(\S+)\s*)===" };
        const auto & varFunctionNameRegex = globalFunctionNameRegex;
        const auto & varFileNameRegex = globalFileNameRegex;
        const auto & varNameSpaceRegex = globalNameSpaceRegex;
        std::string varLine;
        while (varReadFile.good()) {
            std::getline(varReadFile, varLine);
            std::smatch varMatch;
            if (std::regex_search(varLine.cbegin(), varLine.cend(), varMatch, varFileNameRegex)) {
                varFileName = varMatch[1].str();
            } else if (std::regex_search(varLine.cbegin(), varLine.cend(), varMatch, varFunctionNameRegex)) {
                varFunctionName = varMatch[1].str();
            } else if (std::regex_search(varLine.cbegin(), varLine.cend(), varMatch, varNameSpaceRegex)) {
                varNameSpace = varMatch[1].str();
            }
        }
    }

    if( varFileName.empty() ) {
        return -19;
    }

    if( varFunctionName.empty() ) {
        return -29;
    }

    Concept varConcept{varNameSpace,
                varFunctionName,
                fs::path(argv[1]).replace_filename(varFileName) ,
                argv[2]};
    varConcept.run();
    return 0;

} catch(int ans){
    return ans;
} catch( ... ){
    return -999999;
}


