#include <stdio.h>
#include "csapp.h"

#include <regex.h>

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *requestlint_hdr_format = "GET %s HTTP/1.0\r\n";
static const char *host_hdr_format = "Host: %s\r\n";
static const char *conn_hdr = "Connection: close\r\n";
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *prox_hdr = "Proxy-Connection: close\r\n";
static const char *endof_hdr = "\r\n";

static const char *host_key = "Host";
static const char *conn_key = "Connection";
static const char *user_agent_key= "User-Agent";
static const char *proxy_connection_key = "Proxy-Connection";

void *thread(void *vargp);
void doit(int connfd);
void parse_uri(const char *uri, char *host ,char *hostname, char *port, char *path);
void build_http_header(rio_t *client_rio, const char *host, const char *path, char *http_header);

int main(int argc, char **argv){
  int listenfd, connfd;
  socklen_t  clientlen;
  char hostname[MAXLINE], port[MAXLINE];

  struct sockaddr_storage clientaddr;/*generic sockaddr struct which is 28 Bytes.The same use as sockaddr*/

  if(argc != 2){
    fprintf(stderr, "usage :%s <port> \n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]);
  while(1){
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); // input:listenfd , output:clientaddr, clientlen, connfd
    /*print accepted message*/
    Getnameinfo((SA*)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);//input:clientaddr, clientlen, MAXLINE, 0 , output:hostname, port
    printf("Accepted connection from (%s %s).\n", hostname, port);
    /*concurrent request*/
    pthread_t tid;
    Pthread_create(&tid,NULL,thread,(void *)(intptr_t)connfd);
  }
  return 0;
}

/*thread function*/
void *thread(void *vargp){
  int connfd = (intptr_t)vargp;
  Pthread_detach(pthread_self());
  doit(connfd);
  Close(connfd);
  return NULL;
}

/*handle the client HTTP transaction*/
void doit(int connfd){
  int end_serverfd;/*the end server file descriptor*/

  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char endserver_http_header [MAXLINE];
  /*store the request line arguments*/
  char host[MAXLINE],hostname[MAXLINE], path[MAXLINE];
  char port[MAXLINE]="80";

  rio_t rio, server_rio;/*rio is client's rio, server_rio is endserver's rio*/

  Rio_readinitb(&rio, connfd);
  Rio_readlineb(&rio, buf, MAXLINE);
  sscanf(buf, "%s %s %s", method, uri, version); /*read the client request line*/
  printf("%s][%s][%s", method, uri, version);
  if(strcasecmp(method, "GET")){
    printf("Proxy does not implement the method");
    return;
  }
  /*parse the uri to get hostname, file path , port*/
  parse_uri(uri, host, hostname, port, path);
  /*build the http header which will send to the end server*/
  build_http_header(&rio, host, path, endserver_http_header);
  /*connect to the end server*/
  end_serverfd = Open_clientfd(hostname, port);
  Rio_readinitb(&server_rio, end_serverfd);
  /*write the http header to endserver*/
  Rio_writen(end_serverfd, endserver_http_header, strlen(endserver_http_header));
  /*receive message from end server and send to the client*/
  size_t n;
  while((n=Rio_readlineb(&server_rio, buf, MAXLINE))!=0){
    Rio_writen(connfd, buf, n);
  }
  Close(end_serverfd);
}

/*parse the uri to get hostname, file path , port*/
void parse_uri(const char *uri, char *host, char *hostname, char *port, char *path){
  regex_t reg;
  const size_t nmatch = 5;
  regmatch_t pm[nmatch];

  regcomp(&reg, "//([^/:]*?):?([0-9]*)(/.*)" , REG_EXTENDED);
  if( regexec(&reg, uri, nmatch, pm, 0) == 0){
    strncpy(host, uri + pm[1].rm_so, pm[1].rm_eo-pm[1].rm_so+pm[2].rm_eo-pm[2].rm_so);  // www.baidu.com:8080
    host[pm[1].rm_eo-pm[1].rm_so+pm[2].rm_eo-pm[2].rm_so]=0;

    strncpy(hostname, uri + pm[1].rm_so, pm[1].rm_eo-pm[1].rm_so);                      // www.baidu.com
    hostname[pm[1].rm_eo-pm[1].rm_so]=0;

    if(pm[2].rm_eo!=pm[2].rm_so){
      strncpy(port, uri + pm[2].rm_so, pm[2].rm_eo-pm[2].rm_so);                        // 8080
      port[pm[2].rm_eo-pm[2].rm_so]=0;
    }

    strncpy(path, uri + pm[3].rm_so, pm[3].rm_eo-pm[3].rm_so);
    path[pm[3].rm_eo-pm[3].rm_so]=0;
  }
  regfree(&reg);
  return;
}

void build_http_header(rio_t *client_rio, const char *host, const char *path, char *http_header){
  char buf[MAXLINE], request_hdr[MAXLINE], other_hdr[MAXLINE], host_hdr[MAXLINE];
  /*request line*/
  sprintf(request_hdr, requestlint_hdr_format, path);
  /*get other request header for client rio and change it */
  while(Rio_readlineb(client_rio, buf, MAXLINE)>0){
    if(strcmp(buf, endof_hdr)==0)/*EOF*/
      break;
    if(!strncasecmp(buf, host_key, strlen(host_key))){/*Host:*/
      strcpy(host_hdr, buf);
      continue;
    }
    if( !strncasecmp(buf, conn_key, strlen(conn_key))&&
        !strncasecmp(buf, proxy_connection_key, strlen(proxy_connection_key))&&
        !strncasecmp(buf, user_agent_key, strlen(user_agent_key))){
      strcat(other_hdr, buf);
    }
  }
  if(strlen(host_hdr)==0){
    sprintf(host_hdr, host_hdr_format, host);
  }
  sprintf(http_header, "%s%s%s%s%s%s%s", 
      request_hdr, 
      host_hdr, 
      conn_hdr, 
      user_agent_hdr, 
      prox_hdr, 
      other_hdr, 
      endof_hdr);
  return ;
}
