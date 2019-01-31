#pragma once

#include <string>
#include <map>

namespace ziapi {
    struct Request {
        std::string method;
        std::string uri;
        std::string version;
        std::map<std::string, std::string> headers;
        std::string body;
    };
} // namespace ziapi