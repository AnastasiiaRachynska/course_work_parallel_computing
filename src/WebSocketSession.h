#ifndef COURSE_WORK_PARALLEL_COMPUTING_WEBSOCKETSESSION_H
#define COURSE_WORK_PARALLEL_COMPUTING_WEBSOCKETSESSION_H

#include "SearchSystem.h"
#include "ThreadPool.h"
#include <ixwebsocket/IXWebSocketServer.h>
using namespace std;

class WebSocketSession {
public:
    static bool start(ix::WebSocketServer& server, SearchSystem& search_system, ThreadPool& thread_pool);
};

#endif