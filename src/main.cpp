#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>
#include <queue>
#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>

#include "ApiHandler.h"
#include "InvertedIndex.h"
#include "Ollama.h"
#include "SearchSystem.h"
#include "ThreadPool.h"
#include "WebSocketSession.h"


#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
#endif

#define PORT 8080

using namespace std;


int main() {
    
    return 0;
}