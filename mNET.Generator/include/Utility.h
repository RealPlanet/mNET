#pragma once

#ifndef _H_UTILITY_
#define _H_UTILITY_

#include <algorithm>
#include <string>
#include <filesystem>
#include <fstream>

#include "IndentedStringStream.h"

namespace mnet::utility
{
    constexpr char const* DLL_API_NAME = "MNET_PROTOCOL_FILE_DLL_API";
    constexpr char const* MESSAGE_SIZE_CONSTEXPR_SUFFIX = "MessageSizeInfo";


    inline void ToUpper(std::string& s) {
        std::transform(s.begin(), s.end(), s.begin(),
            toupper);
    }

    inline bool WriteStreamToFile(const std::filesystem::path& fileName, IndentedStringStream& stream) {
        std::ofstream os(fileName);
        if (!os.is_open()) {
            return false;
        }

        os << stream.str();
        os.close();
        return true;
    }
}

#endif // !_H_UTILITY_


