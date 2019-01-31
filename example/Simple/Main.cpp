#include "IModule.hpp"
#include <iostream>
#include <vector>
#include <unordered_map>

/**
 * In this example, we will:
 *  - implement the Pipeline class while handling the hook type
 *  - implement an ExampleModule that modify the http response
 *  - implement a PrintResponseModule that print out the http response
 *
 * !!!The result of this example will be the same as the Basic example
 * but this time, the http response is printed out by a Module
 */

/**
 * Pipeline class
 * implements IPipeline functions
 */
class Pipeline : public ziapi::IPipeline {
    using Modules = std::vector<std::shared_ptr<ziapi::IModule>>;

  public:
    Pipeline() = default;
    ~Pipeline() = default;

  public:
    void handleRequest(const ziapi::Request &req) final {
        // Create a default response
        ziapi::Response res = {"HTTP/1.1",
                                  501,
                                  "Request not handled",
                                  {{"Server", "Zia/1.0"},
                                   {"Content-Type", "text/html"},
                                   {"Connection", "keep-alive"},
                                   {"Location", "localhost"}}};

        // Call the Pipeline's modules start with REALLY_FIRST ones to
        // REALLY_LAST ones in order to handle the request
        for (int hookType = ziapi::REALLY_FIRST;
             hookType <= ziapi::REALLY_LAST; ++hookType)
            for (auto &it : _modules[static_cast<ziapi::HookType>(hookType)])
                it->handleRequest(req, res);
    }

    bool hook(std::shared_ptr<ziapi::IModule> module,
              ziapi::HookType type) final {
        if (type < ziapi::HookType::REALLY_FIRST ||
            type > ziapi::HookType::REALLY_LAST)
            return false;
        // Add the module to the Pipeline
        // while taking into account its hook type
        _modules[type].push_back(module);
        return true;
    }

    void unhook(std::shared_ptr<ziapi::IModule> module) final{};

  private:
    std::unordered_map<ziapi::HookType, Modules> _modules;
};

class ExampleModule : public ziapi::IModule,
                      public std::enable_shared_from_this<ExampleModule> {
  public:
    ExampleModule() = default;
    ~ExampleModule() = default;

  public:
    bool start(ziapi::IPipeline *pipeline) final {
        // Add self to the Pipeline
        return pipeline->hook(shared_from_this(), ziapi::HookType::FIRST);
    }

    bool stop() final {
        return true;
    };

    void handleRequest(const ziapi::Request &req,
                       ziapi::Response &res) final {
        res.status = 200;
        res.reason = "ExampleModule received the request";
        res.body = "<h2>Your Method: " + req.method + "</h2>\n";
        res.body += "<h2>Your HTTP Version: " + req.version + "</h2>\n";
        res.body += "<h2>Your URI: " + req.uri + "</h2>";
    }
};

class PrintResponseModule
    : public ziapi::IModule,
      public std::enable_shared_from_this<PrintResponseModule> {
  public:
    PrintResponseModule() = default;
    ~PrintResponseModule() = default;

  public:
    bool start(ziapi::IPipeline *pipeline) final {
        // Add self to the Pipeline, this module will be called after everthing
        return pipeline->hook(shared_from_this(),
                              ziapi::HookType::REALLY_LAST);
    }

    bool stop() final {
        return true;
    };

    void handleRequest(const ziapi::Request &req,
                       ziapi::Response &res) final {
        // Print out the response
        std::cout << res.version << " " << res.status << " " << res.reason
                  << std::endl;
        for (auto &pair : res.headers)
            std::cout << pair.first << ": " << pair.second << std::endl;
        std::cout << std::endl << res.body << std::endl;
    }
};

int main() {
    auto pipeline = new Pipeline();
    auto exampleModule = std::shared_ptr<ExampleModule>(new ExampleModule);
    auto printResponseModule =
        std::shared_ptr<PrintResponseModule>(new PrintResponseModule);
    // In this example we'll use a fake request
    ziapi::Request fakeRequest = {"OPTIONS", "*", "HTTP/1.1"};

    exampleModule->start(static_cast<ziapi::IPipeline *>(pipeline));
    printResponseModule->start(static_cast<ziapi::IPipeline *>(pipeline));

    pipeline->handleRequest(fakeRequest);
    return 0;
}