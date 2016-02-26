/*
 * Daniel Farley - dfarley@ucsc.edu
 * CE 156 - Programing Assignment 4
 * Usage: ./myproxy <local_port>
 */

#include "myunp.h"

#define MAX_LISTEN_QUEUE 16
#define MAX_URL_LENGTH 253

int create_socket(int port);
void *child_handler();
void read_forbidden_list(char *forbidden_file);

int servsock, clisock, remotesock, remote_port = 0;
char **forbidden_sites = NULL;


int main(int argc, char **argv)
{
    struct sockaddr_in cliaddr;
    int local_port, cliaddr_len = sizeof(cliaddr);
    
    if ((argc < 2)|| (argc > 3) || ((local_port = strtoul(argv[1], NULL, 10)) < 0)) {
        printf("Usage: %s <local_port> [-f forbidden_sites.text]\n\n", argv[0]);
        exit(1);
    }
    if (argc == 3) {
        read_forbidden_list(argv[2]);
    } 
        
    
    servsock = create_socket(local_port);
    
    for (;;) {
        clisock = Accept(servsock, (SA *) &cliaddr, &cliaddr_len);
        //TODO
    }
    
    return 0;
}

//Read server IPs/ports into string array
//Modified from https://stackoverflow.com/a/19174415
void read_forbidden_list(char *forbidden_file)
{
    int num_alloced = 4;
    FILE *fp = fopen(forbidden_file, "r");
    
    if (!fp) {
        printf("read_forbidden_list(): Unable to open %s for reading (%s)...\n", 
                forbidden_file, strerror(errno));
        forbidden_sites = NULL;
        return;
    }
    
    if ((forbidden_sites = calloc(num_alloced + 1, sizeof(char *))) == NULL) {
        err_sys("read_forbidden_list(): ERROR allocating memory!\n\n");
    }
    
    int i;
    for (i = 0; 1; i++) {
        
        if (i >= num_alloced) {//We need more space!
            num_alloced *= 2;
            //Get us more space
            if ((forbidden_sites = realloc(forbidden_sites, sizeof(char*) * num_alloced + 1)) == NULL) {
                err_sys("read_forbidden_list(): ERROR allocating memory!\n\n");
            }
            
            //Why isn't there a Recalloc?
            int j;
            for (j = num_alloced/2; j < num_alloced + 1; j++) {
                forbidden_sites[j] = NULL;
            }
        }
        
        //Get next line in the filelength
        if ((forbidden_sites[i] = calloc(MAX_URL_LENGTH + 2, sizeof(char))) == NULL) {
            err_sys("read_forbidden_list(): ERROR allocating memory!\n\n");
        }
        if ((fgets(forbidden_sites[i], MAX_URL_LENGTH + 2, fp)) == NULL) {
            //if it's an empty string (only a newline when read in), free and set to NULL
            free(forbidden_sites[i]);
            forbidden_sites[i] = NULL;
            break;
        }
        
        //strip CR/LF characters and insert \0
        int j;
        for (j = strlen(forbidden_sites[i])-1; 
             j <= 0 && (forbidden_sites[i][j] == '\n' || forbidden_sites[i][j] == '\r');
             j--)
             ;
        forbidden_sites[i][j] = '\0';
    }
    fclose(fp);
    
    // int tmp = 0;
    // while (forbidden_sites[tmp] != NULL) {
        // printf("%d: \"%s\"\n", strlen(forbidden_sites[tmp]), forbidden_sites[tmp]);
        // tmp++;
    // }
    // printf("%d\n\n", tmp);
}

//Create socket for the server end of the proxy
int create_socket(int local_port)
{
    int _servsock, optval;
    struct sockaddr_in servaddr;
    
    _servsock = Socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(_servsock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        err_sys("create_socket(): setsockopt() error: %s", strerror(errno));
    }
    
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(local_port);
    servaddr.sin_addr.s_addr = INADDR_ANY;
    
    Bind(_servsock, (SA *) &servaddr, sizeof(servaddr));
    Listen(_servsock, MAX_LISTEN_QUEUE);
    
    return _servsock;
}
