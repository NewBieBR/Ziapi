# Ziapi

This is a simple Zia' API.

[Concept explanation](https://epitechfr-my.sharepoint.com/:p:/g/personal/hung_dao-nguyen_epitech_eu/EWejVtEkfnZOiK3fuA-CgXUBjUrfPaUUGrIZt5COcq9qbQ?e=cgnn5o)

### To create a module
```cpp
class ExampleModule : public ziapi::IModule {
  public:
    // Server's configurations are not handled in this example
    bool start(ziapi::IPipeline *pipeline, const ziapi::Config &) final {
        // Add this module to the Pipeline and define its hook type to
        // REALLY_FIRST
        return pipeline->hook(std::shared_ptr<ExampleModule>(this),
                              ziapi::HookType::REALLY_FIRST);
    }

    bool stop() final {
        return true;
    }

    void handleRequest(const ziapi::Request &req, ziapi::Response &res) final {
        res.status = 200;
        res.reason = "This module is called before anything";
    }
};
```

### To load a module
```cpp
auto pipeline = new Pipeline();
auto exampleModule = std::shared_ptr<ExampleModule>(new ExampleModule);
exampleModule->start(pipeline, {});
```

### To handle a request
```cpp
pipeline->handleRequest(request);
```

## HookType
HookType is used to define the modules' calling order

| HookType  | Description |
| ------------- | ------------- |
| REALLY_FIRST  | Modules with this HookType will be called before ANYTHING  |
| FIRST  | Modules with this HookType will be called first  |
| MIDDLE  | This HookType should be used by Modules that don't care when they are called  |
| LAST  | Modules with this HookType will be called last  |
| REALLY_LAST  | Modules with this HookType will be called after EVERYTHING  |

## Request
```cpp
struct Request {
    std::string method;
    std::string uri;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
};
```

## Response
```cpp
struct Response {
    std::string version;
    int status;
    std::string reason;
    std::map<std::string, std::string> headers;
    std::string body;
};
```
