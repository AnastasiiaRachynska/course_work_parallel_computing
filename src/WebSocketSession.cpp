#include "WebSocketSession.h"
#include <iostream>
#include <memory>
#include <ixwebsocket/IXWebSocketServer.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

using namespace std;


void send_error(ix::WebSocket& client, const string& error) {
    json message = {{"stage", "error"}, {"data", {{"error", error}}}};
    client.send(message.dump());
}

bool WebSocketSession::start( ix::WebSocketServer& server, SearchSystem& searchSystem, ThreadPool& threadPool) {
    
    server.setOnConnectionCallback([&searchSystem, &threadPool](
        weak_ptr<ix::WebSocket> weakClient,shared_ptr<ix::ConnectionState>) {
            
        auto client = weakClient.lock();
        if (!client) return;

        client->setOnMessageCallback([weakClient, &searchSystem, &threadPool](const ix::WebSocketMessagePtr& message) {
            auto client = weakClient.lock();
            if (!client) return;

            if (message->type == ix::WebSocketMessageType::Open) {
                if (message->openInfo.uri != "/ws/search") client->close();
                    return;
                }

            if (message->type != ix::WebSocketMessageType::Message) return;

            json request = json::parse(message->str, nullptr, false);

            if (message->binary || !request.contains("query") ||!request["query"].is_string()) {
                send_error(*client, "Expected JSON with a string field 'query'");
                return;
            }

            string query = request["query"].get<string>();

            threadPool.add_task([weakClient, query, &searchSystem]() {

                auto client = weakClient.lock();
                if (!client || client->getReadyState() != ix::ReadyState::Open) return;

                json result = searchSystem.search_stream(query, *client);
                            
                if (result.contains("error")) send_error(*client, result["error"].get<string>());
            });
        });
    });

    if (!server.listenAndStart()) {
        cout << "Failed to start WebSocket server" << endl;
        return false;
    }

    cout << "WebSocket listening on port 8081" << endl;
    return true;
}