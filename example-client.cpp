#include "easywsclient.hpp"
//#include "easywsclient.cpp" // <-- include only if you don't want compile separately
#ifdef _WIN32
#pragma comment( lib, "ws2_32" )
#include <WinSock2.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <string>

using easywsclient::WebSocket;
static WebSocket::pointer ws = NULL;

void handle_message(const std::string & message)
{
    printf(">>> %s\n", message.c_str());
    if (message == "world") { ws->close(); }
}

int main()
{
    int ret;
#ifdef _WIN32
    INT rc;
    WSADATA wsaData;

    rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (rc) {
        printf("WSAStartup Failed.\n");
        return 1;
    }
#endif

    ws = WebSocket::from_url("ws://localhost:8126/foo");
    assert(ws);
    ret = ws->send("goodbye");
    printf("ws send [%d] bytes\n", ret);
    ret = ws->send("hello");
    printf("ws send [%d] bytes\n", ret);
    int count = 0;
    while (ws->getReadyState() != WebSocket::CLOSED) {
      ret = ws->poll(1000);
      count += 1;
      if(count > 100){
        ws->close();    
         break;    //If you want to go out first,
      }
      if (!ret) continue;
      ws->dispatch(handle_message);
    }
    delete ws;                  //if you don't close the ws, delete will safely close the socket now.
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
