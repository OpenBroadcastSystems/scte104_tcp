/* SCTE 104 TCP automation examplec

Copyright (c) 2020 Open Broadcast Systems Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. 
*/

#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

#include <bitstream/scte/104.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 5000

int main(int argc, char *argv[]) 
{
    uint8_t msg[500];
    uint8_t msg_number = 0;
    int len = SCTE104S_HEADER_SIZE;

    scte104_set_opid(msg, SCTE104_OPID_INIT_REQUEST_DATA);
    scte104_set_size(msg, len);
    scte104s_set_result(msg, 100);
    scte104s_set_result_extension(msg, 0xffff);
    scte104s_set_protocol(msg, 0);
    scte104s_set_as_index(msg, 0);
    scte104s_set_message_num(msg, msg_number);
    scte104s_set_dpi_pid_index(msg, 0);

    struct sockaddr_in server_addr;
    int sockfd;

    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd < 0) {
        fprintf(stderr, "socket creation failed \n");
        return -1;    
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT);

    if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "connect() failed \n");
        return -1;
    }
    
    struct pollfd fds[1];
    fds[0].fd = sockfd;
    fds[0].events = POLLOUT;

    while (poll(fds, 1, 10000)) {
        if(fds[0].revents & POLLOUT) {
            int ret = send(sockfd, msg, len, 0);
            fds[0].events = 0;
        }
    }

    return 0;
}