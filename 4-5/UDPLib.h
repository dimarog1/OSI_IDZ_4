#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

void closeAll();

void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

void handleSigInt(int sig)
{
    if (sig != SIGINT)
    {
        return;
    }
    printf("\n\nFINISH USING SIGINT\n\n");
    closeAll();
    exit(0);
}

enum REQUEST_CODE
{
    WAIT_CALL = 0,
    MAKE_CALL = 1,
    END_CALL = 2
};

enum RESPONSE_CODE
{
    UB = -1,
    CALL_ACCEPTED = 0,
    NO_ANSWER = 1,
    END = 2,
    NO_CALL = 3,
    CALL_RECEIVED = 4,
    FINISH = 5
};

struct call
{
    int id;
    int caller_id;
    int receiver_id;
};

struct request
{
    int request_code;
    int yapper_id;

    struct call call;
};

struct response
{
    int response_code;
    struct call call;
};