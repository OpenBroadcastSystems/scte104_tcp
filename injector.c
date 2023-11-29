/* SCTE 104 TCP injector examplec

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

#include <poll.h>

#include <bitstream/scte/104.h>

#define PORT (5000)

static int receive_scte104_packet(int connfd)
{
    uint8_t msg[500];
    int len = recv(connfd, (void *)msg, 500, 0);
    if(len < 0) {
        fprintf(stderr, "error receiving \n");
        return -1;
    }

    if (!scte104s_validate(msg, len)) {
        fprintf(stderr, "invalid scte104 message \n");
        return -1;
    }

    uint16_t size = 0;
    uint8_t *p = scte104s_get_data(msg, &size);
    if(scte104o_get_opid(p) == SCTE104_OPID_INIT_REQUEST_DATA) {
        uint8_t msg_number = 0;
        memset(msg, 0, sizeof(msg));
        len = SCTE104S_HEADER_SIZE;

        scte104_set_opid(msg, SCTE104_OPID_INIT_RESPONSE_DATA);
        scte104_set_size(msg, len);
        scte104s_set_result(msg, 100);
        scte104s_set_result_extension(msg, 0xffff);
        scte104s_set_protocol(msg, 0);
        scte104s_set_as_index(msg, 0);
        scte104s_set_message_num(msg, msg_number);
        scte104s_set_dpi_pid_index(msg, 0);

        if( send(connfd, msg, len, 0) < 0)
            fprintf( stderr, "Failed to send scte 104 reply \n" );
    }

    return 0;
}

int main(int argc, char *argv[]) 
{
    int sockfd, connfd, len;
    struct sockaddr_in listen_addr, client;

    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd < 0) {
        fprintf(stderr, "socket creation failed \n");
        return -1;    
    }

    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    listen_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr*)&listen_addr, sizeof(listen_addr)) < 0) {
        fprintf(stderr, "bind() failed \n");
        perror("fail ");
        return -1;
    }

    if (listen(sockfd, 5) < 0) {
        fprintf(stderr, "listen() failed \n");
        return -1;
    }

    len = sizeof(client);
    connfd = accept(sockfd, (struct sockaddr*)&client, &len);
    if (connfd < 0) {
        fprintf(stderr, "accept() failed \n");
        return -1;
    }

    struct pollfd fds[1];
    fds[0].fd = connfd;
    fds[0].events = POLLIN;

    while (poll(fds, 1, 10000)) {
        if(fds[0].revents & POLLIN) {
            receive_scte104_packet(connfd);
            fds[0].events = 0;
        }
    }

    close(sockfd);

    return 0;
}