#include "IModule.hpp"
#include <iostream>
#include <vector>

/**
 * In this example, we will:
 *  - implement the Pipeline class without taking into account the hook type
 *  - implement an ExampleModule that modify the http response
 */

/**
 * Pipeline class
 * implements IPipeline functions
 */
class Pipeline : public ziapi::IPipeline {
  public:
    Pipeline() = default;
    ~Pipeline() = default;

  public:
    // Server's configurations are not handled in this example
    bool configure(const ziapi::Config &) final {
        return true;
    }

    void handleRequest(const ziapi::Request &req) final {
        // Create a default response
        ziapi::Response res = {"HTTP/1.1",
                               501,
                               "Request not handled",
                               {{"Server", "Zia/1.0"},
                                {"Content-Type", "text/html"},
                                {"Connection", "keep-alive"},
                                {"Location", "localhost"}}};

        // Call the Pipeline's modules to handle the request
        for (auto &it : modules) {
            it->handleRequest(req, res);
        }

        // Print out the response after handling
        std::cout << res.version << " " << res.status << " " << res.reason
                  << std::endl;
        for (auto &pair : res.headers)
            std::cout << pair.first << ": " << pair.second << std::endl;
        std::cout << std::endl << res.body << std::endl;
    }
    /**
     * !!!HookType is not handled in this example
     */
    bool hook(std::shared_ptr<ziapi::IModule> module, ziapi::HookType) final {
        // Add the module to the Pipeline
        modules.push_back(module);
        return true;
    }

    void unhook(std::shared_ptr<ziapi::IModule> module) final {
    }

  private:
    /**
     * !!!HookType is not handled in this example
     */
    std::vector<std::shared_ptr<ziapi::IModule>> modules;
};

/**
 * ExampleModule class
 */
class ExampleModule : public ziapi::IModule,
                      public std::enable_shared_from_this<ExampleModule> {
  public:
    ExampleModule() = default;
    ~ExampleModule() = default;

  public:
    // Server's configurations are not handled in this example
    bool start(ziapi::IPipeline *pipeline, const ziapi::Config &) final {
        // Add self to the Pipeline
        return pipeline->hook(shared_from_this());
    }

    bool stop() final {
        return true;
    }

    void handleRequest(const ziapi::Request &req, ziapi::Response &res) final {
        res.status = 200;
        res.reason = "ExampleModule received the request";
        res.body = "<h2>Your Method: " + req.method + "</h2>\n";
        res.body += "<h2>Your HTTP Version: " + req.version + "</h2>\n";
        res.body += "<h2>Your URI: " + req.uri + "</h2>";
    }
};

int main() {
    auto pipeline = new Pipeline();
    auto exampleModule = std::shared_ptr<ExampleModule>(new ExampleModule);
    // In this example we'll use a fake request
    ziapi::Request fakeRequest = {"OPTIONS", "*", "HTTP/1.1"};

    exampleModule->start(static_cast<ziapi::IPipeline *>(pipeline), {});

    pipeline->handleRequest(fakeRequest);
    return 0;
}