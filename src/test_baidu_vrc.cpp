/***************************************************************************
 *
 * Copyright (c) 2014 Baidu.com, Inc. All Rights Reserved
 *
 **************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include "curl/curl.h"
#include "curl/easy.h"
#include "json/json.h"
#include "base64.h"

#define MAX_BUFFER_SIZE 512
#define MAX_BODY_SIZE 1000000

#define _METHOD_1_

//put your own params here
char const *CUID = "e6:f6:85:78:cd:25";
char const *API_KEY = "vM5QGtUWb9z0TXHCIm1nLLX6";
char const *SECRET_KEY = "caa8656826529433aaa997d2fd002851";

static size_t writefunc(void *ptr, size_t size, size_t nmemb, char **result)
{
    size_t result_len = size * nmemb;
    *result = (char *)realloc(*result, result_len + 1);
    if (*result == NULL)
    {
        printf("realloc failure!\n");
        return 1;
    }
    memcpy(*result, ptr, result_len);
    (*result)[result_len] = '\0';
    printf("%s\n", *result);
    return result_len;
}


int main (int argc,char* argv[])
{
	char cHost[MAX_BUFFER_SIZE];
	std::string sToken;

    if ((argc != 2) && (argc != 3))
    {
        printf("Usage: %s audiofile\n", argv[0]);
        return -1;
    }

    if (argc == 2){
        snprintf(cHost, sizeof(cHost),
                "https://openapi.baidu.com/oauth/2.0/token?grant_type=client_credentials&client_id=%s&client_secret=%s",
                API_KEY, SECRET_KEY);
        FILE* pNewProcessPipe = NULL;
        char cCurlCmd[MAX_BUFFER_SIZE];
        char* cCurlExecuteResult = (char*)malloc(MAX_BUFFER_SIZE);
        snprintf(cCurlCmd, sizeof(cCurlCmd), "curl -k \"%s\"", cHost);
        pNewProcessPipe = popen(cCurlCmd, "r");
        fgets(cCurlExecuteResult, MAX_BUFFER_SIZE, pNewProcessPipe);
        pclose(pNewProcessPipe);

        if (cCurlExecuteResult != NULL) {
            Json::Reader JsonTokenReader;
            Json::Value JsonTokenValue;
            Json::FastWriter JsonTokenWriter;
            if (JsonTokenReader.parse(cCurlExecuteResult, JsonTokenValue, false)) {
                sToken = JsonTokenValue.get("access_token","").asString();
                puts("parse token OK");
                puts(JsonTokenWriter.write(JsonTokenValue).c_str());
                printf("Token came from fresh gotten: %s", sToken.c_str());
            } else {
            	puts("Parse token error");
            	puts(JsonTokenWriter.write(JsonTokenValue).c_str());
            	return (-1);
            }
        }
    } else {
    	printf("Token came from input: %s", argv[2]);
    	sToken = std::string(argv[2]);
    }


    memset(cHost, 0, sizeof(cHost));
    snprintf(cHost, sizeof(cHost), "%s", "http://vop.baidu.com/server_api");

    FILE *pAudioFile = NULL;
    pAudioFile = fopen(argv[1], "r");
    if (NULL == pAudioFile)
    {
        return -1;
    }
    fseek(pAudioFile, 0, SEEK_END);
    unsigned int unLength = ftell(pAudioFile);
    fseek(pAudioFile, 0, SEEK_SET);
    char *pAudioData = (char *)malloc(unLength);
    fread(pAudioData, unLength, sizeof(char), pAudioFile);

#ifdef _METHOD_1_
    //method 1
    char cHttpHeader[MAX_BUFFER_SIZE];
    memset(cHttpHeader, 0, sizeof(cHttpHeader));
    char cPostBody[MAX_BODY_SIZE];
    memset(cPostBody, 0, sizeof(cPostBody));
    std::string sEncodeData = base64_encode((unsigned char const*)pAudioData, unLength);
    if (0 == sEncodeData.length())
    {
        printf("base64 encoded data is empty.\n");
        return 1;
    }

    Json::Value JsonPostValue;
    Json::FastWriter JsonPostWriter;
    JsonPostValue["format"]  = "pcm";
    JsonPostValue["rate"]    = 8000;
    JsonPostValue["channel"] = 1;
    JsonPostValue["token"]   = sToken.c_str();
    JsonPostValue["cuid"]    = CUID;
    JsonPostValue["speech"]  = sEncodeData;
    JsonPostValue["len"]     = unLength;
//    buffer["url"]  = url;
//    buffer["callback"]     = callback;

    unLength = JsonPostWriter.write(JsonPostValue).length();
    memcpy(cPostBody, JsonPostWriter.write(JsonPostValue).c_str(), unLength);

    CURL *pCurl;
    CURLcode nCurlCode;
    char *resultBuf = NULL;
    struct curl_slist *headerlist = NULL;
    snprintf(cHttpHeader, sizeof(cHttpHeader), "%s", "Content-Type: application/json; charset=utf-8");
    headerlist = curl_slist_append(headerlist, cHttpHeader);
    snprintf(cHttpHeader, sizeof(cHttpHeader), "Content-Length: %d", unLength);
    headerlist = curl_slist_append(headerlist, cHttpHeader);

    pCurl = curl_easy_init();
    curl_easy_setopt(pCurl, CURLOPT_URL, cHost);
    curl_easy_setopt(pCurl, CURLOPT_POST, 1);
    curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 30);
    curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, headerlist);
    curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, cPostBody);
    curl_easy_setopt(pCurl, CURLOPT_POSTFIELDSIZE, unLength);
    curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &resultBuf);
    nCurlCode = curl_easy_perform(pCurl);
    if (nCurlCode != CURLE_OK)
    {
        printf("perform curl error:%d.\n", nCurlCode);
        return 1;
    }
    curl_slist_free_all(headerlist);
    curl_easy_cleanup(pCurl);

#endif

#ifdef _METHOD_2_
    //second way, post raw data
    char cHttpHeader[MAX_BUFFER_SIZE];
    memset(cHttpHeader, 0, sizeof(cHttpHeader));
    snprintf(cHttpHeader, sizeof(cHttpHeader), "?cuid=%s&token=%s", CUID, sToken.c_str());
    strcat(cHost, cHttpHeader);

    CURL *pCurl;
    CURLcode nCurlCode;
    char *resultBuf = NULL;
    struct curl_slist *headerlist = NULL;
    snprintf(cHttpHeader, sizeof(cHttpHeader), "%s","Content-Type: audio/pcm; rate=8000");
    headerlist = curl_slist_append(headerlist, cHttpHeader);
    snprintf(cHttpHeader, sizeof(cHttpHeader), "Content-Length: %d", unLength);
    headerlist = curl_slist_append(headerlist, cHttpHeader);

    pCurl = curl_easy_init();
    curl_easy_setopt(pCurl, CURLOPT_URL, cHost);
    curl_easy_setopt(pCurl, CURLOPT_POST, 1);
    curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 30);
    curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, headerlist);
    curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, pAudioData);
    curl_easy_setopt(pCurl, CURLOPT_POSTFIELDSIZE, unLength);
    curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &resultBuf);
    nCurlCode = curl_easy_perform(pCurl);
    if (nCurlCode != CURLE_OK)
    {
        printf("perform curl error:%d.\n", nCurlCode);
        return 1;
    }
    curl_slist_free_all(headerlist);
    curl_easy_cleanup(pCurl);

#endif
    fclose(pAudioFile);
    free(pAudioData);
    return 0;
}
