#include <exception>
#include <fstream>
#include <sstream>
#include <stdio.h>

#include "cloudgame.h"
#include "Simple-Web-Server/status_code.hpp"
#include "boost/log/sources/record_ostream.hpp"
#include "boost/property_tree/ptree_fwd.hpp"
#include "config.h"
#include "crypto.h"
#include "curl/curl.h"
#include "curl/easy.h"

// lib includes
#include <Simple-Web-Server/server_http.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/context_base.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <string>

#include "display_device.h"
#include "httpcommon.h"
#include "logging.h"
#include "moonlight-common-c/src/Limelight.h"
#include "network.h"
#include "nvhttp.h"
#include "platform/common.h"
#include "platform/windows/virtual_display.h"
#include "process.h"
#include "rtsp.h"
#include "video.h"

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

void Cloudgame::WriteResponse(HttpResponse& response, pt::ptree& tree, HttpStatusCode statusCode, SimpleWeb::CaseInsensitiveMultimap headers) {
    std::string responseBuffer = PTreeToXml(tree);

    response->write(statusCode, responseBuffer.data(), headers);

    response->close_connection_after_response = true;
}

void Cloudgame::ValidateRequest(HttpRequest& request) {
    HttpStatusCode statusCode = HttpStatusCode::client_error_unauthorized;
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

    auto localEndpoint = request->local_endpoint();

    PTreeSetItem<std::string, int>(tree, "root.<xmlattr>.status_code", 200);
    PTreeSetItem(tree, "root.hostname", config::nvhttp.sunshine_name);

    PTreeSetItem(tree, "root.appversion", VERSION);
    PTreeSetItem(tree, "root.GfeVersion", GFE_VERSION);
    PTreeSetItem(tree, "root.uniqueid", http::unique_id);
    PTreeSetItem<std::string, uint16_t>(tree, "root.HttpsPort", net::map_port(PORT_HTTPS));
    PTreeSetItem<std::string, uint16_t>(tree, "root.ExternalPort", net::map_port(PORT_HTTP));
    PTreeSetItem<std::string, uint16_t>(tree, "root.CloudgamePort", net::map_port(PORT_CLOUDGAME_HTTP));
    PTreeSetItem(tree, "root.MaxLumaPixelsHEVC", video::active_hevc_mode > 1 ? "1869449984" : "0");

    PTreeSetItem(tree, "root.mac", platf::get_mac_address(net::addr_to_normalized_string(localEndpoint.address())));

    pt::ptree& rootNode = tree.get_child("root");

    if (config::sunshine.server_cmds.size() > 0) {
        for (const auto& cmd : config::sunshine.server_cmds) {
            pt::ptree cmdNode;

            cmdNode.put_value(cmd.cmd_name);

            rootNode.push_back(std::make_pair("ServerCommand", cmdNode));
        }
    }

    PTreeSetItem<std::string, int>(tree, "root.Permission", (int)crypto::PERM::_all);

    #ifdef _WIN32
        PTreeSetItem<std::string, bool>(tree, "root.VirtualDisplayCapable", true);
        PTreeSetItem<std::string, bool>(tree, "root.VirtualDisplayDriverReady", proc::vDisplayDriverStatus == VDISPLAY::DRIVER_STATUS::OK);
    #endif

    if (localEndpoint.address().is_v6() && !localEndpoint.address().to_v6().is_v4_mapped()) {
        PTreeSetItem(tree, "root.LocalIP", "127.0.0.1");
    } else {
        PTreeSetItem(tree, "root.LocalIP", net::addr_to_normalized_string(localEndpoint.address()));
    }

    uint32_t codec_mode_flags = SCM_H264;

    if (video::last_encoder_probe_supported_yuv444_for_codec[0]) {
        codec_mode_flags |= SCM_H264_HIGH8_444;
    }

    if (video::active_hevc_mode >= 2) {
        codec_mode_flags |= SCM_HEVC;

        if (video::last_encoder_probe_supported_yuv444_for_codec[1]) {
            codec_mode_flags |= SCM_HEVC_REXT8_444;
        }
    }

    if (video::active_hevc_mode >= 3) {
        codec_mode_flags |= SCM_HEVC_MAIN10;

        if (video::last_encoder_probe_supported_yuv444_for_codec[1]) {
            codec_mode_flags |= SCM_HEVC_REXT10_444;
        }
    }

    if (video::active_av1_mode >= 2) {
        codec_mode_flags |= SCM_AV1_MAIN8;

        if (video::last_encoder_probe_supported_yuv444_for_codec[2]) {
            codec_mode_flags |= SCM_AV1_HIGH8_444;
        }
    }

    if (video::active_av1_mode >= 3) {
        codec_mode_flags |= SCM_AV1_MAIN10;

        if (video::last_encoder_probe_supported_yuv444_for_codec[2]) {
            codec_mode_flags |= SCM_AV1_HIGH10_444;
        }
    }

    PTreeSetItem(tree, "root.ServerCodecModeSupport", codec_mode_flags);

    PTreeSetItem<std::string, int>(tree, "root.PairStatus", 1);

    auto currentAppId = proc::proc.running();

    PTreeSetItem<std::string, int>(tree, "root.currentgame", currentAppId);
    PTreeSetItem(tree, "root.state", currentAppId > 0 ? "SUNSHINE_SERVER_BUSY" : "SUNSHINE_SERVER_FREE");
    
    WriteResponse(response, tree);
SAFE_SCOPE_END(response, tree)

void H app_list(HttpResponse response, HttpRequest request)
SAFE_SCOPE_START(pt::ptree tree)
    ValidateRequest(request);

    auto& apps = tree.add_child("root", pt::ptree());

    PTreeSetItem<std::string, int>(apps, "<xmlattr>.status_code", 200);

    for (auto& app : proc::proc.get_apps()) {
        pt::ptree appNode;

        PTreeSetItem<std::string, int>(appNode, "IsHdrSupported", video::active_hevc_mode == 3 ? 1 : 0);
        PTreeSetItem(appNode, "AppTitle"s, app.name);
        PTreeSetItem(appNode, "UUID", app.uuid);
        PTreeSetItem(appNode, "ID", app.id);

        apps.push_back(std::make_pair("App", std::move(appNode)));
    }
    
    WriteResponse(response, tree);
SAFE_SCOPE_END(response, tree)

void H app_asset(HttpResponse response, HttpRequest request)
SAFE_SCOPE_START(pt::ptree tree)
    ValidateRequest(request);

    auto args = request->parse_query_string();
    auto appImage = proc::proc.get_app_image(util::from_view(get_arg(args, "appid")));

    std::ifstream in(appImage, std::ios::binary);

    SimpleWeb::CaseInsensitiveMultimap headers;
    headers.emplace("Content-Type", "image/png");

    WriteResponse<std::ifstream&>(response, in, HttpStatusCode::success_ok, headers);
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

    PTreeSetItem<std::string, int>(tree, "root.cancel", 1);
    PTreeSetItem<std::string, int>(tree, "root.<xmlattr>.status_code", 200);

    rtsp_stream::terminate_sessions();

    if (proc::proc.running() > 0) {
        proc::proc.terminate();
    }

    display_device::revert_configuration();

    WriteResponse(response, tree);
SAFE_SCOPE_END(response, tree)

void H get_clipboard(HttpResponse response, HttpRequest request)
SAFE_SCOPE_START(pt::ptree tree)
    ValidateRequest(request);

    auto args = request->parse_query_string();
    auto clipboardType = get_arg(args, "type");

    if (clipboardType != "text"sv) {
        throw RemoteRequest::Exception(std::string("Clipboard type [") + clipboardType + "] is not supported!");\
    }

    std::string content = platf::get_clipboard();

    WriteResponse<std::string&>(response, content);
SAFE_SCOPE_END(response, tree)

void H set_clipboard(HttpResponse response, HttpRequest request)
SAFE_SCOPE_START(pt::ptree tree)
    ValidateRequest(request);

    auto args = request->parse_query_string();
    auto clipboardType = get_arg(args, "type");

    if (clipboardType != "text"sv) {
        throw RemoteRequest::Exception(std::string("Clipboard type [") + clipboardType + "] is not supported!");
    }

    std::string content = request->content.string();

    bool success = platf::set_clipboard(content);

    if (!success) {
        throw RemoteRequest::Exception("Setting clipboard failed!");
    }
    
    WriteResponse(response, tree);
SAFE_SCOPE_END(response, tree)

#undef SAFE_SCOPE_START
#undef SAFE_SCOPE_END
#undef SAFE_SCOPE_END_SC

#undef H