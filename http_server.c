#include "http_server.h"
#include "log.h"
#include <assert.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

#define STRINGS_MATCH 0
#define SERVER_LISTENNING_ERROR -15
#define LISTENED 0
#define BINEDED 0
#define SERVER_CREATION_ERROR -30
#define TIME_OUT_COUNTER_MAX 500
#define SET_BAD_REQUEST_INVALID "invalid"
#define LINE_READ_ERROR -1
#define GET_LINE_ERROR -2
#define ZERO_VALUE 0
#define SPACE_CHAR ' '
#define ARRAY_INIT_NUM 1000
#define SIZEOF_LONGEST_ACTION 10
#define LAB1_BUFFER_SIZE 1024
#define MAX_ACCEPTABLE_BUFFER_CAPACITY_PERCENTAGE 0.7
#define TWO_VALUE 2
#define ONE_VALUE 1
#define NULL_TERMINATOR '\0'
#define READ_FROM_STDIN '-'
#define LENGTH_OF_LONGEST_ACTION 10
#define HANDLE_RESPONSE_ERROR -13
#define UPPERCASE_CODE 0x1
#define LOWERCASE_CODE 0x2
#define TITLE_CASE_CODE 0x4
#define REVERSE_CODE 0x8
#define SHUFFLE_CODE 0x10
#define DYNAMIC_BUFFER_ERROR -33
#define RECEIVED_CHAR_ERROR -34
#define RESPONSE_ERROR -35
#define SENDING_ERROR -36
#define CONNECT_ERROR -37
#define SENT_COMPLETE 0
#define ZERO_RESET_INIT_VALUE 0


// Parses the options given to the program. It will return a Config struct with
// the necessary information filled in. argc and argv are provided by main. If
// an error occurs in processing the arguments and options (such as an invalid
// option), this function will print the correct message and then exit.
Config http_server_parse_arguments(int argc, char *argv[]) {

  Config myConfig;
  myConfig.port = "invalid";
  myConfig.relative_path = "invalid";
  bool stillParsing = true;
  int selectedOption = 0;
  int optionIndex = 0;
  bool portReceived = false;
  bool pathReceived = false;
  log_set_quiet(true);
  struct option long_opts[] = {{"help", no_argument, 0, 0},
                               {"verbose", no_argument, 0, 'v'},
                               {"port", required_argument, 0, 'p'},
                               {"folder", required_argument, 0, 'f'},
                               {0, 0, 0, 0}};

  while ((selectedOption = getopt_long(argc, argv, ":hvp:f:", long_opts,
                                       &optionIndex)) != -1) {

    while (stillParsing) {
      switch (selectedOption) {
      case 0:
        // printUsage();
        break;
      case 'h':
        log_trace("Providing help information");
        myConfig.port = "invalid";
        myConfig.relative_path = "invalid";
        return myConfig;
        break;
      case 'v':
        log_set_quiet(false);
        log_trace("Verbose flag activated\n");
        stillParsing = false;
        break;
      case 'p':
        log_trace("Port option chosen and now parsing parameter passed in");
        stillParsing = false;
        myConfig.port = optarg;
        portReceived = true;
        break;
      case 'f':
        myConfig.relative_path = optarg;
        log_trace("Folder option was chosen\n");
        pathReceived = true;
        stillParsing = false;
        DIR *dir = opendir(myConfig.relative_path);
        if (!dir || errno == ENOTDIR) {
          log_error("invlaid strating folder");
          myConfig.port = "invalid";
          myConfig.relative_path = "invalid";
          return myConfig;
        }
        closedir(dir);
        break;
      case '?':
        break;
      default:
        printf("Unkonwn argument provided\n");
      }
    }
  }
  // subtracting the options from the arguments to be left with th arguments
  // passed in
  argc -= optind;
  argv += optind;
  // Checking if port received or I need to assign the default ports
  if (!portReceived) {
    myConfig.port = HTTP_SERVER_DEFAULT_PORT;
  }
  if (!pathReceived) {
    myConfig.port = HTTP_SERVER_DEFAULT_RELATIVE_PATH;
  }
  return myConfig;
}

////////////////////////////////////////////////////
///////////// SOCKET RELATED FUNCTIONS /////////////
////////////////////////////////////////////////////

// Create and bind to a server socket using the provided configuration. A socket
// file descriptor should be returned. If something fails, a -1 must be
// returned.
int http_server_create(Config config) {
  log_trace("Creating the server\n");
  // Socket var
  int sockfd;
  // var to save the address of the server
  struct sockaddr_in servaddr;

  // socket create and verification
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    log_error("socket creation failed...\n");
    return SERVER_CREATION_ERROR;
  } else
    log_info("Socket successfully created..\n");
  bzero(&servaddr, sizeof(servaddr));

  // assign IP, PORT
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(atoi(config.port));

  // Binding newly created socket to given IP and verification
  if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) !=
      BINEDED) {
    log_error("socket bind failed...\n");
    return -1;
  } else {
    log_info("Socket successfully binded..\n");
  }
  // if this point was reached then the socket it binded and ready to connect
  return sockfd;
}

