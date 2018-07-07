#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>

#define BUF_SIZE 4096


static void die(const char *msg) {
    perror(msg);
    exit(1);
}

static void printUsage() {
    fprintf(stderr, "usage: http-server  <server_port> <web_root> <mdb-lookup-host> <mdb-lookup-port>\n");
    fprintf(stderr, "ex) http-server 8888 ~/html localhost 9999\n");
    exit(1);
}

int main(int argc, char **argv) {

    // ignore SIGPIPE so we don't terminate when we call send() on a
    // disconnected socket
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) 
        die("signal() failed");

    int serverPort; // port for web server
    char *webRoot; // top level directory for HTML files
    char *mdbHost; // mdb-lookup
    int mdbPort; // port number for mdb-lookup
    int servsock; // Socket descriptor for server
    int clntsock; // Socket descriptor for client
    char *serverIP; // IP for MDB lookup

    // parse args
    serverPort = atoi(argv[1]); // local port
    webRoot = argv[2]; // top level directory
    mdbHost = argv[3];
    mdbPort = atoi(argv[4]);

    if (argc != 5) {
        printUsage();   
    }

    /* Part 2 */

    int mdbsock;
    struct hostent *he;

    // create socket
    if ((mdbsock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        die("socket failed");
    }


    // get server ip from server name
    if ((he = gethostbyname(mdbHost)) == NULL) {
        die("gethostbyname failed");
    }

    serverIP = inet_ntoa(*(struct in_addr *)he->h_addr);

    // Construct MDB address structure
    struct sockaddr_in mdbaddr; // local address
    memset(&mdbaddr, 0, sizeof(mdbaddr)); // zero out structure
    mdbaddr.sin_family = AF_INET; // internet address family
    mdbaddr.sin_addr.s_addr = inet_addr(serverIP); // any network interface
    mdbaddr.sin_port = htons(mdbPort);

    // connect to MDB server as a client
    if (connect(mdbsock, (struct sockaddr *)&mdbaddr, sizeof(mdbaddr)) < 0) {
        die("connect failed");
    }

    /* Part 2 End  */

    // Create socket for inc connections as server
    if ((servsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        die("socket failed");

    // Construct local address structure
    struct sockaddr_in servaddr; // local address
    memset(&servaddr, 0, sizeof(servaddr)); // zero out structure
    servaddr.sin_family = AF_INET; // internet address family
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // any network interface
    servaddr.sin_port = htons(serverPort);

    // Bind to the local address
    if (bind(servsock, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
        die("bind failed");

    // Mark socket so it will listen for incoming connections
    if (listen(servsock, 5) < 0)
        die("listen failed");

    socklen_t clntlen;
    struct sockaddr_in clntaddr;

    char request[100];
    char buf[4096];
    char *token_sep = "\t \r\n"; // tab, space, new line
    struct stat st;

    // Error and response messages
    char *not_implemented = "501 Not Implemented\n\n"
        "<html><body><h1>501 Not Implemented</h1></body></html>\n";

    char *bad_req = "400 Bad Request\n\n"
        "<html><body><h1>400 Bad Request</h1></body></html>\n";

    char *fnf = "404 File Not Found\n\n"
        "<html><body><h1>404 File Not Found</h1></body></html>\r\n";

    char *good_req = "200 OK ";
    char *not_imp = "501 Not Implemented\n";
    char *bad_r = "400 Bad Request\n";
    char *fnf_c = "404 File Not Found\n";

    char *response = "\r\nHTTP/1.0 200 OK\r\n\r\n";

    while (1) { // Run forever

        clntlen = sizeof(clntaddr); // Initialize the in-out parameter

        // Wait for client to connect
        if ((clntsock = accept(servsock,
                        (struct sockaddr *) &clntaddr, &clntlen)) < 0)
            die("accept failed");

        // Accept() returned a connected socket (also called client socket)
        // and filled in the client's address into clntaddr

        FILE *clntfile = fdopen(clntsock, "r"); // Wrapper to use fgets()

        if(fgets(request, sizeof(request), clntfile) != NULL){ // Read into request

            char log[256]; // Log of each request
            char *method = strtok(request, token_sep); // Check for GET
            char *requestURI = strtok(NULL, token_sep); // Read URI
            char *httpVersion = strtok(NULL, token_sep); // Read HTTP version

            char http[10];
            strcpy(http, httpVersion); // Preserve original httpVersion

            char URI[30];
            strcpy(URI, requestURI); // Preserve original URI

            /* Part 2 Code begins here */ 

            const char *form =
                "<html>\n"
                "<h1>mdb-lookup</h1>\n"
                "<p>\n"
                "<form method=GET action=/mdb-lookup>\n"
                "lookup: <input type=text name=key>\n"
                "<input type=submit>\n"
                "</form>\n"
                "<p>\n";

            char *http_response = "HTTP/1.0 200 OK\r\n\r\n";

            // Sending hard-coded form
            if ((strcmp(requestURI, "/mdb-lookup") == 0)) {
                send(clntsock, http_response, strlen(http_response), 0);
                if(send(clntsock, form, strlen(form), 0) < 0)
                    die("First send failed.");
                snprintf(log, sizeof(log),"%s \"%s %s %s\" %s\n", inet_ntoa(clntaddr.sin_addr), method, requestURI, httpVersion, good_req);
                fwrite(log, strlen(log), 1, stderr);
                // Good request logged and sent to stdout

                close(clntsock);
            }

            // Extracting search term
            char *search;
            char ch = '=';
            if((search = strrchr(requestURI, ch)) != NULL) { 

                ++search;
                send(clntsock, http_response, strlen(http_response), 0); // Send response

                char lookup[50];
                snprintf(lookup, sizeof(lookup), "looking up [%s]:", search);
                char newline[2] = "\n";
                strcat(search, newline); // append '\n' to search
                //fprintf(stderr, "%s", search); // Check for search
                send(mdbsock, search, strlen(search), 0); // Send search

                FILE *mdb_fp = fdopen(mdbsock, "r"); // Wrap socket
                char match[200]; // Buffer for matches

                snprintf(log, sizeof(log),"%s %s \"%s %s %s\" %s\n", lookup, inet_ntoa(clntaddr.sin_addr), method, URI, http, good_req); // Why not logging HTTP?
                // How remove '/n' from the log?

                fwrite(log, strlen(log), 1, stderr); // Log entry
                send(clntsock, form, strlen(form), 0); // Send lookup form
                int count = 1;
                char *end = "</table>\n</html>";
                char *start = "<table border>\n";
                send(clntsock, start, strlen(start), 0);

                // Print out matches
                while(strcmp((fgets(match, sizeof(match), mdb_fp)), "\n")){
                    if((count % 2) == 0) {
                        char *html = "<tr><td bgcolor=yellow>\n"; 
                        send(clntsock, html, strlen(html), 0);
                        send(clntsock, match, strlen(match), 0);
                        count++;

                    } else {
                        char *html = "<tr><td>\n";
                        send(clntsock, html, strlen(html), 0);
                        send(clntsock, match, strlen(match), 0);
                        count++;

                    }
                }

                send(clntsock, end, strlen(end), 0);

                close(clntsock);
            }

            /* Part 2 Code Ends Here */

            // IF REQUEST IS NOT FOR MDB-LOOKUP
            if(strstr(requestURI, "mdb-lookup") == NULL){
                char rootDir[100];
                memset(rootDir, 0, sizeof(rootDir));
                strcat(rootDir, webRoot);
                strcat(rootDir, requestURI); 

                // Now check if URI ends in '/' using strrchr
                char ch = '/';
                char *chp;
                char *index = "index.html"; // If ends in '/'
                char *index_p = "/index.html"; // If doesn't end in '/' but is directory

                chp = strrchr(rootDir, ch); // Point to last occurrence of '/'
                if (*(chp + 1) == '\0'){ // If ends in '/'
                    strcat(rootDir, index); 
                    // concatenate strings to add index.html
                }

                if (*(chp + 1) == 't') { // Next char is NOT null
                    stat(rootDir, &st);
                    if(S_ISDIR(st.st_mode)){ // Check if directory
                        strcat(rootDir, index_p);
                        // Concatenate strings to add "/index.html"
                    }
                }

                if (strcmp(method, "GET") != 0) { // Check GET and method difference
                    send(clntsock, not_implemented, strlen(not_imp), 0);
                    // Send 501 error message

                    snprintf(log, sizeof(log),"%s \"%s %s %s\" %s", inet_ntoa(clntaddr.sin_addr), method, requestURI, httpVersion, not_imp);

                    fwrite(log, strlen(log), 1, stderr);
                    // Store log and write to stdout
                    printf("\n");
                   // close(clntsock);  // close current socket
                }

                // CHECK FOR '/'
                if (requestURI[0] != '/'){ // Check URI for '/'
                    send(clntsock, bad_req, strlen(bad_req), 0);
                    // Send 400 bad req

                    snprintf(log, sizeof(log),"%s \"%s %s %s\" %s", inet_ntoa(clntaddr.sin_addr), method, requestURI, httpVersion, bad_r); 
                    fwrite(log, strlen(log), 1, stderr);
                    // Store log and write to std out
                    printf("\n");
                   // close(clntsock); // Close current socket
                } 

                // CHECK FOR /../
                if (strstr(requestURI, "/../") != NULL){
                    send(clntsock, bad_req, strlen(bad_req), 0);

                    snprintf(log, sizeof(log),"%s \"%s %s %s\" %s", inet_ntoa(clntaddr.sin_addr), method, requestURI, httpVersion, bad_r);            
                    fwrite(log, strlen(log), 1, stderr);
                    printf("\n");
                   // close(clntsock);
                } // Same as above but now check for "/../"

                // CHECK FOR 1.0 1.1
                if ((strcmp(httpVersion, "HTTP/1.1") && strcmp(httpVersion, "HTTP/1.0") != 0)){
                    send(clntsock, not_implemented, strlen(not_implemented), 0);
                    snprintf(log, sizeof(log),"%s \"%s %s %s\" %s", inet_ntoa(clntaddr.sin_addr), method, requestURI, httpVersion, not_imp);            
                    fwrite(log, strlen(log), 1, stderr);
                    printf("\n");
                   // close(clntsock);
                } // Same as above but now check for HTTP 1.0/1.1 only

                // First, receive file size

                if(send(clntsock, response, strlen(response), 0) != strlen(response))
                     die("Send Failed");

                // Second, receive the file content
                FILE *fp;
                if ((fp = fopen(rootDir, "r")) == NULL){
                    send(clntsock, fnf, strlen(fnf_c), 0);
                    snprintf(log, sizeof(log),"%s \"%s %s %s\" %s", inet_ntoa(clntaddr.sin_addr), method, requestURI, httpVersion, fnf_c);
                    fwrite(log, strlen(log), 1, stderr);
                    //close(clntsock);
// If file does not exist, throw 404 error and close socket

                } else if((strcmp(httpVersion, "HTTP/1.1") && strcmp(httpVersion, "HTTP/1.0") != 0) || (strstr(requestURI, "/../") != NULL) || (requestURI[0] != '/') || (strcmp(method, "GET") != 0)){
                    close(clntsock);
                        }
                else {

                    snprintf(log, sizeof(log),"%s \"%s %s %s\" %s\n", inet_ntoa(clntaddr.sin_addr), method, requestURI, httpVersion, good_req);
                    fwrite(log, strlen(log), 1, stderr);
                    // Good request logged and sent to stdout

                    int count;
                    while ((count = fread(buf, 1, sizeof(buf), fp)) != 0){
                        if (send(clntsock, buf, count, 0) != count) 
                            die("Send failed");
                    } fclose(fp);
                }

                /*Ends here */


                // Finally, close the client connection and go back to accept()
            }
        }
    }
}
