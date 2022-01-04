#include <iostream>
#include "cpp-can-parser/CANDatabase.h"

int main(int argc, char** argv) {
    using namespace CppCAN;
    
    std::vector<std::string> successParseFile = {
        "dbc-files/empty.dbc", "dbc-files/single-frame-1.dbc"
    };

    std::vector<size_t> errors;

    size_t i = 0;
    for(const auto& file : successParseFile) {
        try {
            CANDatabase::fromFile(file);
        } 
        catch(const CANDatabaseException& e) {
            std::cerr << "Error with file \"" << file << "\": " << e.what() << std::endl;
            errors.push_back(i);
        }
        catch(...) {
            std::cerr << "An unexpected expection happened during "
                         "the parsing of \"" << file << "\"" << std::endl;
            errors.push_back(i);
        }

        i++;
    }

    std::cout << "-----------" << std::endl;
    if(errors.size() == 0) {
        std::cout << "Success. All tests passed." << std::endl;
    }
    else {
        std::cout << "Failure. The following file could not be properly parsed: " << std::endl;
        for(size_t i : errors) {
            std::cout << "  " << successParseFile[i] << std::endl;
        }
    }

    return static_cast<int>(errors.size() != 0);
}