// Listen on the provided server socket for incoming clients. When a client
// connects, return the client socket file descriptor. This is a blocking call.
// If an error occurs, return a -1.
int tcp_server_accept(int socket) {
  log_trace("Server about to listen for incoming client connections");
  // var for the socket
  int finalSockfd;
  // Variable to recognize client
  struct sockaddr_in cli;
  // Now server is ready to listen and verification
  if ((listen(socket, 5)) != 0) {
    log_error("Listen failed...\n");
    return -1;
  } else
    log_info("Server listening..\n");
  socklen_t len = sizeof(cli);
  // Accept the data packet from client and verification
  finalSockfd = accept(socket, (struct sockaddr *)&cli, &len);

  if (finalSockfd < 0) {
    log_error("server acccept failed...\n");
    return -1;
  } else
    log_info("server acccept the client...\n");

  // server should be running and connected to a client on theis socket
  return finalSockfd; // FIX ME
}

// Listen on the provided server socket for incoming clients. When a client
// connects, return the client socket file descriptor. This is a blocking
// call. If an error occurs, return a -1.
int http_server_accept(int socket) {
  // var for the socket
  int finalSockfd;
  // Variable to recognize client
  struct sockaddr_in cli;
  // Now server is ready to listen and verification
  if ((listen(socket, 5)) != 0) {
    log_error("Listen failed...\n");
    return -1;
  } else
    log_info("Server listening..\n");
  socklen_t len = sizeof(cli);
  // Accept the data packet from client and verification
  finalSockfd = accept(socket, (struct sockaddr *)&cli, &len);

  if (finalSockfd < 0) {
    log_error("server acccept failed...\n");
    return -1;
  } else
    log_info("server acccept the client...\n");

  // server should be running and connected to a client on theis socket
  return finalSockfd; // FIX ME
}

// Read data from the provided client socket, parse the data, and return a
// Request struct. This function will allocate the necessary buffers to fill
// in the Request struct. The buffers contained in the Request struct must be
// freed using http_server_client_cleanup. If an error occurs, return an empty
// request and this function will free any allocated resources.
Request http_server_receive_request(int socket) {
  log_trace("Reading request");

  uint32_t timeOutCounter = ZERO_RESET_INIT_VALUE;
  // initalizing the request struct
  Request newRequest;
  newRequest.headers = NULL;
  newRequest.method = NULL;
  newRequest.num_headers = ZERO_RESET_INIT_VALUE;
  newRequest.path = NULL;
  // sentinal values used for checking for complete requests
  char *sentinal = "\r\n\r\n\0";

  size_t startingSize = LAB1_BUFFER_SIZE;

  char *dynamicBuffer = malloc(sizeof(char) * startingSize);

  size_t receivedAll = ZERO_RESET_INIT_VALUE;

  while (1) {
    if (receivedAll >=
        (MAX_ACCEPTABLE_BUFFER_CAPACITY_PERCENTAGE * startingSize)) {
      startingSize *= 2;
      dynamicBuffer = realloc(dynamicBuffer, sizeof(char) * startingSize);

      if (dynamicBuffer == NULL) {
        free(dynamicBuffer);
        log_error("Something went wrong with the buffer");
        newRequest.headers = NULL;
        newRequest.method = NULL;
        newRequest.num_headers = ZERO_RESET_INIT_VALUE;
        newRequest.path = NULL;
        return newRequest;
      }
    }

    int charsReceived =
        recv(socket, (dynamicBuffer + receivedAll),
             (startingSize - receivedAll), ZERO_RESET_INIT_VALUE);

    if (charsReceived == -1) {
      log_error("Read function had an error");
      free(dynamicBuffer);
        newRequest.headers = NULL;
        newRequest.method = NULL;
        newRequest.num_headers = ZERO_RESET_INIT_VALUE;
        newRequest.path = NULL;
      return newRequest;
    }

    receivedAll += charsReceived;
    dynamicBuffer[receivedAll] = NULL_TERMINATOR;

    if (strcmp(&dynamicBuffer[receivedAll - 4], sentinal) == STRINGS_MATCH) {
      log_trace("Received complete request");
      break;
    } else if (timeOutCounter >= 100000 &&
               charsReceived == ZERO_RESET_INIT_VALUE) { // did not receive
                                                         // anything for a while
      log_error("Connection timeout, now disconncing");
        newRequest.headers = NULL;
        newRequest.method = NULL;
        newRequest.num_headers = ZERO_RESET_INIT_VALUE;
        newRequest.path = NULL;
      return newRequest;
    } else {
      timeOutCounter++;
    }
    return http_server_parse_request(dynamicBuffer);
  }
}

