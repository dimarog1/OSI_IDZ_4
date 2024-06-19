#include "UDPLib.h"
#include <time.h>

int yapper_id, sock;

void closeAll()
{
    printf("\n\nFINISH USING SIGINT\n\n");
    close(sock);
}

int chooseState()
{
    srand(time(NULL));
    return rand() % 2;
}

int getOtherYapperNumber()
{
    int yapper = yapper_id - 1;
    while (yapper == yapper_id - 1)
    {
        yapper = rand() % 5;
    }
    return yapper;
}

int main(int argc, char *argv[])
{
    (void)signal(SIGINT, handleSigInt);

    struct sockaddr_in servaddr;
    char *ip;                // IP broadcast address
    unsigned short port;     // server port
    int broadcastPermission; // socket opt to set permission to broadcast

    if (argc < 4)
    {
        fprintf(stderr, "Usage: %s <IP Address> <Port> <Yapper ID>\n", argv[0]);
        exit(1);
    }
    ip = argv[1];
    port = atoi(argv[2]);
    int yapper_id = atoi(argv[3]);

    // create socket with UDP options
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == -1)
    {
        DieWithError("socket() failed");
    }

    // set socket to allow broadcast
    broadcastPermission = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *)&broadcastPermission, sizeof(broadcastPermission)) < 0)
    {
        DieWithError("setsockopt() failed");
    }

    // setup local address structure
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(ip);

    struct call call = {yapper_id - 1, yapper_id, 1 + rand() % 5};
    struct request request = {2, 0, call};

    while (1)
    {
        if (request.request_code == END_CALL)
        {
            if (chooseState())
            {
                call.id = yapper_id - 1;
                call.caller_id = yapper_id - 1;
                call.receiver_id = getOtherYapperNumber();
                request.yapper_id = yapper_id - 1;
                request.request_code = 1;
                request.call = call;
                printf("Yapper  #%d is trying to call %d from server\n", yapper_id, call.receiver_id + 1);
            }
            else
            {
                call.id = yapper_id - 1;
                call.caller_id = -1;
                call.receiver_id = yapper_id - 1;
                request.yapper_id = yapper_id - 1;
                request.request_code = 0;
                request.call = call;
                printf("Yapper  #%d is waiting calls from server\n", yapper_id);
            }
        }

        if (sendto(sock, &request, sizeof(struct request), 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) != sizeof(request))
        {
            DieWithError("sendto() failed");
        }
        printf("Yapper #%d has sent his request = %d to server\n", yapper_id, request.request_code);

        struct response response = {-1, -1, -1, -1};
        if (recv(sock, &response, sizeof(response), 0) < 0)
        {
            DieWithError("recv() bad");
        }
        printf("Yapper  #%d has got the response = %d from server\n", yapper_id, response.response_code);

        if (response.response_code == FINISH)
        {
            break;
        }
        switch (response.response_code)
        {
        case END:
            break;
        case NO_ANSWER:
            sleep(rand() % 3);
            call.receiver_id = getOtherYapperNumber();
            request.call = call;
            request.request_code = MAKE_CALL;
            printf("Yapper  #%d is trying to call %d from server\n", yapper_id, call.receiver_id + 1);
            break;
        case CALL_ACCEPTED:
            sleep(rand() % 3);
            printf("!!!This Yapper talking to Yapper recv #%d \n", response.call.receiver_id + 1);
            request.call = response.call;
            request.request_code = END_CALL;

            if (sendto(sock, &request, sizeof(struct request), 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) != sizeof(request))
            {
                DieWithError("sendto() failed");
            }
            printf("Yapper #%d has sent his request = %d to server\n", yapper_id, request.request_code);

            struct response response = {-1, -1, -1, -1};
            if (recvfrom(sock, &response, sizeof(struct response), 0, NULL, NULL) != sizeof(response))
            {
                DieWithError("recvfrom() failed");
            }

            break;
        case CALL_RECEIVED:
            sleep(rand() % 3);
            printf("!!!This Yapper talking to Yapper caller #%d \n", response.call.receiver_id + 1);
            request.call = response.call;
            request.request_code = END_CALL;

            if (sendto(sock, &request, sizeof(struct request), 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) != sizeof(request))
            {
                DieWithError("sendto() failed");
            }
            printf("Yapper #%d has sent his request = %d to server\n", yapper_id, request.request_code);

            if (recv(sock, &response, sizeof(response), 0) < 0)
            {
                DieWithError("recv() bad");
            }

            break;
        case NO_CALL:
            request.request_code = WAIT_CALL;
            break;
        }
        sleep(5);
    }

    close(sock);
    exit(0);
}