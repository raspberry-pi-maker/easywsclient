easywsclient
============

It was copied from https://github.com/dhbaird/easywsclient.
Detailed introduction and usage can be found on the above site.

David Baird's easywsclient is a good code, but from my experience it needs some improvement.

- Functions such as Send and SendBinary that send data are void types. It is recommended that these functions be returned as many bytes as sent.
- The sending function send does not need to be asynchronous.
- Finally, the close function does not close the socket. It only sends a packet to the server to terminate the websocket. Therefore, if you clear the websocet locally before shutting down on the server, the socket does not close. Therefore, if the socket is not closed in the destructor, it is recommended to close it.

<br>

Modification
=====

return type modification:

```c++
    // Original return type(void)
    void poll(int timeout = 0);
    void send(const std::string& message);
    void sendBinary(const std::string& message);
    void sendBinary(const std::vector<uint8_t>& message);
    void sendPing();

    // New return type (int) which is the return value of send function
    int poll(int timeout = 0);
    int send(const std::string& message);
    int sendBinary(const std::string& message);
    int sendBinary(const std::vector<uint8_t>& message);
    int sendPing();

```




Put together, the usage looks like this:

```c++
#include "easywsclient.hpp"
//#include "easywsclient.cpp" // <-- include only if you don't want compile separately

int
main()
{
    ...
    using easywsclient::WebSocket;
    WebSocket::pointer ws = WebSocket::from_url("ws://localhost:8126/foo");
    assert(ws);
    while (true) {
        ws->poll();
        ws->send("hello");
        ws->dispatch(handle_message);
        // ...do more stuff...
    }
    ...
    delete ws; // alternatively, use unique_ptr<> if you have C++11
    return 0;
}
```

Example
=======

    # Launch a test server:
    node example-server.js

    # Build and launch the client:
    g++ -c easywsclient.cpp -o easywsclient.o
    g++ -c example-client.cpp -o example-client.o
    g++ example-client.o easywsclient.o -o example-client
    ./example-client

    # ...or build and launch a C++11 client:
    g++ -std=gnu++0x -c easywsclient.cpp -o easywsclient.o
    g++ -std=gnu++0x -c example-client-cpp11.cpp -o example-client-cpp11.o
    g++ example-client-cpp11.o easywsclient.o -o example-client-cpp11
    ./example-client-cpp11

    # Expect the output from example-client:
    Connected to: ws://localhost:8126/foo
    >>> galaxy
    >>> world

Threading
=========

This library is not thread safe. The user must take care to use locks if
accessing an instance of `WebSocket` from multiple threads. If you need
a quick threading library and don't have Boost or something else already,
I recommend [TinyThread++](http://tinythreadpp.bitsnbites.eu/).

Future Work
===========

(contributions appreciated!)

* Parameterize the `pointer` type (especially for `shared_ptr`).
* Support optional integration on top of an async (event-driven) library,
  especially Asio.
