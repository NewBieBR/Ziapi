#pragma once

#include <string>
#include <map>

namespace ziapi {
    struct Response {
        std::string version;
        int status;
        std::string reason;
        std::map<std::string, std::string> headers;
        std::string body;
    };
} // namespace ziapi