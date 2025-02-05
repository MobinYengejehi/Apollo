#pragma once

#ifndef CLOUDGAME_HEADER
#define CLOUDGAME_HEADER

#include <string>
#include <unordered_map>
#include <curl/curl.h>

#define CLOUDGAME_API_URL           "http://10.202.9.24:7000/"
#define CLOUDGAME_API_VERSION_1_URL CLOUDGAME_API_URL "v1/"

#define CLOUDGAME_API_SELECTED_VERSION_URL CLOUDGAME_API_VERSION_1_URL

#define CLOUDGAME_API_ENDPOINT_USER CLOUDGAME_API_SELECTED_VERSION_URL "user/"

#define CLOUDGAME_API_ENDPOINT_USER_PROFILE CLOUDGAME_API_ENDPOINT_USER "profile/"

namespace Cloudgame {
    typedef std::unordered_map<std::string, std::string> HeaderMap;
    typedef CURL* RemoteRequestHandle;
    typedef CURLcode RemoteRequestErrorCode;

    class RemoteRequest {
    protected:
        std::string URL;
        std::string method;
        std::string response;

    public:
        RemoteRequest(std::string url, std::string requestMethod = "GET");
        ~RemoteRequest();

        bool Initialize();
        bool Cleanup();

        std::string        GetURL() const;
        std::string        GetMethod() const;
        const std::string& GetResponse() const;
        HeaderMap&         GetHeaders();
        const HeaderMap&   GetHeaders() const;

        CURL* GetHandle() const;

        RemoteRequestErrorCode Perform();

        bool IsValid() const;
        bool IsReady() const;

        operator bool() const;

        static bool        HasError(RemoteRequestErrorCode errorCode);
        static std::string GetErrorCodeMessage(RemoteRequestErrorCode errorCode);
    
    private:
        CURL* handle;
        bool  ready;

        HeaderMap headers;

        static size_t WriteDataCallback(void* content, size_t size, size_t nmemb, std::string* response);

    public:
        class Exception : public std::exception {
        public:
            Exception(std::string errorMessage, RemoteRequestErrorCode errorCode = CURLE_OK);
            Exception(RemoteRequestErrorCode errorCode);

            const char* what() const noexcept override;
        private:
            std::string            message;
            RemoteRequestErrorCode codeE;
        };
    };

    void Initialize();
}

#endif