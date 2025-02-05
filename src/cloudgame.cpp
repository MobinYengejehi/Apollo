#include <sstream>
#include <stdio.h>

#include "cloudgame.h"
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

// - -IC:\msys64\ucrt64\include
// - -IC:\msys64\ucrt64\include\c++\14.2.0

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

void Cloudgame::Initialize() {
    // pt::ptree tree;

    // tree.put("name", "something");

    // std::ostringstream data;
    // pt::write_json(data, tree);

    // BOOST_LOG(info) << "data is : " << data.str().c_str();

    try {
        pt::ptree response;



        PerformAPIRequest(response, CLOUDGAME_API_ENDPOINT_USER_PROFILE);
    } catch (const RemoteRequest::Exception& exc) {
        BOOST_LOG(error) << exc.what() << " | " << exc.code();
    }

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