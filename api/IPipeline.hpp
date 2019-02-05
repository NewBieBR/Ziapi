#pragma once

#include <memory>
#include <unordered_map>
#include "Request.hpp"

namespace ziapi {
    /**
     * Forward declaring IModule
     */
    class IModule;

    /**
     * Hook is used to defined Modules' calling order
     * REALLY_FIRST will be called before anything
     * FIRST will be called first
     * MIDDLE should be used by Modules that don't care when they are called
     * LAST will be called last
     * REALLY_LAST will be called after everything
     */
    enum HookType {
        REALLY_FIRST = 0,
        FIRST,
        MIDDLE,
        LAST,
        REALLY_LAST,
    };

    using Config = std::unordered_map<std::string, std::string>;

    /**
     * The Pipeline is the processing line that
     * inputs HTTP requests and outputs corresponding responses
     * Modules are the processing units of the Pipeline
     */
    class IPipeline {
      public:
        virtual ~IPipeline() = 0;

      public:
        /**
         * This function should configure the Pipeline
         */
        virtual bool configure(const Config &) = 0;

        /**
         * Receive a http request
         * This function should initialize a default Response
         * and call the Pipeline's modules while respecting
         * the calling order defined by their hook types
         */
        virtual void handleRequest(const Request &) = 0;

        /**
         * This function should add the module to the Pipeline
         * and save its hook type
         */
        virtual bool hook(std::shared_ptr<IModule>,
                          HookType type = HookType::MIDDLE) = 0;

        /**
         * This function should remove the module from the Pipeline
         */
        virtual void unhook(std::shared_ptr<IModule>) = 0;
    };
} // namespace ziapi