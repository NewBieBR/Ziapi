#include "IModule.hpp"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sstream>
#include <fstream>

/**
 * In this example, we will:
 *  - implement a mini server in order to receive real http request
 *  - implement the Pipeline class while handling the hook type
 *  - implement an ExampleModule that will serve a simple html file
 * Launch the program and visit localhost:4242 with your browser
 * !!! To simplify the example, the server is single stream
 * Server code source: http://www.linuxhowtos.org/data/6/server.c
 */

// The client socket file descriptor
int newsockfd;

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
        for (int hookType = ziapi::REALLY_FIRST; hookType <= ziapi::REALLY_LAST;
             ++hookType)
            for (auto &it : _modules[static_cast<ziapi::HookType>(hookType)])
                it->handleRequest(req, res);
        sendResponse(res);
    }

    void sendResponse(ziapi::Response res) {
        std::string rawResponse;
        int n;
        rawResponse += res.version + " " + std::to_string(res.status) + " " +
                       res.reason + "\n";
        for (auto &pair : res.headers)
            rawResponse += pair.first + ": " + pair.second + "\n";
        rawResponse += "\n" + res.body;
        n = write(newsockfd, rawResponse.c_str(), rawResponse.size());
        if (n < 0) error("ERROR writing to socket");
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

    void handleRequest(const ziapi::Request &req, ziapi::Response &res) final {
        res.status = 200;
        std::ifstream ifs("index.html");
        std::string content((std::istreambuf_iterator<char>(ifs)),
                            (std::istreambuf_iterator<char>()));
        res.body = content;
    }
};

static void error(const char *msg) {
    perror(msg);
    exit(1);
}

static int launchServer() {
    int sockfd, portno;
    struct sockaddr_in serv_addr, cli_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");
    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = 4242;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    listen(sockfd, 5);
    return sockfd;
}

static int acceptClient(int sockfd, char *buffer) {
    socklen_t clilen;
    struct sockaddr_in cli_addr;
    int n;

    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0) error("ERROR on accept");
    bzero(buffer, 256);
    n = read(newsockfd, buffer, 255);
    if (n < 0) error("ERROR reading from socket");
    return newsockfd;
}

ziapi::Request createRequest(const std::string &rawRequest) {
    std::istringstream iss(rawRequest);
    std::string method, uri, version;
    iss >> method >> uri >> version;
    ziapi::Request req = {method, uri, version};
    return req;
}

int main() {
    auto pipeline = new Pipeline();
    auto exampleModule = std::shared_ptr<ExampleModule>(new ExampleModule);
    int sockfd;
    char buffer[256];

    sockfd = launchServer();
    newsockfd = acceptClient(sockfd, buffer);
    // Print client's request
    std::cout << buffer << std::endl;
    std::string rawRequest(buffer);
    auto request = createRequest(rawRequest);
    exampleModule->start(static_cast<ziapi::IPipeline *>(pipeline));
    pipeline->handleRequest(request);
    while (1)
        ;
    close(newsockfd);
    close(sockfd);
    return 0;
}