#include <exception>
#include <sstream>
#include <stdio.h>

#include "cloudgame.h"
#include "Simple-Web-Server/status_code.hpp"
#include "boost/log/sources/record_ostream.hpp"
#include "boost/property_tree/ptree_fwd.hpp"
#include "curl/curl.h"
#include "curl/easy.h"

// lib includes
#include <Simple-Web-Server/server_http.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/context_base.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "logging.h"

#define H Cloudgame::HttpHandlers::

Cloudgame::RemoteRequest::RemoteRequest(std::string url, std::string requestMethod) {
    Cleanup();

    URL = url;
    method = requestMethod;
}

Cloudgame::RemoteRequest::~RemoteRequest() {
    Cleanup();
}

bool Cloudgame::RemoteRequest::Initialize() {
    Cleanup();

    handle = curl_easy_init();

    if (handle) {
        curl_easy_setopt(handle, CURLOPT_URL, URL.c_str());
        curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, WriteDataCallback);
        curl_easy_setopt(handle, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, method.c_str());
    }

    return IsValid();
}

bool Cloudgame::RemoteRequest::Cleanup() {
    response = "";
    ready = false;
    
    headers.clear();

    if (handle) {
        curl_easy_cleanup(handle);
    }

    handle = NULL;

    return true;
}

std::string Cloudgame::RemoteRequest::GetURL() const {
    return URL;
}

std::string Cloudgame::RemoteRequest::GetMethod() const {
    return method;
}

const std::string& Cloudgame::RemoteRequest::GetResponse() const {
    return response;
}

Cloudgame::HeaderMap& Cloudgame::RemoteRequest::GetHeaders() {
    return headers;
}

const Cloudgame::HeaderMap& Cloudgame::RemoteRequest::GetHeaders() const {
    return headers;
}

CURL* Cloudgame::RemoteRequest::GetHandle() const {
    return handle;
}

Cloudgame::RemoteRequestErrorCode Cloudgame::RemoteRequest::Perform() {
    RemoteRequestErrorCode code = curl_easy_perform(handle);

    ready = true;

    return code;
}

bool Cloudgame::RemoteRequest::IsValid() const {
    return handle != NULL;
}

bool Cloudgame::RemoteRequest::IsReady() const {
    return ready;
}

Cloudgame::RemoteRequest::operator bool() const {
    return IsValid();
}

bool Cloudgame::RemoteRequest::HasError(RemoteRequestErrorCode errorCode) {
    return errorCode != CURLE_OK;
}

std::string Cloudgame::RemoteRequest::GetErrorCodeMessage(RemoteRequestErrorCode errorCode) {
    return curl_easy_strerror(errorCode);
}

size_t Cloudgame::RemoteRequest::WriteDataCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t totalSize = size * nmemb;

    response->append((char*)contents, totalSize);

    return totalSize;
}

Cloudgame::RemoteRequest::Exception::Exception(std::string errorMessage, RemoteRequestErrorCode errorCode) {
    message = errorMessage;
    codeE = errorCode;
}

Cloudgame::RemoteRequest::Exception::Exception(RemoteRequestErrorCode errorCode) {
    codeE = errorCode;
    message = GetErrorCodeMessage(codeE);
}

const char* Cloudgame::RemoteRequest::Exception::what() const noexcept {
    return message.c_str();
}

const Cloudgame::RemoteRequestErrorCode Cloudgame::RemoteRequest::Exception::code() const noexcept {
    return codeE;
}

void Cloudgame::Initialize(HttpServer& server, bool& host_audio) {
    // pt::ptree tree;

    // tree.put("name", "something");

    // std::ostringstream data;
    // pt::write_json(data, tree);

    // BOOST_LOG(info) << "data is : " << data.str().c_str();

    // try {
    //     pt::ptree response;

    //     PerformAPIRequest(response, CLOUDGAME_API_ENDPOINT_USER_PROFILE);
    // } catch (const std::exception& exc) {
    //     BOOST_LOG(error) << exc.what();
    // }
    
    server.default_resource["GET"] = H not_found;
    server.resource["^/serverinfo$"]["GET"] = H serverinfo;
    server.resource["^/applist$"]["GET"] = H app_list;
    server.resource["^/appasset$"]["GET"] = H app_asset;
    server.resource["^/launch$"]["GET"] = [&host_audio](HttpResponse response, HttpRequest request) { H launch(host_audio, response, request); };
    server.resource["^/resume$"]["GET"] = [&host_audio](HttpResponse response, HttpRequest request) { H resume(host_audio, response, request); };
    server.resource["^/cancel$"]["GET"] = H cancel;
    server.resource["^/actions/clipboard$"]["GET"] = H get_clipboard;
    server.resource["^/actions/clipboard$"]["POST"] = H set_clipboard;

    BOOST_LOG(info) << "'Cloudgame Service' started working.";
}

