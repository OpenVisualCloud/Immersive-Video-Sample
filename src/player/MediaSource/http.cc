#ifdef _ENABLE_WEBRTC_SOURCE_
#include "http.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MY_HTTP_DEFAULT_PORT 80

#define BUFFER_SIZE 1024
#define HTTP_POST "POST /%s HTTP/1.1\r\nHOST: %s:%d\r\nAccept: application/json\r\n" \
                  "Content-Type: application/json\r\nContent-Length: %ld\r\n\r\n%s"
#define HTTP_PATCH "PATCH /%s HTTP/1.1\r\nHOST: %s:%d\r\nAccept: application/json\r\n" \
                  "Content-Type: application/json\r\nContent-Length: %ld\r\n\r\n%s"
#define HTTP_GET "GET /%s HTTP/1.1\r\nHOST: %s:%d\r\nAccept: application/json\r\n\r\n"


#define LOG_DEBUG(args...)
//#define LOG_DEBUG printf

CHttp::CHttp()
{
}

CHttp::~CHttp()
{
}

int CHttp::http_tcpclient_create(const string &host, int port)
{
    LOG_DEBUG("");
    struct hostent *he;
    struct sockaddr_in server_addr;
    int socket_fd;

    if ((he = gethostbyname(host.c_str())) == NULL)
    {
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr *)he->h_addr);

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        return -1;
    }

    if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        return -1;
    }

    return socket_fd;
}

void CHttp::http_tcpclient_close(int socket)
{
    LOG_DEBUG("");
    close(socket);
}

int CHttp::http_parse_url(const string &url, char *host, char *file, int *port)
{
    LOG_DEBUG("");
    char *ptr1, *ptr2;
    int len = 0;
    if (url.empty() || !host || !file || !port)
    {
        return -1;
    }

    ptr1 = (char *)url.c_str();

    if (!strncmp(ptr1, "http://", strlen("http://")))
    {
        ptr1 += strlen("http://");
    }
    else
    {
        return -1;
    }

    ptr2 = strchr(ptr1, '/');
    if (ptr2)
    {
        len = strlen(ptr1) - strlen(ptr2);
        memcpy(host, ptr1, len);
        host[len] = '\0';
        if (*(ptr2 + 1))
        {
            memcpy(file, ptr2 + 1, strlen(ptr2) - 1);
            file[strlen(ptr2) - 1] = '\0';
        }
    }
    else
    {
        memcpy(host, ptr1, strlen(ptr1));
        host[strlen(ptr1)] = '\0';
    }
    //get host and ip
    ptr1 = strchr(host, ':');
    if (ptr1)
    {
        *ptr1++ = '\0';
        *port = atoi(ptr1);
    }
    else
    {
        *port = MY_HTTP_DEFAULT_PORT;
    }

    return 0;
}

int CHttp::http_tcpclient_recv(int socket, char *lpbuff)
{
    LOG_DEBUG("");
    int recvnum = 0;

    recvnum = recv(socket, lpbuff, BUFFER_SIZE * 4, 0);

    return recvnum;
}

int CHttp::http_tcpclient_send(int socket, char *buff, int size)
{
    LOG_DEBUG("");
    int sent = 0, tmpres = 0;

    while (sent < size)
    {
        tmpres = send(socket, buff + sent, size - sent, 0);
        if (tmpres == -1)
        {
            return -1;
        }
        sent += tmpres;
    }
    return sent;
}

string CHttp::http_parse_result(const char *lpbuf)
{
    LOG_DEBUG("");
    char *ptmp = NULL;
    char *response = NULL;
    ptmp = (char *)strstr(lpbuf, "HTTP/1.1");
    if (!ptmp)
    {
        LOG_DEBUG("http/1.1 not faind");
        return "http/1.1 not faind";
    }
    if (atoi(ptmp + 9) != 200)
    {
        LOG_DEBUG("result:%s", lpbuf);
        return "Internal Server Error";
    }

    ptmp = (char *)strstr(lpbuf, "\r\n\r\n");
    if (!ptmp)
    {
        LOG_DEBUG("ptmp is NULL");
        return "ptmp is NULL";
    }
    response = (char *)malloc(strlen(ptmp) + 1);
    if (!response)
    {
        LOG_DEBUG("malloc failed");
        return "malloc failed";
    }
    strcpy(response, ptmp + 4);
    string ret = response;
    free(response);
    return ret;
}

