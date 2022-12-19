easywsclient
============

It was forked from https://github.com/dhbaird/easywsclient.
Detailed introduction and usage can be found on the above site.

David Baird's easywsclient is a good code, but from my experience it needs some improvement.

- Functions such as send,sendBinary, sendBinary, sendData that send data are void types. It is recommended that these functions be returned as many bytes as sent.
- The sending function does not need to be asynchronous.
- Finally, in the original source code, the close function does not close the socket. It only sends a close frame packet to the server and wait for termination. Therefore, if you clear the websocet locally before shutting down on the server, the socket does not close. Therefore, if the socket is not closed in the destructor, it is recommended to close it.

<br><br>

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

<br><br>
Add destructor for safe socket closing

```c++
int is_valid_fd(int fd)
{
    return fcntl(fd, F_GETFL) != -1 || errno != EBADF;
}

......

//in the destructor, if socket is opend , close it for preventing resource leakage
~_RealWebSocket(){
    if(is_valid_fd(sockfd)) closesocket(sockfd);
}

```
<br><br>
send function send packet immediately, so in the poll function, select does not use write fd

```c++
// Original 
select(sockfd + 1, &rfds, &wfds, 0, timeout > 0 ? &tv : 0);

// Modified
int ret = select(sockfd + 1, &rfds, NULL, 0, timeout > 0 ? &tv : 0);
if(0 == ret){
    return 0;   //No data to process 
}
else if(-1 == ret ){    //error
    return -1;
}

// I don't need this code anymore.
/*
while (txbuf.size()) {
    int ret = ::send(sockfd, (char*)&txbuf[0], txbuf.size(), 0);
    if (false) { } // ??
    else if (ret < 0 && (socketerrno == SOCKET_EWOULDBLOCK || socketerrno == SOCKET_EAGAIN_EINPROGRESS)) {
        break;
    }
    else if (ret <= 0) {
        closesocket(sockfd);
        readyState = CLOSED;
        fputs(ret < 0 ? "Connection error!\n" : "Connection closed!\n", stderr);
        break;
    }
    else {
        txbuf.erase(txbuf.begin(), txbuf.begin() + ret);
    }
}
*/

```
<br><br>

The senddata function sends the packet immediately and return the sent bytes.
```c++
// Original 
        ........
        ........
        // N.B. - txbuf will keep growing until it can be transmitted over the socket:
        txbuf.insert(txbuf.end(), header.begin(), header.end());
        txbuf.insert(txbuf.end(), message_begin, message_end);
        if (useMask) {
            size_t message_offset = txbuf.size() - message_size;
            for (size_t i = 0; i != message_size; ++i) {
                txbuf[message_offset + i] ^= masking_key[i&0x3];
            }
        }

// Modified
        ........
        ........
        // N.B. - txbuf will keep growing until it can be transmitted over the socket:
        txbuf.insert(txbuf.end(), header.begin(), header.end());
        txbuf.insert(txbuf.end(), message_begin, message_end);
        if (useMask) {
            size_t message_offset = txbuf.size() - message_size;
            for (size_t i = 0; i != message_size; ++i) {
                txbuf[message_offset + i] ^= masking_key[i&0x3];
            }
        }
        // New added code 
        // Send the data synchronous (not asynchronous)
        int sum = 0;
        while (txbuf.size()){
            int ret = ::send(sockfd, (char*)&txbuf[0], txbuf.size(), 0);
            if (ret <= 0) {
                closesocket(sockfd);
                readyState = CLOSED;
                fputs(ret < 0 ? "sendData Connection error!\n" : "sendData Connection closed!\n", stderr);
            }
            else {
                txbuf.erase(txbuf.begin(), txbuf.begin() + ret);
                sum += ret;
                //fprintf(stderr, "WebSock send[%d] bytes\n", ret);
            }
        }
        return sum;

```

<br><br>

The close function immediately closes the socket after sending a close frame to the server.
```c++
// Original 
   void close() {
        if(readyState == CLOSING || readyState == CLOSED) { return; }
        readyState = CLOSING;
        uint8_t closeFrame[6] = {0x88, 0x80, 0x00, 0x00, 0x00, 0x00}; // last 4 bytes are a masking key
        std::vector<uint8_t> header(closeFrame, closeFrame+6);
        txbuf.insert(txbuf.end(), header.begin(), header.end());
    }

// Modified
    void close() {
        if(readyState == CLOSING || readyState == CLOSED) { return; }
        readyState = CLOSING;
        uint8_t closeFrame[6] = {0x88, 0x80, 0x00, 0x00, 0x00, 0x00}; // last 4 bytes are a masking key
        std::vector<uint8_t> header(closeFrame, closeFrame+6);
        txbuf.insert(txbuf.end(), header.begin(), header.end());
        int ret = ::send(sockfd, (char*)&txbuf[0], txbuf.size(), 0);
        txbuf.erase(txbuf.begin(), txbuf.begin() + ret);

    }

```