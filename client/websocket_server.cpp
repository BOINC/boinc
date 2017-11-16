#include "websocket_server.h"
#include <string>
#include <vector>
#include "version.h"
#include <iostream>
#include <typeinfo>
#include "miofile.h"
#include "pers_file_xfer.h"
#include "gui_rpc_server.h"
#include "client_state.h"
using std::vector;
using std::string;
#define NTHREADS 100


WEBSOCK::WEBSOCK() {}

int portno = 31416;
struct sockaddr_in serveraddr;
struct hostent *server;
char *hostname = "127.0.0.1";

struct threading {
     struct mg_connection* nc;
     int socketfd;
};

pthread_t th[NTHREADS];
std::map<mg_connection*, int> tunnel;
static int socket_num = 0;

int is_websocket(const struct mg_connection *nc) {
  return nc->flags & MG_F_IS_WEBSOCKET;
}

void* recv_message(void* nco){
 
     char recv_buf[50000];
     struct threading* threadin = (struct threading *) nco;	
     struct mg_connection *nc =  threadin->nc;
     int sockfd = threadin->socketfd;

     memset(recv_buf, 0, sizeof(recv_buf));
     recv(sockfd, recv_buf, 50000, 0);

     printf("\n\nThe thread id is: %u\n and the sockfd is: %d\n\n", (unsigned int)pthread_self(), sockfd);

     printf("Received in pid=%d, text:\n\n%s\n\n",getpid(), recv_buf);

     mg_send_websocket_frame( nc, WEBSOCKET_OP_TEXT, recv_buf, strlen(recv_buf));

     return 0; 
}


void command(struct mg_connection *nc) {

  int sockfd;

  server = gethostbyname(hostname);
  
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
 
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_port = htons(portno);
  serveraddr.sin_addr = *((struct in_addr *)server->h_addr);
  bzero(&(serveraddr.sin_zero), 8);
  
  connect(sockfd, (struct sockaddr *) &serveraddr, sizeof(struct sockaddr));

  ++socket_num;
  tunnel[nc] = sockfd;

  printf ("The number of created sockets is: %d\n\n", socket_num);

}

void proxy(struct mg_connection *nc, const struct mg_str msg) {

        char buf[50000];

	int sockfd = tunnel.find(nc)->second;

	printf("Proxying websocket msg to actual socket with id: %d\n\n",sockfd);
  
        struct threading *threadinio = ( struct threading *)malloc(sizeof(struct threading));
        threadinio->nc = nc;
        threadinio->socketfd = sockfd;
  
	memset(buf, 0, sizeof(buf));
        snprintf(buf, sizeof(buf), "%s", /*(int) msg.len,*/ msg.p);
        printf("%s", buf);
        write(sockfd, buf, 50000);
        pthread_create(&th[socket_num], NULL, recv_message, threadinio);
	return;
}

void closer(struct mg_connection *nc) {

      int sockfd = tunnel.find(nc)->second;

      if (close(sockfd)) 
	 printf("Could not correctly close open socket for socket: %d\n\n", sockfd);
      else printf("Succesfully closed socket with id: %d\n\n",sockfd);
      return;
}


void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
     switch (ev) {
        case MG_EV_WEBSOCKET_HANDSHAKE_DONE: {
	   command(nc);
           break;
        }   
        case MG_EV_WEBSOCKET_FRAME: {
           struct websocket_message *wm = (struct websocket_message *) ev_data;
           struct mg_str d = {(char *) wm->data, wm->size};
	   
	   proxy(nc, d);
           break;
        }
        case MG_EV_CLOSE: {
           if (is_websocket(nc)) {
	       closer(nc);
           }   
           break;
        }
     }
}

void WEBSOCK::initiate(mg_mgr& mgr) {

    
    struct mg_connection *nc;
    struct mg_bind_opts bind_opts;
    const char *err;

    mg_mgr_init(&mgr, NULL);
    memset(&bind_opts, 0, sizeof(bind_opts));
    bind_opts.ssl_cert = s_ssl_cert;
    bind_opts.ssl_key = s_ssl_key;
    bind_opts.error_string = &err;

    printf("Starting SSL server on port %s, cert from %s, key from %s\n\n\n",
           s_http_port, bind_opts.ssl_cert, bind_opts.ssl_key);
    nc = mg_bind_opt(&mgr, s_http_port, ev_handler, bind_opts);

    if (nc == NULL) {
         printf("Failed to create listener: %s\n", err);
         return;
    }
    
    setvbuf(stdout, NULL, _IOLBF, 0);
    setvbuf(stderr, NULL, _IOLBF, 0);


    mg_set_protocol_http_websocket(nc);
}