// Sends the provided Response struct on the provided client socket.
int http_server_send_response(int socket, Response response) {
  log_trace("About to send back the response");

  char *myHeader = malloc(sizeof(char) * HTTP_SERVER_MAX_HEADER_SIZE);
  int headerLength =
      snprintf(NULL, ZERO_RESET_INIT_VALUE, "%s\r\n%s: %s\r\n\r\n",
               response.status, response.headers[ZERO_RESET_INIT_VALUE]->name,
               response.headers[ZERO_RESET_INIT_VALUE]->value);

  size_t totalSentChars = headerLength;
  size_t totalBytes =
      headerLength + atoi(response.headers[ZERO_RESET_INIT_VALUE]->value);


      int succesfullySent = ZERO_RESET_INIT_VALUE;
      int justSent = ZERO_RESET_INIT_VALUE;
      while (succesfullySent < totalBytes)
      {
        justSent = send(socket, response.headers, headerLength, NULL);
        succesfullySent += justSent;
        if( justSent == HTTP_SERVER_BAD_SOCKET)
        {
          return EXIT_FAILURE;
        }
      }
      justSent = ZERO_RESET_INIT_VALUE;
      succesfullySent = ZERO_RESET_INIT_VALUE;
      char* responseMem = malloc(sizeof(char) * HTTP_SERVER_FILE_CHUNK);

      size_t responseSize = ZERO_RESET_INIT_VALUE;

      while((responseSize = fread(responseMem, sizeof(char), HTTP_SERVER_FILE_CHUNK, response.file)))
      {
        totalSentChars += responseSize;

        while (succesfullySent < responseSize)
        {
          justSent = send(socket, responseMem, responseSize, NULL);
          justSent += succesfullySent;
                  if( justSent == HTTP_SERVER_BAD_SOCKET)
        {
          return EXIT_FAILURE;
        }
        }
        


      }
      return EXIT_SUCCESS;
}

// Closes the provided client socket and cleans up allocated resources.
void http_server_client_cleanup(int socket, Request request,
                                Response response) {}

// Closes provided server socket
void http_server_cleanup(int socket) {
  log_trace("Closing socket connection and cleaning up");

  int status = close(socket);
  if (status != ZERO_RESET_INIT_VALUE) {
    log_error("Server socket did not close successfully");
  }
  log_trace("Closed socket successfully");
}

////////////////////////////////////////////////////
//////////// PROTOCOL RELATED FUNCTIONS ////////////
////////////////////////////////////////////////////

// A helper function to be used inside of http_server_receive_request. This
// should not be used directly in main.c.
Request http_server_parse_request(char *buf) {
  log_trace("Parsing the request...");
  Request newRequest;
        newRequest.headers = NULL;
        newRequest.method = NULL;
        newRequest.num_headers = ZERO_RESET_INIT_VALUE;
        newRequest.path = NULL;
  char* context = NULL;
  char* header;
  int headerCount = 10;

  newRequest.headers = malloc(sizeof(Header*) * headerCount);
  strtok_r(buf, "\r\n", &context);
  char* localMethod = strtok(buf, " ");
  if(localMethod == NULL)
  {
    log_error("The header was invalid");
        newRequest.headers = NULL;
        newRequest.method = NULL;
        newRequest.num_headers = ZERO_RESET_INIT_VALUE;
        newRequest.path = NULL;
    return newRequest;
  }

  newRequest.method = malloc(HTTP_SERVER_MAX_HEADER_SIZE);
  memcpy(newRequest.method, localMethod, strlen(localMethod) + 1);

  char* localPath = strtok(NULL, " ");
  if(localMethod == NULL)
  {
    log_error("Path was invalid");
        newRequest.headers = NULL;
        newRequest.method = NULL;
        newRequest.num_headers =ZERO_RESET_INIT_VALUE;
        newRequest.path = NULL;
    return newRequest;
  }

  newRequest.path = malloc(HTTP_SERVER_MAX_HEADER_SIZE);
  memcpy(newRequest.path, localPath, strlen(localPath) + 1);

  char* currentHTTPVersion = strtok(NULL, " ");
  if(currentHTTPVersion == NULL)
  {
    log_error("The current http version is invalid");        
        newRequest.headers = NULL;
        newRequest.method = NULL;
        newRequest.num_headers = ZERO_RESET_INIT_VALUE;
        newRequest.path = NULL;
    return newRequest;
  }

  newRequest.num_headers = 1;

  while(1)
  {
    header = strtok_r(NULL, "\r\n", &context);

    if(header == NULL)
    {
      newRequest.num_headers--;
      log_info("Parsing done");
      return newRequest;
    }

    if(newRequest.num_headers == (headerCount - 2))
    {
      headerCount *= 2;
      newRequest.headers = realloc(newRequest.headers, sizeof(Header*) * headerCount);
    }
  }
}