void Cloudgame::PerformAPIRequest(pt::ptree& tree, std::string URL, std::string method) {
    RemoteRequest request(URL, method);
    request.Initialize();

    if (!request) {
        throw RemoteRequest::Exception("Failed to initialize the request.", CURLE_COULDNT_CONNECT);
    }

    RemoteRequestErrorCode errorCode = request.Perform();

    if (RemoteRequest::HasError(errorCode)) {
        throw RemoteRequest::Exception(errorCode);
    }

    std::string response = request.GetResponse();

    PTreeReadJson(tree, response);

    if (!PTreeExistsItem(tree, "isSuccess")) {
        throw RemoteRequest::Exception(CURLE_BAD_CONTENT_ENCODING);
    }

    if (!PTreeGetItem(tree, "isSuccess", false)) {
        throw RemoteRequest::Exception(
            "[API_ERROR]: " + PTreeGetItem<std::string>(tree, "message", ""),
            (RemoteRequestErrorCode)PTreeGetItem<int>(tree, "statusCode", 0)
        );
    }
}

void Cloudgame::PTreeReadJson(pt::ptree& tree, std::string jsonRaw) {
    std::istringstream jsonRawStream(jsonRaw);

    pt::read_json(jsonRawStream, tree);
}

void Cloudgame::PTreeReadXml(pt::ptree& tree, std::string xmlRaw) {
    std::istringstream xmlRawStream(xmlRaw);

    pt::read_xml(xmlRawStream, tree);
}

std::string Cloudgame::PTreeToJson(pt::ptree& tree) {
    std::ostringstream jsonRawStream;

    pt::write_json(jsonRawStream, tree);

    return jsonRawStream.str();
}

std::string Cloudgame::PTreeToXml(pt::ptree& tree) {
    std::ostringstream xmlRawStream;

    pt::write_xml(xmlRawStream, tree);

    return xmlRawStream.str();
}

void Cloudgame::WriteResponse(HttpResponse& response, pt::ptree& tree, HttpStatusCode statusCode) {
    std::string responseBuffer = PTreeToXml(tree);

    response->write(statusCode, responseBuffer.data());

    response->close_connection_after_response = true;
}

void Cloudgame::ValidateRequest(HttpRequest& request) {
    
}

#define SAFE_SCOPE_START(vars)                         { vars; try {
#define SAFE_SCOPE_END(response, tree)                 } catch (const std::exception& ex) { HttpStatusCode statusCode = HttpStatusCode::client_error_bad_request; PTreeSetItem<std::string, int>(tree, "root.<xmlattr>.status_code", (int)statusCode); PTreeSetItem(tree, "root.<xmlattr>.status_message", ex.what()); WriteResponse(response, tree, statusCode); } }
#define SAFE_SCOPE_END_SC(response, tree, status_code) } catch (const std::exception& ex) { HttpStatusCode statusCode = status_code; PTreeSetItem<std::string, int>(tree, "root.<xmlattr>.status_code", (int)statusCode); PTreeSetItem(tree, "root.<xmlattr>.status_message", ex.what()); WriteResponse(response, tree, statusCode); } }

void H not_found(HttpResponse response, HttpRequest request)
SAFE_SCOPE_START(pt::ptree tree)
    HttpStatusCode statusCode = HttpStatusCode::client_error_not_found;

    PTreeSetItem<std::string, int>(tree, "root.<xmlattr>.status_code", (int)statusCode);

    WriteResponse(response, tree, statusCode);
SAFE_SCOPE_END(response, tree)

void H serverinfo(HttpResponse response, HttpRequest request)
SAFE_SCOPE_START(pt::ptree tree)
    ValidateRequest(request);

    not_found(response, request);
SAFE_SCOPE_END(response, tree)

void H app_list(HttpResponse response, HttpRequest request)
SAFE_SCOPE_START(pt::ptree tree)
    ValidateRequest(request);

    not_found(response, request);
SAFE_SCOPE_END(response, tree)

void H app_asset(HttpResponse response, HttpRequest request)
SAFE_SCOPE_START(pt::ptree tree)
    ValidateRequest(request);

    not_found(response, request);
SAFE_SCOPE_END(response, tree)

void H launch(bool& host_audio, HttpResponse response, HttpRequest request)
SAFE_SCOPE_START(pt::ptree tree)
    ValidateRequest(request);

    not_found(response, request);
SAFE_SCOPE_END(response, tree)

void H resume(bool& host_audio, HttpResponse response, HttpRequest request)
SAFE_SCOPE_START(pt::ptree tree)
    ValidateRequest(request);

    not_found(response, request);
SAFE_SCOPE_END(response, tree)

void H cancel(HttpResponse response, HttpRequest request)
SAFE_SCOPE_START(pt::ptree tree)
    ValidateRequest(request);

    not_found(response, request);
SAFE_SCOPE_END(response, tree)

void H get_clipboard(HttpResponse response, HttpRequest request)
SAFE_SCOPE_START(pt::ptree tree)
    ValidateRequest(request);

    not_found(response, request);
SAFE_SCOPE_END(response, tree)

void H set_clipboard(HttpResponse response, HttpRequest request)
SAFE_SCOPE_START(pt::ptree tree)
    ValidateRequest(request);

    not_found(response, request);
SAFE_SCOPE_END(response, tree)

#undef SAFE_SCOPE_START
#undef SAFE_SCOPE_END
#undef SAFE_SCOPE_END_SC

#undef H