#pragma once

#include "IPipeline.hpp"
#include "Response.hpp"

namespace ziapi {
    class IModule {
      public:
        /**
         * The module should register a hook type in this function
         * Cf IPipeline.hpp for more informations about hook
         */
        virtual bool start(IPipeline *) = 0;

        virtual bool stop() = 0;

        /**
         * Receive a http request
         * The module may modify the Response accordingly to the Request
         */
        virtual void handleRequest(const Request &, Response &) = 0;
    };
} // namespace ziapi