// Convert a Request struct into a Response struct. Use relative_path to
// determine the path of the file being requested. This function will allocate
// the necessary buffers to fill in the Response struct. The buffers contained
// in the Resposne struct must be freeded using http_server_client_cleanup. If
// an error occurs, an empty Response will be returned and this function will
// free any allocated resources.
Response http_server_process_request(Request request, char *relative_path) {
  log_trace("About to process client request");

  Response newResponse;
  char *fPath;
  if (request.num_headers == -500) {
    newResponse.status = malloc(35);
    memcpy(newResponse.status, "HTTP/1.1 500 Unternal Server Error", 34);
    newResponse.status[34] = NULL_TERMINATOR;
    newResponse.file = fopen("www/500.html", "r");
  }
  if (request.num_headers == -400) {
    newResponse.status = malloc(25);
    memcpy(newResponse.status, "HTTP/1.1 400 Bad Request", 24);
    newResponse.status[24] = NULL_TERMINATOR;

    newResponse.file = fopen("www/400.html", "r");
  }
  if (request.method != NULL) {
    size_t lengthOfFilePath = strlen(relative_path) + strlen(request.path);
    fPath = malloc(sizeof(char) * (lengthOfFilePath + 1));
    memcpy(fPath, relative_path, strlen(relative_path));
    memcpy(fPath[strlen(relative_path)], request.path, strlen(request.path));
    fPath[lengthOfFilePath] = NULL_TERMINATOR;

    struct stat myStat;

    if (stat(fPath, &myStat) == ZERO_RESET_INIT_VALUE) {
      if (myStat.st_mode & S_IFDIR) {
        log_error("This is a directeory not a file");
        newResponse.status = malloc(23);
        memcpy(newResponse.status, "HTTP/1.1 403 Forbidden", 22);
        newResponse.status[22] = NULL_TERMINATOR;

        newResponse.file = fopen("www/403.html", "r");
      } else if (myStat.st_mode & S_IFREG) {
        newResponse.file = fopen(fPath, "r");
        if (newResponse.file == NULL) {
          newResponse.status = malloc(23);
          memcpy(newResponse.status, "HTTP/1.1 404 Not Found", 22);
          newResponse.status[22] = NULL_TERMINATOR;

          newResponse.file = fopen("www/404.html", "r");
        } else if (strcmp(request.method, "GET") == STRINGS_MATCH) {
          newResponse.status = malloc(16);
          memcpy(newResponse.status, "HTTP/1.1 200 Ok", 15);
          newResponse.status[15] = NULL_TERMINATOR;
        } else {
          newResponse.status = malloc(32);
          memcpy(newResponse.status, "HTTP/1.1 405 Method Not Allowed", 31);
          newResponse.status[31] = NULL_TERMINATOR;

          newResponse.file = fopen("www/405.html", "r");
        }
      } else {
        log_error("something failed %s", strerror(errno));

        newResponse.status = malloc(23);
        memcpy(newResponse.status, "HTTP/1.1 404 Not Found", 22);
        newResponse.status[22] = NULL_TERMINATOR;

        newResponse.file = fopen("www/404.html", "r");
      }
    }
    size_t fLen = ZERO_RESET_INIT_VALUE;
    if (newResponse.file != NULL) {
      fseek(newResponse.file, 0L, SEEK_END);
      fLen = ftell(newResponse.file);
      rewind(newResponse.file);
    }

    newResponse.num_headers = 1;

    newResponse.headers = malloc(sizeof(Header));
    newResponse.headers[ZERO_RESET_INIT_VALUE]->name =
        malloc(sizeof(char) * 15);
    memcpy(newResponse.headers[ZERO_RESET_INIT_VALUE]->name, "Content-Length",
           14);
    newResponse.headers[ZERO_RESET_INIT_VALUE]->name[14] = NULL_TERMINATOR;

    size_t contentLen = snprintf(NULL, 0, "%ld", fLen);
    newResponse.headers[ZERO_RESET_INIT_VALUE]->value =
        malloc(sizeof(char) * (contentLen + 1));

    snprintf(newResponse.headers[ZERO_RESET_INIT_VALUE]->value, contentLen + 1,
             "u", fLen);
  }

  return newResponse;
}

void printUsage() {
  printf("Usage: http_server [--help] [-v] [-p PORT] [-f FOLDER]\n\n");
  printf("Options:\n");
  printf("--help\n");
  printf("-v, --verbose\n");
  printf("--port PORT, -p PORT\n");
  printf("--folder FOLDER, -f FOLDER\n");
}