string CHttp::http_post(const string &url, const string &content)
{
    LOG_DEBUG("");
    char post[BUFFER_SIZE] = {'\0'};
    int socket_fd = -1;
    char lpbuf[BUFFER_SIZE * 4] = {'\0'};
    char *ptmp;
    char host_addr[BUFFER_SIZE] = {'\0'};
    char file[BUFFER_SIZE] = {'\0'};
    int port = 0;
    int len = 0;
    char *response = NULL;

    if (url.empty() || content.empty())
    {
        LOG_DEBUG("failed!");
        return "url or content is empty";
    }

    if (http_parse_url(url.c_str(), host_addr, file, &port))
    {
        LOG_DEBUG("http_parse_url failed!");
        return "http_parse_url failed";
    }
    //LOG_DEBUG("host_addr : %s\tfile:%s\t,%d",host_addr,file,port);

    socket_fd = http_tcpclient_create(host_addr, port);
    if (socket_fd < 0)
    {
        LOG_DEBUG("http_tcpclient_create failed");
        return "http_tcpclient_create failed";
    }

    sprintf(lpbuf, HTTP_POST, file, host_addr, port, content.size(), content.c_str());

    if (http_tcpclient_send(socket_fd, lpbuf, strlen(lpbuf)) < 0)
    {
        LOG_DEBUG("http_tcpclient_send failed..");
        return "http_tcpclient_send failed";
    }
    //LOG_DEBUG("send request:\n%s",lpbuf);

    /*it's time to recv from server*/
    if (http_tcpclient_recv(socket_fd, lpbuf) <= 0)
    {
        LOG_DEBUG("http_tcpclient_recv failed");
        return "http_tcpclient_recv failed";
    }

    http_tcpclient_close(socket_fd);

    return http_parse_result(lpbuf);
}

string CHttp::http_patch(const string &url, const string &content)
{
    LOG_DEBUG("");
    char post[BUFFER_SIZE] = {'\0'};
    int socket_fd = -1;
    char lpbuf[BUFFER_SIZE * 4] = {'\0'};
    char *ptmp;
    char host_addr[BUFFER_SIZE] = {'\0'};
    char file[BUFFER_SIZE] = {'\0'};
    int port = 0;
    int len = 0;
    char *response = NULL;

    if (url.empty() || content.empty())
    {
        LOG_DEBUG("failed!");
        return "url or content is empty";
    }

    if (http_parse_url(url.c_str(), host_addr, file, &port))
    {
        LOG_DEBUG("http_parse_url failed!");
        return "http_parse_url failed";
    }
    //LOG_DEBUG("host_addr : %s\tfile:%s\t,%d",host_addr,file,port);

    socket_fd = http_tcpclient_create(host_addr, port);
    if (socket_fd < 0)
    {
        LOG_DEBUG("http_tcpclient_create failed");
        return "http_tcpclient_create failed";
    }

    sprintf(lpbuf, HTTP_PATCH, file, host_addr, port, content.size(), content.c_str());
    LOG_DEBUG("lpbuf : %s",lpbuf);

    if (http_tcpclient_send(socket_fd, lpbuf, strlen(lpbuf)) < 0)
    {
        LOG_DEBUG("http_tcpclient_send failed..");
        return "http_tcpclient_send failed";
    }
    //LOG_DEBUG("send request:\n%s",lpbuf);

    /*it's time to recv from server*/
    if (http_tcpclient_recv(socket_fd, lpbuf) <= 0)
    {
        LOG_DEBUG("http_tcpclient_recv failed");
        return "http_tcpclient_recv failed";
    }

    http_tcpclient_close(socket_fd);

    return http_parse_result(lpbuf);
}

string CHttp::http_get(const string &url)
{
    LOG_DEBUG("");
    char post[BUFFER_SIZE] = {'\0'};
    int socket_fd = -1;
    char lpbuf[BUFFER_SIZE * 4] = {'\0'};
    char *ptmp;
    char host_addr[BUFFER_SIZE] = {'\0'};
    char file[BUFFER_SIZE] = {'\0'};
    int port = 0;
    int len = 0;

    if (url.empty())
    {
        LOG_DEBUG("failed!");
        return "url is empty";
    }

    if (http_parse_url(url, host_addr, file, &port))
    {
        LOG_DEBUG("http_parse_url failed!");
        return "http_parse_url failed";
    }
    //LOG_DEBUG("host_addr : %s\tfile:%s\t,%d",host_addr,file,port);

    socket_fd = http_tcpclient_create(host_addr, port);
    if (socket_fd < 0)
    {
        LOG_DEBUG("http_tcpclient_create failed");
        return "http_tcpclient_create failed";
    }

    sprintf(lpbuf, HTTP_GET, file, host_addr, port);

    if (http_tcpclient_send(socket_fd, lpbuf, strlen(lpbuf)) < 0)
    {
        LOG_DEBUG("http_tcpclient_send failed..");
        return "http_tcpclient_send failed";
    }
    //	LOG_DEBUG("send request:%s",lpbuf);

    if (http_tcpclient_recv(socket_fd, lpbuf) <= 0)
    {
        LOG_DEBUG("http_tcpclient_recv failed");
        return "http_tcpclient_recv failed";
    }
    http_tcpclient_close(socket_fd);

    return http_parse_result(lpbuf);
}

string CHttp::getToken(const string &addr, const string &room)
{
    LOG_DEBUG("");
    string content = "{\"room\":\"" + room + "\",\"role\":\"presenter\",\"username\":\"user\"}";
    string url = addr + "/createToken";
    return http_post(url, content);
}
#endif /* _ENABLE_WEBRTC_SOURCE_ */
