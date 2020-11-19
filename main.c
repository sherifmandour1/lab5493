#include "http_server.h"
#include "log.h"
#include <signal.h>
#include <stdio.h>

#define STRINGS_MATCH 0
static int mainSocket;

void serverHandler()
{
    log_trace("server interreupteda and shutting down");
    signal(SIGINT, serverHandler);
    log_trace("Server is now taken down");
    
    exit(EXIT_SUCCESS);
}


int main(int argc, char* argv[])
{
    Config mainConfig = http_server_parse_arguments(argc, argv);

    if(mainConfig.port == NULL && mainConfig.relative_path == NULL)
    {
        log_error("Config was empty, exiting now");
        return EXIT_FAILURE;
    }
     if (strcmp(mainConfig.port, "invalid") == STRINGS_MATCH && strcmp(mainConfig.relative_path, "invalid") == STRINGS_MATCH)
    {
        log_error("Config contents were invalid, now exiting");
        return EXIT_SUCCESS;
    }
    
    mainSocket = http_server_create(mainConfig);

    if(mainSocket == HTTP_SERVER_BAD_SOCKET)
    {
        
        return EXIT_FAILURE;
    }

    signal(SIGINT, serverHandler);

    while(1)
    {
        int clientSocket = http_server_accept(mainSocket);

        if(clientSocket == HTTP_SERVER_BAD_SOCKET)
        {
            continue;
        }

        Request newRequest = http_server_receive_request(clientSocket);

        Response newResponse = http_server_process_request(newRequest, mainConfig.relative_path);

        if(newResponse.status == NULL && newResponse.file == NULL && newResponse.num_headers == -404 && newResponse.headers == NULL)
        {
            
            return EXIT_FAILURE;
        }

        int sendResponse = http_server_send_response(clientSocket, newResponse);

        if(sendResponse != EXIT_SUCCESS)
        {
            
            return EXIT_FAILURE;
        }

        http_server_client_cleanup(clientSocket, newRequest, newResponse);

    }
    http_server_cleanup(mainSocket);
    return EXIT_SUCCESS;


}


