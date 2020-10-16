// Copyright (C) <2019> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifdef _ENABLE_WEBRTC_SOURCE_
#pragma once
#include <string>

using namespace std;

class CHttp
{

public:
    CHttp();
    ~CHttp();
    static string http_get(const string &url);
    static string http_post(const string &url, const string &content);
    static string http_patch(const string &url, const string &content);
    static string getToken(const string &addr, const string &room);

private:
    static int http_tcpclient_create(const string &host, int port);
    static void http_tcpclient_close(int socket);
    static int http_parse_url(const string &url, char *host, char *file, int *port);
    static int http_tcpclient_recv(int socket, char *lpbuff);
    static int http_tcpclient_send(int socket, char *buff, int size);
    static string http_parse_result(const char *lpbuf);
};
#endif /* _ENABLE_WEBRTC_SOURCE_ */
