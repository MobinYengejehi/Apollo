#pragma once

#include "Simple-Web-Server/status_code.hpp"
#include <memory>

#ifndef CLOUDGAME_HEADER
#define CLOUDGAME_HEADER

#include "boost/property_tree/ptree_fwd.hpp"

#include <string>
#include <unordered_map>
#include <curl/curl.h>

#include "nvhttp.h"

// #define CLOUDGAME_API_URL           "http://10.202.9.24:7000"
// #define CLOUDGAME_API_VERSION_1_URL CLOUDGAME_API_URL "/v1"

#define CLOUDGAME_API_URL           (config::nvhttp.cloudgame_service_url)
#define CLOUDGAME_API_VERSION_1_URL CLOUDGAME_API_URL

#define CLOUDGAME_API_SELECTED_VERSION_URL CLOUDGAME_API_VERSION_1_URL

#define CLOUDGAME_API_ENDPOINT_USER (CLOUDGAME_API_SELECTED_VERSION_URL + "/user")

#define CLOUDGAME_API_ENDPOINT_USER_PROFILE (CLOUDGAME_API_ENDPOINT_USER + "/profile")
#define CLOUDGAME_API_ENDPOINT_USER_SETTING (CLOUDGAME_API_ENDPOINT_USER + "/setting")

#define CLOUDGAME_API_ENDPOINT_USER_SETTING_GET_TIMEOUT_DETAILS (CLOUDGAME_API_ENDPOINT_USER_SETTING + "/getTimeAmountDetails")

#define CLOUDGAME_API_AUTHENTICATION_HEADER_NAME "Authorization"
#define CLOUDGAME_API_AUTHENTICATION_BEARER "Bearer "

#define CLOUDGAME_DISABLE_VALIDATION (false)

namespace Cloudgame {
    using namespace nvhttp;

    typedef std::unordered_map<std::string, std::string> HeaderMap;
    typedef CURL* RemoteRequestHandle;
    typedef CURLcode RemoteRequestErrorCode;

    typedef resp_http_t HttpResponse;
    typedef req_http_t HttpRequest;

    typedef https_server_t HttpsServer;
    typedef http_server_t HttpServer;

    #define HttpStatusCode SimpleWeb::StatusCode

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
        struct curl_slist* GetHeaderListHandle() const;

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

        HeaderMap          headers;
        struct curl_slist* headerListHandle;

        static size_t WriteDataCallback(void* content, size_t size, size_t nmemb, std::string* response);

    public:
        class Exception : public std::exception {
        public:
            Exception(std::string errorMessage, RemoteRequestErrorCode errorCode = CURLE_OK);
            Exception(RemoteRequestErrorCode errorCode);

            const char* what() const noexcept override;

            const RemoteRequestErrorCode code() const noexcept;
        private:
            std::string            message;
            RemoteRequestErrorCode codeE;
        };
    };

    void Initialize(HttpServer& server, bool& host_audio);

    void PerformAPIRequest(pt::ptree& tree, std::string URL, std::string method = "GET", std::string jwToken = "");

    template<typename KeyType = std::string>
    bool PTreeExistsItem(const pt::ptree& tree, KeyType key) {
        return tree.find(key) != tree.not_found();
    }

    template<typename ItemType, typename KeyType = std::string>
    ItemType PTreeGetItem(const pt::ptree& tree, KeyType key, ItemType defaultValue) {
        return tree.get_optional<ItemType>(key).value_or(defaultValue);
    }

    template<typename KeyType = std::string, typename ItemType = std::string>
    void PTreeSetItem(pt::ptree& tree, KeyType key, ItemType item) {
        tree.put(key, item);
    }

    void PTreeReadJson(pt::ptree& tree, std::string jsonRaw);
    void PTreeReadXml(pt::ptree& tree, std::string xmlRaw);

    std::string PTreeToJson(pt::ptree& tree);
    std::string PTreeToXml(pt::ptree& tree);

    void WriteResponse(HttpResponse& response, pt::ptree& tree, HttpStatusCode statusCode = HttpStatusCode::success_ok, SimpleWeb::CaseInsensitiveMultimap headers = SimpleWeb::CaseInsensitiveMultimap());

    template<typename StreamType>
    void WriteResponse(HttpResponse& response, StreamType stream, HttpStatusCode statusCode = HttpStatusCode::success_ok, SimpleWeb::CaseInsensitiveMultimap headers = SimpleWeb::CaseInsensitiveMultimap()) {
        response->write(statusCode, stream, headers);

        response->close_connection_after_response = true;
    }

    void ValidateRequest(HttpRequest& request);

    std::shared_ptr<rtsp_stream::launch_session_t> MakeLaunchSession(bool host_audio, int appid, const args_t& args, bool launchFromClient = true);
    
    namespace HttpHandlers {
        void not_found(HttpResponse response, HttpRequest request);
        
        void serverinfo(HttpResponse response, HttpRequest request);
        
        void app_list(HttpResponse response, HttpRequest request);
        void app_asset(HttpResponse repsonse, HttpRequest request);

        void launch(bool& host_audio, HttpResponse response, HttpRequest request);
        void resume(bool& host_audio, HttpResponse response, HttpRequest request);
        void cancel(HttpResponse response, HttpRequest request);

        void get_clipboard(HttpResponse respons, HttpRequest request);
        void set_clipboard(HttpResponse response, HttpRequest request);
    };
}

#endif