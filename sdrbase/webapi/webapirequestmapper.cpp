///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2017 Edouard Griffiths, F4EXB.                                  //
//                                                                               //
// Swagger server adapter interface                                              //
//                                                                               //
// This program is free software; you can redistribute it and/or modify          //
// it under the terms of the GNU General Public License as published by          //
// the Free Software Foundation as version 3 of the License, or                  //
// (at your option) any later version.                                           //
//                                                                               //
// This program is distributed in the hope that it will be useful,               //
// but WITHOUT ANY WARRANTY; without even the implied warranty of                //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                  //
// GNU General Public License V3 for more details.                               //
//                                                                               //
// You should have received a copy of the GNU General Public License             //
// along with this program. If not, see <http://www.gnu.org/licenses/>.          //
///////////////////////////////////////////////////////////////////////////////////

#include <QDirIterator>
#include <QJsonDocument>
#include <QJsonArray>

#include <boost/lexical_cast.hpp>

#include "httpdocrootsettings.h"
#include "webapirequestmapper.h"
#include "SWGInstanceSummaryResponse.h"
#include "SWGInstanceConfigResponse.h"
#include "SWGInstanceDevicesResponse.h"
#include "SWGInstanceChannelsResponse.h"
#include "SWGAudioDevices.h"
#include "SWGLocationInformation.h"
#include "SWGDVSerialDevices.h"
#include "SWGAMBEDevices.h"
#include "SWGPresets.h"
#include "SWGPresetTransfer.h"
#include "SWGPresetIdentifier.h"
#include "SWGPresetImport.h"
#include "SWGPresetExport.h"
#include "SWGDeviceSettings.h"
#include "SWGDeviceState.h"
#include "SWGDeviceReport.h"
#include "SWGChannelsDetail.h"
#include "SWGChannelSettings.h"
#include "SWGChannelReport.h"
#include "SWGSuccessResponse.h"
#include "SWGErrorResponse.h"

const QMap<QString, QString> WebAPIRequestMapper::m_channelURIToSettingsKey = {
    {"sdrangel.channel.bfm", "BFMDemodSettings"}
};

WebAPIRequestMapper::WebAPIRequestMapper(QObject* parent) :
    HttpRequestHandler(parent),
    m_adapter(0)
{
#ifndef _MSC_VER
    Q_INIT_RESOURCE(webapi);
#endif
    qtwebapp::HttpDocrootSettings docrootSettings;
    docrootSettings.path = ":/webapi";
    m_staticFileController = new qtwebapp::StaticFileController(docrootSettings, parent);
}

WebAPIRequestMapper::~WebAPIRequestMapper()
{
    delete m_staticFileController;
#ifndef _MSC_VER
    Q_CLEANUP_RESOURCE(webapi);
#endif
}

void WebAPIRequestMapper::service(qtwebapp::HttpRequest& request, qtwebapp::HttpResponse& response)
{
    if (m_adapter == 0) // format service unavailable if adapter is null
    {
        SWGSDRangel::SWGErrorResponse errorResponse;
        response.setHeader("Content-Type", "application/json");
        response.setHeader("Access-Control-Allow-Origin", "*");
        response.setStatus(500,"Service not available");

        errorResponse.init();
        *errorResponse.getMessage() = "Service not available";
        response.write(errorResponse.asJson().toUtf8());
    }
    else // normal processing
    {
        QByteArray path=request.getPath();

        // Handle pre-flight requests
        if (request.getMethod() == "OPTIONS")
        {
            qDebug("WebAPIRequestMapper::service: method OPTIONS: assume pre-flight");
            response.setHeader("Access-Control-Allow-Origin", "*");
            response.setHeader("Access-Control-Allow-Headers", "*");
            response.setHeader("Access-Control-Allow-Methods", "*");
            response.setStatus(200, "OK");
            return;
        }

        if (path == WebAPIAdapterInterface::instanceSummaryURL) {
            instanceSummaryService(request, response);
        } else if (path == WebAPIAdapterInterface::instanceConfigURL) {
            instanceConfigService(request, response);
        } else if (path == WebAPIAdapterInterface::instanceDevicesURL) {
            instanceDevicesService(request, response);
        } else if (path == WebAPIAdapterInterface::instanceChannelsURL) {
            instanceChannelsService(request, response);
        } else if (path == WebAPIAdapterInterface::instanceLoggingURL) {
            instanceLoggingService(request, response);
        } else if (path == WebAPIAdapterInterface::instanceAudioURL) {
            instanceAudioService(request, response);
        } else if (path == WebAPIAdapterInterface::instanceAudioInputParametersURL) {
            instanceAudioInputParametersService(request, response);
        } else if (path == WebAPIAdapterInterface::instanceAudioOutputParametersURL) {
            instanceAudioOutputParametersService(request, response);
        } else if (path == WebAPIAdapterInterface::instanceAudioInputCleanupURL) {
            instanceAudioInputCleanupService(request, response);
        } else if (path == WebAPIAdapterInterface::instanceAudioOutputCleanupURL) {
            instanceAudioOutputCleanupService(request, response);
        } else if (path == WebAPIAdapterInterface::instanceLocationURL) {
            instanceLocationService(request, response);
        } else if (path == WebAPIAdapterInterface::instanceAMBESerialURL) {
            instanceAMBESerialService(request, response);
        } else if (path == WebAPIAdapterInterface::instanceAMBEDevicesURL) {
            instanceAMBEDevicesService(request, response);
        } else if (path == WebAPIAdapterInterface::instancePresetsURL) {
            instancePresetsService(request, response);
        } else if (path == WebAPIAdapterInterface::instancePresetURL) {
            instancePresetService(request, response);
        } else if (path == WebAPIAdapterInterface::instancePresetFileURL) {
            instancePresetFileService(request, response);
        } else if (path == WebAPIAdapterInterface::instanceDeviceSetsURL) {
            instanceDeviceSetsService(request, response);
        } else if (path == WebAPIAdapterInterface::instanceDeviceSetURL) {
            instanceDeviceSetService(request, response);
        }
        else
        {
            std::smatch desc_match;
            std::string pathStr(path.constData(), path.length());

            if (std::regex_match(pathStr, desc_match, WebAPIAdapterInterface::devicesetURLRe)) {
                devicesetService(std::string(desc_match[1]), request, response);
            } else if (std::regex_match(pathStr, desc_match, WebAPIAdapterInterface::devicesetDeviceURLRe)) {
                devicesetDeviceService(std::string(desc_match[1]), request, response);
            } else if (std::regex_match(pathStr, desc_match, WebAPIAdapterInterface::devicesetFocusURLRe)) {
                devicesetFocusService(std::string(desc_match[1]), request, response);
            } else if (std::regex_match(pathStr, desc_match, WebAPIAdapterInterface::devicesetDeviceSettingsURLRe)) {
                devicesetDeviceSettingsService(std::string(desc_match[1]), request, response);
            } else if (std::regex_match(pathStr, desc_match, WebAPIAdapterInterface::devicesetDeviceRunURLRe)) {
                devicesetDeviceRunService(std::string(desc_match[1]), request, response);
            } else if (std::regex_match(pathStr, desc_match, WebAPIAdapterInterface::devicesetDeviceReportURLRe)) {
                devicesetDeviceReportService(std::string(desc_match[1]), request, response);
            } else if (std::regex_match(pathStr, desc_match, WebAPIAdapterInterface::devicesetChannelsReportURLRe)) {
                devicesetChannelsReportService(std::string(desc_match[1]), request, response);
            } else if (std::regex_match(pathStr, desc_match, WebAPIAdapterInterface::devicesetChannelURLRe)) {
                devicesetChannelService(std::string(desc_match[1]), request, response);
            } else if (std::regex_match(pathStr, desc_match, WebAPIAdapterInterface::devicesetChannelIndexURLRe)) {
                devicesetChannelIndexService(std::string(desc_match[1]), std::string(desc_match[2]), request, response);
            } else if (std::regex_match(pathStr, desc_match, WebAPIAdapterInterface::devicesetChannelSettingsURLRe)) {
                devicesetChannelSettingsService(std::string(desc_match[1]), std::string(desc_match[2]), request, response);
            } else if (std::regex_match(pathStr, desc_match, WebAPIAdapterInterface::devicesetChannelReportURLRe)) {
                devicesetChannelReportService(std::string(desc_match[1]), std::string(desc_match[2]), request, response);
            }
            else // serve static documentation pages
            {
                m_staticFileController->service(request, response);
            }

//            QDirIterator it(":", QDirIterator::Subdirectories);
//            while (it.hasNext()) {
//                qDebug() << "WebAPIRequestMapper::service: " << it.next();
//            }

        }
    }
}

void WebAPIRequestMapper::instanceSummaryService(qtwebapp::HttpRequest& request, qtwebapp::HttpResponse& response)
{
    SWGSDRangel::SWGErrorResponse errorResponse;
    response.setHeader("Content-Type", "application/json");
    response.setHeader("Access-Control-Allow-Origin", "*");

    if (request.getMethod() == "GET")
    {
        SWGSDRangel::SWGInstanceSummaryResponse normalResponse;

        int status = m_adapter->instanceSummary(normalResponse, errorResponse);
        response.setStatus(status);

        if (status/100 == 2) {
            response.write(normalResponse.asJson().toUtf8());
        } else {
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    else if (request.getMethod() == "DELETE")
    {
        SWGSDRangel::SWGSuccessResponse normalResponse;

        int status = m_adapter->instanceDelete(normalResponse, errorResponse);
        response.setStatus(status);

        if (status/100 == 2) {
            response.write(normalResponse.asJson().toUtf8());
        } else {
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    else
    {
        response.setStatus(405,"Invalid HTTP method");
        errorResponse.init();
        *errorResponse.getMessage() = "Invalid HTTP method";
        response.write(errorResponse.asJson().toUtf8());
    }
}

void WebAPIRequestMapper::instanceConfigService(qtwebapp::HttpRequest& request, qtwebapp::HttpResponse& response)
{
    SWGSDRangel::SWGInstanceConfigResponse query;
    SWGSDRangel::SWGSuccessResponse normalResponse;
    SWGSDRangel::SWGErrorResponse errorResponse;
    response.setHeader("Content-Type", "application/json");
    response.setHeader("Access-Control-Allow-Origin", "*");

    if (request.getMethod() == "GET")
    {
        SWGSDRangel::SWGInstanceConfigResponse normalResponse;
        int status = m_adapter->instanceConfigGet(normalResponse, errorResponse);
        response.setStatus(status);

        if (status/100 == 2) {
            response.write(normalResponse.asJson().toUtf8());
        } else {
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    else if (request.getMethod() == "PUT")
    {
        QString jsonStr = request.getBody();
        QJsonObject jsonObject;

        if (parseJsonBody(jsonStr, jsonObject, response))
        {
            // TODO
        }
        else
        {
            response.setStatus(400,"Invalid JSON format");
            errorResponse.init();
            *errorResponse.getMessage() = "Invalid JSON format";
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    else
    {
        response.setStatus(405,"Invalid HTTP method");
        errorResponse.init();
        *errorResponse.getMessage() = "Invalid HTTP method";
        response.write(errorResponse.asJson().toUtf8());
    }
}

void WebAPIRequestMapper::instanceDevicesService(qtwebapp::HttpRequest& request, qtwebapp::HttpResponse& response)
{
    SWGSDRangel::SWGInstanceDevicesResponse normalResponse;
    SWGSDRangel::SWGErrorResponse errorResponse;
    response.setHeader("Content-Type", "application/json");
    response.setHeader("Access-Control-Allow-Origin", "*");

    if (request.getMethod() == "GET")
    {
        QByteArray dirStr = request.getParameter("direction");
        int direction = 0;

        if (dirStr.length() != 0)
        {
            bool ok;
            int tmp = dirStr.toInt(&ok);
            if (ok) {
                direction = tmp;
            }
        }

        int status = m_adapter->instanceDevices(direction, normalResponse, errorResponse);
        response.setStatus(status);

        if (status/100 == 2) {
            response.write(normalResponse.asJson().toUtf8());
        } else {
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    else
    {
        response.setStatus(405,"Invalid HTTP method");
        errorResponse.init();
        *errorResponse.getMessage() = "Invalid HTTP method";
        response.write(errorResponse.asJson().toUtf8());
    }
}

void WebAPIRequestMapper::instanceChannelsService(qtwebapp::HttpRequest& request, qtwebapp::HttpResponse& response)
{
    SWGSDRangel::SWGInstanceChannelsResponse normalResponse;
    SWGSDRangel::SWGErrorResponse errorResponse;
    response.setHeader("Content-Type", "application/json");
    response.setHeader("Access-Control-Allow-Origin", "*");

    if (request.getMethod() == "GET")
    {
        QByteArray dirStr = request.getParameter("direction");
        int direction = 0;

        if (dirStr.length() != 0)
        {
            bool ok;
            int tmp = dirStr.toInt(&ok);
            if (ok) {
                direction = tmp;
            }
        }

        int status = m_adapter->instanceChannels(direction, normalResponse, errorResponse);
        response.setStatus(status);

        if (status/100 == 2) {
            response.write(normalResponse.asJson().toUtf8());
        } else {
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    else
    {
        response.setStatus(405,"Invalid HTTP method");
        errorResponse.init();
        *errorResponse.getMessage() = "Invalid HTTP method";
        response.write(errorResponse.asJson().toUtf8());
    }
}

void WebAPIRequestMapper::instanceLoggingService(qtwebapp::HttpRequest& request, qtwebapp::HttpResponse& response)
{
    SWGSDRangel::SWGLoggingInfo query;
    SWGSDRangel::SWGLoggingInfo normalResponse;
    SWGSDRangel::SWGErrorResponse errorResponse;
    response.setHeader("Content-Type", "application/json");
    response.setHeader("Access-Control-Allow-Origin", "*");

    if (request.getMethod() == "GET")
    {
        int status = m_adapter->instanceLoggingGet(normalResponse, errorResponse);
        response.setStatus(status);

        if (status/100 == 2) {
            response.write(normalResponse.asJson().toUtf8());
        } else {
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    else if (request.getMethod() == "PUT")
    {
        QString jsonStr = request.getBody();
        QJsonObject jsonObject;

        if (parseJsonBody(jsonStr, jsonObject, response))
        {
            query.fromJson(jsonStr);
            int status = m_adapter->instanceLoggingPut(query, normalResponse, errorResponse);
            response.setStatus(status);

            if (status/100 == 2) {
                response.write(normalResponse.asJson().toUtf8());
            } else {
                response.write(errorResponse.asJson().toUtf8());
            }
        }
        else
        {
            response.setStatus(400,"Invalid JSON format");
            errorResponse.init();
            *errorResponse.getMessage() = "Invalid JSON format";
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    else
    {
        response.setStatus(405,"Invalid HTTP method");
        errorResponse.init();
        *errorResponse.getMessage() = "Invalid HTTP method";
        response.write(errorResponse.asJson().toUtf8());
    }
}

void WebAPIRequestMapper::instanceAudioService(qtwebapp::HttpRequest& request, qtwebapp::HttpResponse& response)
{
    SWGSDRangel::SWGErrorResponse errorResponse;
    response.setHeader("Content-Type", "application/json");
    response.setHeader("Access-Control-Allow-Origin", "*");

    if (request.getMethod() == "GET")
    {
        SWGSDRangel::SWGAudioDevices normalResponse;

        int status = m_adapter->instanceAudioGet(normalResponse, errorResponse);
        response.setStatus(status);

        if (status/100 == 2) {
            response.write(normalResponse.asJson().toUtf8());
        } else {
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    else
    {
        response.setStatus(405,"Invalid HTTP method");
        errorResponse.init();
        *errorResponse.getMessage() = "Invalid HTTP method";
        response.write(errorResponse.asJson().toUtf8());
    }
}

void WebAPIRequestMapper::instanceAudioInputParametersService(qtwebapp::HttpRequest& request, qtwebapp::HttpResponse& response)
{
    SWGSDRangel::SWGErrorResponse errorResponse;
    response.setHeader("Content-Type", "application/json");
    response.setHeader("Access-Control-Allow-Origin", "*");

    QString jsonStr = request.getBody();
    QJsonObject jsonObject;

    if (parseJsonBody(jsonStr, jsonObject, response))
    {
        SWGSDRangel::SWGAudioInputDevice normalResponse;
        resetAudioInputDevice(normalResponse);
        QStringList audioInputDeviceKeys;

        if (validateAudioInputDevice(normalResponse, jsonObject, audioInputDeviceKeys))
        {
            if (request.getMethod() == "PATCH")
            {
                int status = m_adapter->instanceAudioInputPatch(
                        normalResponse,
                        audioInputDeviceKeys,
                        errorResponse);
                response.setStatus(status);

                if (status/100 == 2) {
                    response.write(normalResponse.asJson().toUtf8());
                } else {
                    response.write(errorResponse.asJson().toUtf8());
                }
            }
            else if (request.getMethod() == "DELETE")
            {
                int status = m_adapter->instanceAudioInputDelete(
                        normalResponse,
                        errorResponse);
                response.setStatus(status);

                if (status/100 == 2) {
                    response.write(normalResponse.asJson().toUtf8());
                } else {
                    response.write(errorResponse.asJson().toUtf8());
                }
            }
            else
            {
                response.setStatus(405,"Invalid HTTP method");
                errorResponse.init();
                *errorResponse.getMessage() = "Invalid HTTP method";
                response.write(errorResponse.asJson().toUtf8());
            }
        }
        else
        {
            response.setStatus(400,"Invalid JSON request");
            errorResponse.init();
            *errorResponse.getMessage() = "Invalid JSON request";
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    else
    {
        response.setStatus(400,"Invalid JSON format");
        errorResponse.init();
        *errorResponse.getMessage() = "Invalid JSON format";
        response.write(errorResponse.asJson().toUtf8());
    }
}

void WebAPIRequestMapper::instanceAudioOutputParametersService(qtwebapp::HttpRequest& request, qtwebapp::HttpResponse& response)
{
    SWGSDRangel::SWGErrorResponse errorResponse;
    response.setHeader("Content-Type", "application/json");
    response.setHeader("Access-Control-Allow-Origin", "*");

    QString jsonStr = request.getBody();
    QJsonObject jsonObject;

    if (parseJsonBody(jsonStr, jsonObject, response))
    {
        SWGSDRangel::SWGAudioOutputDevice normalResponse;
        resetAudioOutputDevice(normalResponse);
        QStringList audioOutputDeviceKeys;

        if (validateAudioOutputDevice(normalResponse, jsonObject, audioOutputDeviceKeys))
        {
            if (request.getMethod() == "PATCH")
            {
                int status = m_adapter->instanceAudioOutputPatch(
                        normalResponse,
                        audioOutputDeviceKeys,
                        errorResponse);
                response.setStatus(status);

                if (status/100 == 2) {
                    response.write(normalResponse.asJson().toUtf8());
                } else {
                    response.write(errorResponse.asJson().toUtf8());
                }
            }
            else if (request.getMethod() == "DELETE")
            {
                int status = m_adapter->instanceAudioOutputDelete(
                        normalResponse,
                        errorResponse);
                response.setStatus(status);

                if (status/100 == 2) {
                    response.write(normalResponse.asJson().toUtf8());
                } else {
                    response.write(errorResponse.asJson().toUtf8());
                }
            }
            else
            {
                response.setStatus(405,"Invalid HTTP method");
                errorResponse.init();
                *errorResponse.getMessage() = "Invalid HTTP method";
                response.write(errorResponse.asJson().toUtf8());
            }
        }
        else
        {
            response.setStatus(400,"Invalid JSON request");
            errorResponse.init();
            *errorResponse.getMessage() = "Invalid JSON request";
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    else
    {
        response.setStatus(400,"Invalid JSON format");
        errorResponse.init();
        *errorResponse.getMessage() = "Invalid JSON format";
        response.write(errorResponse.asJson().toUtf8());
    }
}

void WebAPIRequestMapper::instanceAudioInputCleanupService(qtwebapp::HttpRequest& request, qtwebapp::HttpResponse& response)
{
    SWGSDRangel::SWGErrorResponse errorResponse;
    response.setHeader("Content-Type", "application/json");
    response.setHeader("Access-Control-Allow-Origin", "*");

    if (request.getMethod() == "PATCH")
    {
        SWGSDRangel::SWGSuccessResponse normalResponse;

        int status = m_adapter->instanceAudioInputCleanupPatch(normalResponse, errorResponse);
        response.setStatus(status);

        if (status/100 == 2) {
            response.write(normalResponse.asJson().toUtf8());
        } else {
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    else
    {
        response.setStatus(405,"Invalid HTTP method");
        errorResponse.init();
        *errorResponse.getMessage() = "Invalid HTTP method";
        response.write(errorResponse.asJson().toUtf8());
    }
}

void WebAPIRequestMapper::instanceAudioOutputCleanupService(qtwebapp::HttpRequest& request, qtwebapp::HttpResponse& response)
{
    SWGSDRangel::SWGErrorResponse errorResponse;
    response.setHeader("Content-Type", "application/json");
    response.setHeader("Access-Control-Allow-Origin", "*");

    if (request.getMethod() == "PATCH")
    {
        SWGSDRangel::SWGSuccessResponse normalResponse;

        int status = m_adapter->instanceAudioOutputCleanupPatch(normalResponse, errorResponse);
        response.setStatus(status);

        if (status/100 == 2) {
            response.write(normalResponse.asJson().toUtf8());
        } else {
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    else
    {
        response.setStatus(405,"Invalid HTTP method");
        errorResponse.init();
        *errorResponse.getMessage() = "Invalid HTTP method";
        response.write(errorResponse.asJson().toUtf8());
    }
}

void WebAPIRequestMapper::instanceLocationService(qtwebapp::HttpRequest& request, qtwebapp::HttpResponse& response)
{
    SWGSDRangel::SWGErrorResponse errorResponse;
    response.setHeader("Content-Type", "application/json");
    response.setHeader("Access-Control-Allow-Origin", "*");

    if (request.getMethod() == "GET")
    {
        SWGSDRangel::SWGLocationInformation normalResponse;

        int status = m_adapter->instanceLocationGet(normalResponse, errorResponse);
        response.setStatus(status);

        if (status/100 == 2) {
            response.write(normalResponse.asJson().toUtf8());
        } else {
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    else if (request.getMethod() == "PUT")
    {
        SWGSDRangel::SWGLocationInformation normalResponse;
        QString jsonStr = request.getBody();
        QJsonObject jsonObject;

        if (parseJsonBody(jsonStr, jsonObject, response))
        {
            normalResponse.fromJson(jsonStr);
            int status = m_adapter->instanceLocationPut(normalResponse, errorResponse);
            response.setStatus(status);

            if (status/100 == 2) {
                response.write(normalResponse.asJson().toUtf8());
            } else {
                response.write(errorResponse.asJson().toUtf8());
            }
        }
        else
        {
            response.setStatus(400,"Invalid JSON format");
            errorResponse.init();
            *errorResponse.getMessage() = "Invalid JSON format";
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    else
    {
        response.setStatus(405,"Invalid HTTP method");
        errorResponse.init();
        *errorResponse.getMessage() = "Invalid HTTP method";
        response.write(errorResponse.asJson().toUtf8());
    }
}

void WebAPIRequestMapper::instanceAMBESerialService(qtwebapp::HttpRequest& request, qtwebapp::HttpResponse& response)
{
    SWGSDRangel::SWGErrorResponse errorResponse;
    response.setHeader("Content-Type", "application/json");
    response.setHeader("Access-Control-Allow-Origin", "*");

    if (request.getMethod() == "GET")
    {
        SWGSDRangel::SWGDVSerialDevices normalResponse;

        int status = m_adapter->instanceAMBESerialGet(normalResponse, errorResponse);
        response.setStatus(status);

        if (status/100 == 2) {
            response.write(normalResponse.asJson().toUtf8());
        } else {
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    else
    {
        response.setStatus(405,"Invalid HTTP method");
        errorResponse.init();
        *errorResponse.getMessage() = "Invalid HTTP method";
        response.write(errorResponse.asJson().toUtf8());
    }
}

void WebAPIRequestMapper::instanceAMBEDevicesService(qtwebapp::HttpRequest& request, qtwebapp::HttpResponse& response)
{
    SWGSDRangel::SWGErrorResponse errorResponse;
    response.setHeader("Content-Type", "application/json");
    response.setHeader("Access-Control-Allow-Origin", "*");

    if (request.getMethod() == "GET")
    {
        SWGSDRangel::SWGAMBEDevices normalResponse;

        int status = m_adapter->instanceAMBEDevicesGet(normalResponse, errorResponse);
        response.setStatus(status);

        if (status/100 == 2) {
            response.write(normalResponse.asJson().toUtf8());
        } else {
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    else if ((request.getMethod() == "PATCH") || (request.getMethod() == "PUT"))
    {
        SWGSDRangel::SWGAMBEDevices query;
        SWGSDRangel::SWGAMBEDevices normalResponse;
        QString jsonStr = request.getBody();
        QJsonObject jsonObject;

        if (parseJsonBody(jsonStr, jsonObject, response))
        {
            if (validateAMBEDevices(query, jsonObject))
            {
                int status;

                if (request.getMethod() == "PATCH") {
                    status = m_adapter->instanceAMBEDevicesPatch(query, normalResponse, errorResponse);
                } else {
                    status = m_adapter->instanceAMBEDevicesPut(query, normalResponse, errorResponse);
                }

                response.setStatus(status);

                if (status/100 == 2) {
                    response.write(normalResponse.asJson().toUtf8());
                } else {
                    response.write(errorResponse.asJson().toUtf8());
                }
            }
            else
            {
                response.setStatus(400,"Invalid JSON request");
                errorResponse.init();
                *errorResponse.getMessage() = "Invalid JSON request";
                response.write(errorResponse.asJson().toUtf8());
            }
        }
        else
        {
            response.setStatus(400,"Invalid JSON format");
            errorResponse.init();
            *errorResponse.getMessage() = "Invalid JSON format";
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    else if (request.getMethod() == "DELETE")
    {
        SWGSDRangel::SWGSuccessResponse normalResponse;

        int status = m_adapter->instanceAMBEDevicesDelete(normalResponse, errorResponse);
        response.setStatus(status);

        if (status/100 == 2) {
            response.write(normalResponse.asJson().toUtf8());
        } else {
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    else
    {
        response.setStatus(405,"Invalid HTTP method");
        errorResponse.init();
        *errorResponse.getMessage() = "Invalid HTTP method";
        response.write(errorResponse.asJson().toUtf8());
    }
}

void WebAPIRequestMapper::instancePresetsService(qtwebapp::HttpRequest& request, qtwebapp::HttpResponse& response)
{
    SWGSDRangel::SWGErrorResponse errorResponse;
    response.setHeader("Content-Type", "application/json");
    response.setHeader("Access-Control-Allow-Origin", "*");

    if (request.getMethod() == "GET")
    {
        SWGSDRangel::SWGPresets normalResponse;
        int status = m_adapter->instancePresetsGet(normalResponse, errorResponse);
        response.setStatus(status);

        if (status/100 == 2) {
            response.write(normalResponse.asJson().toUtf8());
        } else {
            response.write(errorResponse.asJson().toUtf8());
        }
    }
}

void WebAPIRequestMapper::instancePresetService(qtwebapp::HttpRequest& request, qtwebapp::HttpResponse& response)
{
    SWGSDRangel::SWGErrorResponse errorResponse;
    response.setHeader("Content-Type", "application/json");
    response.setHeader("Access-Control-Allow-Origin", "*");

    if (request.getMethod() == "PATCH")
    {
        SWGSDRangel::SWGPresetTransfer query;
        SWGSDRangel::SWGPresetIdentifier normalResponse;
        QString jsonStr = request.getBody();
        QJsonObject jsonObject;

        if (parseJsonBody(jsonStr, jsonObject, response))
        {
            query.fromJson(jsonStr);

            if (validatePresetTransfer(query))
            {
                int status = m_adapter->instancePresetPatch(query, normalResponse, errorResponse);
                response.setStatus(status);

                if (status/100 == 2) {
                    response.write(normalResponse.asJson().toUtf8());
                } else {
                    response.write(errorResponse.asJson().toUtf8());
                }
            }
            else
            {
                response.setStatus(400,"Invalid JSON request");
                errorResponse.init();
                *errorResponse.getMessage() = "Invalid JSON request";
                response.write(errorResponse.asJson().toUtf8());
            }
        }
        else
        {
            response.setStatus(400,"Invalid JSON format");
            errorResponse.init();
            *errorResponse.getMessage() = "Invalid JSON format";
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    else if (request.getMethod() == "PUT")
    {
        SWGSDRangel::SWGPresetTransfer query;
        SWGSDRangel::SWGPresetIdentifier normalResponse;
        QString jsonStr = request.getBody();
        QJsonObject jsonObject;

        if (parseJsonBody(jsonStr, jsonObject, response))
        {
            query.fromJson(jsonStr);

            if (validatePresetTransfer(query))
            {
                int status = m_adapter->instancePresetPut(query, normalResponse, errorResponse);
                response.setStatus(status);

                if (status/100 == 2) {
                    response.write(normalResponse.asJson().toUtf8());
                } else {
                    response.write(errorResponse.asJson().toUtf8());
                }
            }
            else
            {
                response.setStatus(400,"Invalid JSON request");
                errorResponse.init();
                *errorResponse.getMessage() = "Invalid JSON request";
                response.write(errorResponse.asJson().toUtf8());
            }
        }
        else
        {
            response.setStatus(400,"Invalid JSON format");
            errorResponse.init();
            *errorResponse.getMessage() = "Invalid JSON format";
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    else if (request.getMethod() == "POST")
    {
        SWGSDRangel::SWGPresetTransfer query;
        SWGSDRangel::SWGPresetIdentifier normalResponse;
        QString jsonStr = request.getBody();
        QJsonObject jsonObject;

        if (parseJsonBody(jsonStr, jsonObject, response))
        {
            query.fromJson(jsonStr);

            if (validatePresetTransfer(query))
            {
                int status = m_adapter->instancePresetPost(query, normalResponse, errorResponse);
                response.setStatus(status);

                if (status/100 == 2) {
                    response.write(normalResponse.asJson().toUtf8());
                } else {
                    response.write(errorResponse.asJson().toUtf8());
                }
            }
            else
            {
                response.setStatus(400,"Invalid JSON request");
                errorResponse.init();
                *errorResponse.getMessage() = "Invalid JSON request";
                response.write(errorResponse.asJson().toUtf8());
            }
        }
        else
        {
            response.setStatus(400,"Invalid JSON format");
            errorResponse.init();
            *errorResponse.getMessage() = "Invalid JSON format";
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    else if (request.getMethod() == "DELETE")
    {
        SWGSDRangel::SWGPresetIdentifier normalResponse;
        QString jsonStr = request.getBody();
        QJsonObject jsonObject;

        if (parseJsonBody(jsonStr, jsonObject, response))
        {
            normalResponse.fromJson(jsonStr);

            if (validatePresetIdentifer(normalResponse))
            {
                int status = m_adapter->instancePresetDelete(normalResponse, errorResponse);
                response.setStatus(status);

                if (status/100 == 2) {
                    response.write(normalResponse.asJson().toUtf8());
                } else {
                    response.write(errorResponse.asJson().toUtf8());
                }
            }
            else
            {
                response.setStatus(400,"Invalid JSON request");
                errorResponse.init();
                *errorResponse.getMessage() = "Invalid JSON request";
                response.write(errorResponse.asJson().toUtf8());
            }
        }
        else
        {
            response.setStatus(400,"Invalid JSON format");
            errorResponse.init();
            *errorResponse.getMessage() = "Invalid JSON format";
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    else
    {
        response.setStatus(405,"Invalid HTTP method");
        errorResponse.init();
        *errorResponse.getMessage() = "Invalid HTTP method";
        response.write(errorResponse.asJson().toUtf8());
    }
}

void WebAPIRequestMapper::instancePresetFileService(qtwebapp::HttpRequest& request, qtwebapp::HttpResponse& response)
{
    SWGSDRangel::SWGErrorResponse errorResponse;
    response.setHeader("Content-Type", "application/json");
    response.setHeader("Access-Control-Allow-Origin", "*");

    if (request.getMethod() == "PUT")
    {
        SWGSDRangel::SWGPresetImport query;
        SWGSDRangel::SWGPresetIdentifier normalResponse;
        QString jsonStr = request.getBody();
        QJsonObject jsonObject;

        if (parseJsonBody(jsonStr, jsonObject, response))
        {
            query.fromJson(jsonStr);

            if (query.getFilePath())
            {
                int status = m_adapter->instancePresetFilePut(query, normalResponse, errorResponse);
                response.setStatus(status);

                if (status/100 == 2) {
                    response.write(normalResponse.asJson().toUtf8());
                } else {
                    response.write(errorResponse.asJson().toUtf8());
                }
            }
            else
            {
                response.setStatus(400,"Invalid JSON request");
                errorResponse.init();
                *errorResponse.getMessage() = "Invalid JSON request";
                response.write(errorResponse.asJson().toUtf8());
            }
        }
        else
        {
            response.setStatus(400,"Invalid JSON format");
            errorResponse.init();
            *errorResponse.getMessage() = "Invalid JSON format";
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    else if (request.getMethod() == "POST")
    {
        SWGSDRangel::SWGPresetExport query;
        SWGSDRangel::SWGPresetIdentifier normalResponse;
        QString jsonStr = request.getBody();
        QJsonObject jsonObject;

        if (parseJsonBody(jsonStr, jsonObject, response))
        {
            query.fromJson(jsonStr);

            if (validatePresetExport(query))
            {
                int status = m_adapter->instancePresetFilePost(query, normalResponse, errorResponse);
                response.setStatus(status);

                if (status/100 == 2) {
                    response.write(normalResponse.asJson().toUtf8());
                } else {
                    response.write(errorResponse.asJson().toUtf8());
                }
            }
            else
            {
                response.setStatus(400,"Invalid JSON request");
                errorResponse.init();
                *errorResponse.getMessage() = "Invalid JSON request";
                response.write(errorResponse.asJson().toUtf8());
            }
        }
        else
        {
            response.setStatus(400,"Invalid JSON format");
            errorResponse.init();
            *errorResponse.getMessage() = "Invalid JSON format";
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    else
    {
        response.setStatus(405,"Invalid HTTP method");
        errorResponse.init();
        *errorResponse.getMessage() = "Invalid HTTP method";
        response.write(errorResponse.asJson().toUtf8());
    }
}

void WebAPIRequestMapper::instanceDeviceSetsService(qtwebapp::HttpRequest& request, qtwebapp::HttpResponse& response)
{
    SWGSDRangel::SWGErrorResponse errorResponse;
    response.setHeader("Content-Type", "application/json");
    response.setHeader("Access-Control-Allow-Origin", "*");

    if (request.getMethod() == "GET")
    {
        SWGSDRangel::SWGDeviceSetList normalResponse;
        int status = m_adapter->instanceDeviceSetsGet(normalResponse, errorResponse);
        response.setStatus(status);

        if (status/100 == 2) {
            response.write(normalResponse.asJson().toUtf8());
        } else {
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    else
    {
        response.setStatus(405,"Invalid HTTP method");
        errorResponse.init();
        *errorResponse.getMessage() = "Invalid HTTP method";
        response.write(errorResponse.asJson().toUtf8());
    }
}

void WebAPIRequestMapper::instanceDeviceSetService(qtwebapp::HttpRequest& request, qtwebapp::HttpResponse& response)
{
    SWGSDRangel::SWGErrorResponse errorResponse;
    response.setHeader("Content-Type", "application/json");
    response.setHeader("Access-Control-Allow-Origin", "*");

    if (request.getMethod() == "POST")
    {
        SWGSDRangel::SWGSuccessResponse normalResponse;
        QByteArray dirStr = request.getParameter("direction");
        int direction = 0;

        if (dirStr.length() != 0)
        {
            bool ok;
            int tmp = dirStr.toInt(&ok);
            if (ok) {
                direction = tmp;
            }
        }

        int status = m_adapter->instanceDeviceSetPost(direction, normalResponse, errorResponse);
        response.setStatus(status);

        if (status/100 == 2) {
            response.write(normalResponse.asJson().toUtf8());
        } else {
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    else if (request.getMethod() == "DELETE")
    {
        SWGSDRangel::SWGSuccessResponse normalResponse;
        int status = m_adapter->instanceDeviceSetDelete(normalResponse, errorResponse);
        response.setStatus(status);

        if (status/100 == 2) {
            response.write(normalResponse.asJson().toUtf8());
        } else {
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    else
    {
        response.setStatus(405,"Invalid HTTP method");
        errorResponse.init();
        *errorResponse.getMessage() = "Invalid HTTP method";
        response.write(errorResponse.asJson().toUtf8());
    }
}

void WebAPIRequestMapper::devicesetService(const std::string& indexStr, qtwebapp::HttpRequest& request, qtwebapp::HttpResponse& response)
{
    SWGSDRangel::SWGErrorResponse errorResponse;
    response.setHeader("Content-Type", "application/json");
    response.setHeader("Access-Control-Allow-Origin", "*");

    if (request.getMethod() == "GET")
    {
        try
        {
            SWGSDRangel::SWGDeviceSet normalResponse;
            int deviceSetIndex = boost::lexical_cast<int>(indexStr);
            int status = m_adapter->devicesetGet(deviceSetIndex, normalResponse, errorResponse);
            response.setStatus(status);

            if (status/100 == 2) {
                response.write(normalResponse.asJson().toUtf8());
            } else {
                response.write(errorResponse.asJson().toUtf8());
            }
        }
        catch (const boost::bad_lexical_cast &e)
        {
            errorResponse.init();
            *errorResponse.getMessage() = "Wrong integer conversion on device set index";
            response.setStatus(400,"Invalid data");
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    else
    {
        response.setStatus(405,"Invalid HTTP method");
        errorResponse.init();
        *errorResponse.getMessage() = "Invalid HTTP method";
        response.write(errorResponse.asJson().toUtf8());
    }
}

void WebAPIRequestMapper::devicesetFocusService(const std::string& indexStr, qtwebapp::HttpRequest& request, qtwebapp::HttpResponse& response)
{
    SWGSDRangel::SWGErrorResponse errorResponse;
    response.setHeader("Content-Type", "application/json");
    response.setHeader("Access-Control-Allow-Origin", "*");

    try
    {
        int deviceSetIndex = boost::lexical_cast<int>(indexStr);

        if (request.getMethod() == "PATCH")
        {
            SWGSDRangel::SWGSuccessResponse normalResponse;
            int status = m_adapter->devicesetFocusPatch(deviceSetIndex, normalResponse, errorResponse);

            response.setStatus(status);

            if (status/100 == 2) {
                response.write(normalResponse.asJson().toUtf8());
            } else {
                response.write(errorResponse.asJson().toUtf8());
            }
        }
        else
        {
            response.setStatus(405,"Invalid HTTP method");
            errorResponse.init();
            *errorResponse.getMessage() = "Invalid HTTP method";
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    catch (const boost::bad_lexical_cast &e)
    {
        errorResponse.init();
        *errorResponse.getMessage() = "Wrong integer conversion on device set index";
        response.setStatus(400,"Invalid data");
        response.write(errorResponse.asJson().toUtf8());
    }
}

void WebAPIRequestMapper::devicesetDeviceService(const std::string& indexStr, qtwebapp::HttpRequest& request, qtwebapp::HttpResponse& response)
{
    SWGSDRangel::SWGErrorResponse errorResponse;
    response.setHeader("Content-Type", "application/json");
    response.setHeader("Access-Control-Allow-Origin", "*");

    try
    {
        int deviceSetIndex = boost::lexical_cast<int>(indexStr);

        if (request.getMethod() == "PUT")
        {
            QString jsonStr = request.getBody();
            QJsonObject jsonObject;

            if (parseJsonBody(jsonStr, jsonObject, response))
            {
                SWGSDRangel::SWGDeviceListItem query;
                SWGSDRangel::SWGDeviceListItem normalResponse;

                if (validateDeviceListItem(query, jsonObject))
                {
                    int status = m_adapter->devicesetDevicePut(deviceSetIndex, query, normalResponse, errorResponse);
                    response.setStatus(status);

                    if (status/100 == 2) {
                        response.write(normalResponse.asJson().toUtf8());
                    } else {
                        response.write(errorResponse.asJson().toUtf8());
                    }
                }
                else
                {
                    response.setStatus(400,"Missing device identification");
                    errorResponse.init();
                    *errorResponse.getMessage() = "Missing device identification";
                    response.write(errorResponse.asJson().toUtf8());
                }
            }
            else
            {
                response.setStatus(400,"Invalid JSON format");
                errorResponse.init();
                *errorResponse.getMessage() = "Invalid JSON format";
                response.write(errorResponse.asJson().toUtf8());
            }
        }
        else
        {
            response.setStatus(405,"Invalid HTTP method");
            errorResponse.init();
            *errorResponse.getMessage() = "Invalid HTTP method";
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    catch (const boost::bad_lexical_cast &e)
    {
        errorResponse.init();
        *errorResponse.getMessage() = "Wrong integer conversion on device set index";
        response.setStatus(400,"Invalid data");
        response.write(errorResponse.asJson().toUtf8());
    }
}

void WebAPIRequestMapper::devicesetDeviceSettingsService(const std::string& indexStr, qtwebapp::HttpRequest& request, qtwebapp::HttpResponse& response)
{
    SWGSDRangel::SWGErrorResponse errorResponse;
    response.setHeader("Content-Type", "application/json");
    response.setHeader("Access-Control-Allow-Origin", "*");

    try
    {
        int deviceSetIndex = boost::lexical_cast<int>(indexStr);

        if ((request.getMethod() == "PUT") || (request.getMethod() == "PATCH"))
        {
            QString jsonStr = request.getBody();
            QJsonObject jsonObject;

            if (parseJsonBody(jsonStr, jsonObject, response))
            {
                SWGSDRangel::SWGDeviceSettings normalResponse;
                resetDeviceSettings(normalResponse);
                QStringList deviceSettingsKeys;

                if (validateDeviceSettings(normalResponse, jsonObject, deviceSettingsKeys))
                {
                    int status = m_adapter->devicesetDeviceSettingsPutPatch(
                            deviceSetIndex,
                            (request.getMethod() == "PUT"), // force settings on PUT
                            deviceSettingsKeys,
                            normalResponse,
                            errorResponse);
                    response.setStatus(status);

                    if (status/100 == 2) {
                        response.write(normalResponse.asJson().toUtf8());
                    } else {
                        response.write(errorResponse.asJson().toUtf8());
                    }
                }
                else
                {
                    response.setStatus(400,"Invalid JSON request");
                    errorResponse.init();
                    *errorResponse.getMessage() = "Invalid JSON request";
                    response.write(errorResponse.asJson().toUtf8());
                }
            }
            else
            {
                response.setStatus(400,"Invalid JSON format");
                errorResponse.init();
                *errorResponse.getMessage() = "Invalid JSON format";
                response.write(errorResponse.asJson().toUtf8());
            }
        }
        else if (request.getMethod() == "GET")
        {
            SWGSDRangel::SWGDeviceSettings normalResponse;
            resetDeviceSettings(normalResponse);
            int status = m_adapter->devicesetDeviceSettingsGet(deviceSetIndex, normalResponse, errorResponse);
            response.setStatus(status);

            if (status/100 == 2) {
                response.write(normalResponse.asJson().toUtf8());
            } else {
                response.write(errorResponse.asJson().toUtf8());
            }
        }
        else
        {
            response.setStatus(405,"Invalid HTTP method");
            errorResponse.init();
            *errorResponse.getMessage() = "Invalid HTTP method";
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    catch (const boost::bad_lexical_cast &e)
    {
        errorResponse.init();
        *errorResponse.getMessage() = "Wrong integer conversion on device set index";
        response.setStatus(400,"Invalid data");
        response.write(errorResponse.asJson().toUtf8());
    }
}

void WebAPIRequestMapper::devicesetDeviceRunService(const std::string& indexStr, qtwebapp::HttpRequest& request, qtwebapp::HttpResponse& response)
{
    SWGSDRangel::SWGErrorResponse errorResponse;
    response.setHeader("Content-Type", "application/json");
    response.setHeader("Access-Control-Allow-Origin", "*");

    try
    {
        int deviceSetIndex = boost::lexical_cast<int>(indexStr);

        if (request.getMethod() == "GET")
        {
            SWGSDRangel::SWGDeviceState normalResponse;
            int status = m_adapter->devicesetDeviceRunGet(deviceSetIndex, normalResponse, errorResponse);

            response.setStatus(status);

            if (status/100 == 2) {
                response.write(normalResponse.asJson().toUtf8());
            } else {
                response.write(errorResponse.asJson().toUtf8());
            }
        }
        else if (request.getMethod() == "POST")
        {
            SWGSDRangel::SWGDeviceState normalResponse;
            int status = m_adapter->devicesetDeviceRunPost(deviceSetIndex, normalResponse, errorResponse);

            response.setStatus(status);

            if (status/100 == 2) {
                response.write(normalResponse.asJson().toUtf8());
            } else {
                response.write(errorResponse.asJson().toUtf8());
            }
        }
        else if (request.getMethod() == "DELETE")
        {
            SWGSDRangel::SWGDeviceState normalResponse;
            int status = m_adapter->devicesetDeviceRunDelete(deviceSetIndex, normalResponse, errorResponse);

            response.setStatus(status);

            if (status/100 == 2) {
                response.write(normalResponse.asJson().toUtf8());
            } else {
                response.write(errorResponse.asJson().toUtf8());
            }
        }
        else
        {
            response.setStatus(405,"Invalid HTTP method");
            errorResponse.init();
            *errorResponse.getMessage() = "Invalid HTTP method";
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    catch (const boost::bad_lexical_cast &e)
    {
        errorResponse.init();
        *errorResponse.getMessage() = "Wrong integer conversion on device set index";
        response.setStatus(400,"Invalid data");
        response.write(errorResponse.asJson().toUtf8());
    }
}

void WebAPIRequestMapper::devicesetDeviceReportService(const std::string& indexStr, qtwebapp::HttpRequest& request, qtwebapp::HttpResponse& response)
{
    SWGSDRangel::SWGErrorResponse errorResponse;
    response.setHeader("Content-Type", "application/json");
    response.setHeader("Access-Control-Allow-Origin", "*");

    if (request.getMethod() == "GET")
    {
        try
        {
            SWGSDRangel::SWGDeviceReport normalResponse;
            resetDeviceReport(normalResponse);
            int deviceSetIndex = boost::lexical_cast<int>(indexStr);
            int status = m_adapter->devicesetDeviceReportGet(deviceSetIndex, normalResponse, errorResponse);
            response.setStatus(status);

            if (status/100 == 2) {
                response.write(normalResponse.asJson().toUtf8());
            } else {
                response.write(errorResponse.asJson().toUtf8());
            }
        }
        catch (const boost::bad_lexical_cast &e)
        {
            errorResponse.init();
            *errorResponse.getMessage() = "Wrong integer conversion on device set index";
            response.setStatus(400,"Invalid data");
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    else
    {
        response.setStatus(405,"Invalid HTTP method");
        errorResponse.init();
        *errorResponse.getMessage() = "Invalid HTTP method";
        response.write(errorResponse.asJson().toUtf8());
    }
}

void WebAPIRequestMapper::devicesetChannelsReportService(const std::string& indexStr, qtwebapp::HttpRequest& request, qtwebapp::HttpResponse& response)
{
    SWGSDRangel::SWGErrorResponse errorResponse;
    response.setHeader("Content-Type", "application/json");
    response.setHeader("Access-Control-Allow-Origin", "*");

    if (request.getMethod() == "GET")
    {
        try
        {
            SWGSDRangel::SWGChannelsDetail normalResponse;
            int deviceSetIndex = boost::lexical_cast<int>(indexStr);
            int status = m_adapter->devicesetChannelsReportGet(deviceSetIndex, normalResponse, errorResponse);
            response.setStatus(status);

            if (status/100 == 2) {
                response.write(normalResponse.asJson().toUtf8());
            } else {
                response.write(errorResponse.asJson().toUtf8());
            }
        }
        catch (const boost::bad_lexical_cast &e)
        {
            errorResponse.init();
            *errorResponse.getMessage() = "Wrong integer conversion on device set index";
            response.setStatus(400,"Invalid data");
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    else
    {
        response.setStatus(405,"Invalid HTTP method");
        errorResponse.init();
        *errorResponse.getMessage() = "Invalid HTTP method";
        response.write(errorResponse.asJson().toUtf8());
    }
}

void WebAPIRequestMapper::devicesetChannelService(
        const std::string& deviceSetIndexStr,
        qtwebapp::HttpRequest& request,
        qtwebapp::HttpResponse& response)
{
    SWGSDRangel::SWGErrorResponse errorResponse;
    response.setHeader("Content-Type", "application/json");
    response.setHeader("Access-Control-Allow-Origin", "*");

    try
    {
        int deviceSetIndex = boost::lexical_cast<int>(deviceSetIndexStr);

        if (request.getMethod() == "POST")
        {
            QString jsonStr = request.getBody();
            QJsonObject jsonObject;

            if (parseJsonBody(jsonStr, jsonObject, response))
            {
                SWGSDRangel::SWGChannelSettings query;
                SWGSDRangel::SWGSuccessResponse normalResponse;
                resetChannelSettings(query);

                if (jsonObject.contains("direction")) {
                    query.setDirection(jsonObject["direction"].toInt());
                } else {
                    query.setDirection(0); // assume Rx
                }

                if (jsonObject.contains("channelType") && jsonObject["channelType"].isString())
                {
                    query.setChannelType(new QString(jsonObject["channelType"].toString()));

                    int status = m_adapter->devicesetChannelPost(deviceSetIndex, query, normalResponse, errorResponse);

                    response.setStatus(status);

                    if (status/100 == 2) {
                        response.write(normalResponse.asJson().toUtf8());
                    } else {
                        response.write(errorResponse.asJson().toUtf8());
                    }
                }
                else
                {
                    response.setStatus(400,"Invalid JSON request");
                    errorResponse.init();
                    *errorResponse.getMessage() = "Invalid JSON request";
                    response.write(errorResponse.asJson().toUtf8());
                }
            }
            else
            {
                response.setStatus(400,"Invalid JSON format");
                errorResponse.init();
                *errorResponse.getMessage() = "Invalid JSON format";
                response.write(errorResponse.asJson().toUtf8());
            }
        }
        else
        {
            response.setStatus(405,"Invalid HTTP method");
            errorResponse.init();
            *errorResponse.getMessage() = "Invalid HTTP method";
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    catch (const boost::bad_lexical_cast &e)
    {
        errorResponse.init();
        *errorResponse.getMessage() = "Wrong integer conversion on index";
        response.setStatus(400,"Invalid data");
        response.write(errorResponse.asJson().toUtf8());
    }
}

void WebAPIRequestMapper::devicesetChannelIndexService(
        const std::string& deviceSetIndexStr,
        const std::string& channelIndexStr,
        qtwebapp::HttpRequest& request,
        qtwebapp::HttpResponse& response)
{
    SWGSDRangel::SWGErrorResponse errorResponse;
    response.setHeader("Content-Type", "application/json");
    response.setHeader("Access-Control-Allow-Origin", "*");

    try
    {
        int deviceSetIndex = boost::lexical_cast<int>(deviceSetIndexStr);
        int channelIndex = boost::lexical_cast<int>(channelIndexStr);

        if (request.getMethod() == "DELETE")
        {
            SWGSDRangel::SWGSuccessResponse normalResponse;
            int status = m_adapter->devicesetChannelDelete(deviceSetIndex, channelIndex, normalResponse, errorResponse);

            response.setStatus(status);

            if (status/100 == 2) {
                response.write(normalResponse.asJson().toUtf8());
            } else {
                response.write(errorResponse.asJson().toUtf8());
            }
        }
        else
        {
            response.setStatus(405,"Invalid HTTP method");
            errorResponse.init();
            *errorResponse.getMessage() = "Invalid HTTP method";
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    catch (const boost::bad_lexical_cast &e)
    {
        errorResponse.init();
        *errorResponse.getMessage() = "Wrong integer conversion on index";
        response.setStatus(400,"Invalid data");
        response.write(errorResponse.asJson().toUtf8());
    }
}

void WebAPIRequestMapper::devicesetChannelSettingsService(
        const std::string& deviceSetIndexStr,
        const std::string& channelIndexStr,
        qtwebapp::HttpRequest& request,
        qtwebapp::HttpResponse& response)
{
    SWGSDRangel::SWGErrorResponse errorResponse;
    response.setHeader("Content-Type", "application/json");
    response.setHeader("Access-Control-Allow-Origin", "*");

    try
    {
        int deviceSetIndex = boost::lexical_cast<int>(deviceSetIndexStr);
        int channelIndex = boost::lexical_cast<int>(channelIndexStr);

        if (request.getMethod() == "GET")
        {
            SWGSDRangel::SWGChannelSettings normalResponse;
            resetChannelSettings(normalResponse);
            int status = m_adapter->devicesetChannelSettingsGet(deviceSetIndex, channelIndex, normalResponse, errorResponse);
            response.setStatus(status);

            if (status/100 == 2) {
                response.write(normalResponse.asJson().toUtf8());
            } else {
                response.write(errorResponse.asJson().toUtf8());
            }
        }
        else if ((request.getMethod() == "PUT") || (request.getMethod() == "PATCH"))
        {
            QString jsonStr = request.getBody();
            QJsonObject jsonObject;

            if (parseJsonBody(jsonStr, jsonObject, response))
            {
                SWGSDRangel::SWGChannelSettings normalResponse;
                resetChannelSettings(normalResponse);
                QStringList channelSettingsKeys;

                if (validateChannelSettings(normalResponse, jsonObject, channelSettingsKeys))
                {
                    int status = m_adapter->devicesetChannelSettingsPutPatch(
                            deviceSetIndex,
                            channelIndex,
                            (request.getMethod() == "PUT"), // force settings on PUT
                            channelSettingsKeys,
                            normalResponse,
                            errorResponse);
                    response.setStatus(status);

                    if (status/100 == 2) {
                        response.write(normalResponse.asJson().toUtf8());
                    } else {
                        response.write(errorResponse.asJson().toUtf8());
                    }
                }
                else
                {
                    response.setStatus(400,"Invalid JSON request");
                    errorResponse.init();
                    *errorResponse.getMessage() = "Invalid JSON request";
                    response.write(errorResponse.asJson().toUtf8());
                }
            }
            else
            {
                response.setStatus(400,"Invalid JSON format");
                errorResponse.init();
                *errorResponse.getMessage() = "Invalid JSON format";
                response.write(errorResponse.asJson().toUtf8());
            }
        }
        else
        {
            response.setStatus(405,"Invalid HTTP method");
            errorResponse.init();
            *errorResponse.getMessage() = "Invalid HTTP method";
            response.write(errorResponse.asJson().toUtf8());
        }
    }
    catch (const boost::bad_lexical_cast &e)
    {
        errorResponse.init();
        *errorResponse.getMessage() = "Wrong integer conversion on index";
        response.setStatus(400,"Invalid data");
        response.write(errorResponse.asJson().toUtf8());
    }
}

void WebAPIRequestMapper::devicesetChannelReportService(
        const std::string& deviceSetIndexStr,
        const std::string& channelIndexStr,
        qtwebapp::HttpRequest& request,
        qtwebapp::HttpResponse& response)
{
    SWGSDRangel::SWGErrorResponse errorResponse;
    response.setHeader("Content-Type", "application/json");
    response.setHeader("Access-Control-Allow-Origin", "*");

    try
    {
        int deviceSetIndex = boost::lexical_cast<int>(deviceSetIndexStr);
        int channelIndex = boost::lexical_cast<int>(channelIndexStr);

        if (request.getMethod() == "GET")
        {
            SWGSDRangel::SWGChannelReport normalResponse;
            resetChannelReport(normalResponse);
            int status = m_adapter->devicesetChannelReportGet(deviceSetIndex, channelIndex, normalResponse, errorResponse);
            response.setStatus(status);

            if (status/100 == 2) {
                response.write(normalResponse.asJson().toUtf8());
            } else {
                response.write(errorResponse.asJson().toUtf8());
            }
        }
    }
    catch (const boost::bad_lexical_cast &e)
    {
        errorResponse.init();
        *errorResponse.getMessage() = "Wrong integer conversion on index";
        response.setStatus(400,"Invalid data");
        response.write(errorResponse.asJson().toUtf8());
    }
}

bool WebAPIRequestMapper::parseJsonBody(QString& jsonStr, QJsonObject& jsonObject, qtwebapp::HttpResponse& response)
{
    SWGSDRangel::SWGErrorResponse errorResponse;

    try
    {
        QByteArray jsonBytes(jsonStr.toStdString().c_str());
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(jsonBytes, &error);

        if (error.error == QJsonParseError::NoError)
        {
            jsonObject = doc.object();
        }
        else
        {
            QString errorMsg = QString("Input JSON error: ") + error.errorString() + QString(" at offset ") + QString::number(error.offset);
            errorResponse.init();
            *errorResponse.getMessage() = errorMsg;
            response.setStatus(400, errorMsg.toUtf8());
            response.write(errorResponse.asJson().toUtf8());
        }

        return (error.error == QJsonParseError::NoError);
    }
    catch (const std::exception& ex)
    {
        QString errorMsg = QString("Error parsing request: ") + ex.what();
        errorResponse.init();
        *errorResponse.getMessage() = errorMsg;
        response.setStatus(500, errorMsg.toUtf8());
        response.write(errorResponse.asJson().toUtf8());

        return false;
    }
}

bool WebAPIRequestMapper::validatePresetTransfer(SWGSDRangel::SWGPresetTransfer& presetTransfer)
{
    SWGSDRangel::SWGPresetIdentifier *presetIdentifier = presetTransfer.getPreset();

    if (presetIdentifier == 0) {
        return false;
    }

    return validatePresetIdentifer(*presetIdentifier);
}

bool WebAPIRequestMapper::validatePresetIdentifer(SWGSDRangel::SWGPresetIdentifier& presetIdentifier)
{
    return (presetIdentifier.getGroupName() && presetIdentifier.getName() && presetIdentifier.getType());
}

bool WebAPIRequestMapper::validatePresetExport(SWGSDRangel::SWGPresetExport& presetExport)
{
    if (presetExport.getFilePath() == 0) {
        return false;
    }

    SWGSDRangel::SWGPresetIdentifier *presetIdentifier =  presetExport.getPreset();

    if (presetIdentifier == 0) {
        return false;
    }

    return validatePresetIdentifer(*presetIdentifier);
}

bool WebAPIRequestMapper::validateDeviceListItem(SWGSDRangel::SWGDeviceListItem& deviceListItem, QJsonObject& jsonObject)
{
    if (jsonObject.contains("direction")) {
        deviceListItem.setDirection(jsonObject["direction"].toInt());
    } else {
        deviceListItem.setDirection(0); // assume Rx
    }

    bool identified = false;

    if (jsonObject.contains("displayedName") && jsonObject["displayedName"].isString())
    {
        deviceListItem.setDisplayedName(new QString(jsonObject["displayedName"].toString()));
        identified = true;
    } else {
        deviceListItem.setDisplayedName(0);
    }


    if (jsonObject.contains("hwType") && jsonObject["hwType"].isString())
    {
        deviceListItem.setHwType(new QString(jsonObject["hwType"].toString()));
        identified = true;
    } else {
        deviceListItem.setHwType(0);
    }

    if (jsonObject.contains("serial") && jsonObject["serial"].isString())
    {
        deviceListItem.setSerial(new QString(jsonObject["serial"].toString()));
        identified = true;
    } else {
        deviceListItem.setSerial(0);
    }

    if (jsonObject.contains("index")) {
        deviceListItem.setIndex(jsonObject["index"].toInt(-1));
    } else {
        deviceListItem.setIndex(-1);
    }

    if (jsonObject.contains("sequence")){
        deviceListItem.setSequence(jsonObject["sequence"].toInt(-1));
    } else {
        deviceListItem.setSequence(-1);
    }

    if (jsonObject.contains("deviceStreamIndex")) {
        deviceListItem.setDeviceStreamIndex(jsonObject["deviceStreamIndex"].toInt(-1));
    } else {
        deviceListItem.setDeviceStreamIndex(-1);
    }

    return identified;
}

bool WebAPIRequestMapper::validateDeviceSettings(
        SWGSDRangel::SWGDeviceSettings& deviceSettings,
        QJsonObject& jsonObject,
        QStringList& deviceSettingsKeys)
{
    if (jsonObject.contains("direction")) {
        deviceSettings.setDirection(jsonObject["direction"].toInt());
    } else {
        deviceSettings.setDirection(0); // assume single Rx
    }

    if (jsonObject.contains("deviceHwType") && jsonObject["deviceHwType"].isString()) {
        deviceSettings.setDeviceHwType(new QString(jsonObject["deviceHwType"].toString()));
    } else {
        return false;
    }

    QString *deviceHwType = deviceSettings.getDeviceHwType();

    if ((*deviceHwType == "Airspy") && (deviceSettings.getDirection() == 0))
    {
        if (jsonObject.contains("airspySettings") && jsonObject["airspySettings"].isObject())
        {
            QJsonObject airspySettingsJsonObject = jsonObject["airspySettings"].toObject();
            deviceSettingsKeys = airspySettingsJsonObject.keys();
            deviceSettings.setAirspySettings(new SWGSDRangel::SWGAirspySettings());
            deviceSettings.getAirspySettings()->fromJsonObject(airspySettingsJsonObject);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if ((*deviceHwType == "AirspyHF") && (deviceSettings.getDirection() == 0))
    {
        if (jsonObject.contains("airspyHFSettings") && jsonObject["airspyHFSettings"].isObject())
        {
            QJsonObject airspyHFSettingsJsonObject = jsonObject["airspyHFSettings"].toObject();
            deviceSettingsKeys = airspyHFSettingsJsonObject.keys();
            deviceSettings.setAirspyHfSettings(new SWGSDRangel::SWGAirspyHFSettings());
            deviceSettings.getAirspyHfSettings()->fromJsonObject(airspyHFSettingsJsonObject);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if ((*deviceHwType == "BladeRF1") && (deviceSettings.getDirection() == 0))
    {
        if (jsonObject.contains("bladeRF1InputSettings") && jsonObject["bladeRF1InputSettings"].isObject())
        {
            QJsonObject bladeRF1InputSettingsJsonObject = jsonObject["bladeRF1InputSettings"].toObject();
            deviceSettingsKeys = bladeRF1InputSettingsJsonObject.keys();
            deviceSettings.setBladeRf1InputSettings(new SWGSDRangel::SWGBladeRF1InputSettings());
            deviceSettings.getBladeRf1InputSettings()->fromJsonObject(bladeRF1InputSettingsJsonObject);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if ((*deviceHwType == "BladeRF1") && (deviceSettings.getDirection() == 1))
    {
        if (jsonObject.contains("bladeRF1OutputSettings") && jsonObject["bladeRF1OutputSettings"].isObject())
        {
            QJsonObject bladeRF1OutputSettingsJsonObject = jsonObject["bladeRF1OutputSettings"].toObject();
            deviceSettingsKeys = bladeRF1OutputSettingsJsonObject.keys();
            deviceSettings.setBladeRf1OutputSettings(new SWGSDRangel::SWGBladeRF1OutputSettings());
            deviceSettings.getBladeRf1OutputSettings()->fromJsonObject(bladeRF1OutputSettingsJsonObject);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if ((*deviceHwType == "BladeRF2") && (deviceSettings.getDirection() == 0))
    {
        if (jsonObject.contains("bladeRF2InputSettings") && jsonObject["bladeRF2InputSettings"].isObject())
        {
            QJsonObject bladeRF2InputSettingsJsonObject = jsonObject["bladeRF2InputSettings"].toObject();
            deviceSettingsKeys = bladeRF2InputSettingsJsonObject.keys();
            deviceSettings.setBladeRf2InputSettings(new SWGSDRangel::SWGBladeRF2InputSettings());
            deviceSettings.getBladeRf2InputSettings()->fromJsonObject(bladeRF2InputSettingsJsonObject);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if ((*deviceHwType == "BladeRF2") && (deviceSettings.getDirection() == 1))
    {
        if (jsonObject.contains("bladeRF2OutputSettings") && jsonObject["bladeRF2OutputSettings"].isObject())
        {
            QJsonObject bladeRF2OutputSettingsJsonObject = jsonObject["bladeRF2OutputSettings"].toObject();
            deviceSettingsKeys = bladeRF2OutputSettingsJsonObject.keys();
            deviceSettings.setBladeRf2OutputSettings(new SWGSDRangel::SWGBladeRF2OutputSettings());
            deviceSettings.getBladeRf2OutputSettings()->fromJsonObject(bladeRF2OutputSettingsJsonObject);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (*deviceHwType == "FCDPro")
    {
        if (jsonObject.contains("fcdProSettings") && jsonObject["fcdProSettings"].isObject())
        {
            QJsonObject fcdProSettingsJsonObject = jsonObject["fcdProSettings"].toObject();
            deviceSettingsKeys = fcdProSettingsJsonObject.keys();
            deviceSettings.setFcdProSettings(new SWGSDRangel::SWGFCDProSettings());
            deviceSettings.getFcdProSettings()->fromJsonObject(fcdProSettingsJsonObject);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (*deviceHwType == "FCDPro+")
    {
        if (jsonObject.contains("fcdProPlusSettings") && jsonObject["fcdProPlusSettings"].isObject())
        {
            QJsonObject fcdProPlusSettingsJsonObject = jsonObject["fcdProPlusSettings"].toObject();
            deviceSettingsKeys = fcdProPlusSettingsJsonObject.keys();
            deviceSettings.setFcdProPlusSettings(new SWGSDRangel::SWGFCDProPlusSettings());
            deviceSettings.getFcdProPlusSettings()->fromJsonObject(fcdProPlusSettingsJsonObject);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (*deviceHwType == "FileInput")
    {
        if (jsonObject.contains("fileInputSettings") && jsonObject["fileInputSettings"].isObject())
        {
            QJsonObject fileInputSettingsJsonObject = jsonObject["fileInputSettings"].toObject();
            deviceSettingsKeys = fileInputSettingsJsonObject.keys();
            deviceSettings.setFileInputSettings(new SWGSDRangel::SWGFileInputSettings());
            deviceSettings.getFileInputSettings()->fromJsonObject(fileInputSettingsJsonObject);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if ((*deviceHwType == "HackRF") && (deviceSettings.getDirection() == 0))
    {
        if (jsonObject.contains("hackRFInputSettings") && jsonObject["hackRFInputSettings"].isObject())
        {
            QJsonObject hackRFInputSettingsJsonObject = jsonObject["hackRFInputSettings"].toObject();
            deviceSettingsKeys = hackRFInputSettingsJsonObject.keys();
            deviceSettings.setHackRfInputSettings(new SWGSDRangel::SWGHackRFInputSettings());
            deviceSettings.getHackRfInputSettings()->fromJsonObject(hackRFInputSettingsJsonObject);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if ((*deviceHwType == "HackRF") && (deviceSettings.getDirection() == 1))
    {
        if (jsonObject.contains("hackRFOutputSettings") && jsonObject["hackRFOutputSettings"].isObject())
        {
            QJsonObject hackRFOutputSettingsJsonObject = jsonObject["hackRFOutputSettings"].toObject();
            deviceSettingsKeys = hackRFOutputSettingsJsonObject.keys();
            deviceSettings.setHackRfOutputSettings(new SWGSDRangel::SWGHackRFOutputSettings());
            deviceSettings.getHackRfOutputSettings()->fromJsonObject(hackRFOutputSettingsJsonObject);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if ((*deviceHwType == "KiwiSDR") && (deviceSettings.getDirection() == 0))
    {
        if (jsonObject.contains("kiwiSDRSettings") && jsonObject["kiwiSDRSettings"].isObject())
        {
            QJsonObject kiwiSDRSettingsJsonObject = jsonObject["kiwiSDRSettings"].toObject();
            deviceSettingsKeys = kiwiSDRSettingsJsonObject.keys();
            deviceSettings.setKiwiSdrSettings(new SWGSDRangel::SWGKiwiSDRSettings());
            deviceSettings.getKiwiSdrSettings()->fromJsonObject(kiwiSDRSettingsJsonObject);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if ((*deviceHwType == "LimeSDR") && (deviceSettings.getDirection() == 0))
    {
        if (jsonObject.contains("limeSdrInputSettings") && jsonObject["limeSdrInputSettings"].isObject())
        {
            QJsonObject limeSdrInputSettingsJsonObject = jsonObject["limeSdrInputSettings"].toObject();
            deviceSettingsKeys = limeSdrInputSettingsJsonObject.keys();
            deviceSettings.setLimeSdrInputSettings(new SWGSDRangel::SWGLimeSdrInputSettings());
            deviceSettings.getLimeSdrInputSettings()->fromJsonObject(limeSdrInputSettingsJsonObject);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if ((*deviceHwType == "LimeSDR") && (deviceSettings.getDirection() == 1))
    {
        if (jsonObject.contains("limeSdrOutputSettings") && jsonObject["limeSdrOutputSettings"].isObject())
        {
            QJsonObject limeSdrOutputSettingsJsonObject = jsonObject["limeSdrOutputSettings"].toObject();
            deviceSettingsKeys = limeSdrOutputSettingsJsonObject.keys();
            deviceSettings.setLimeSdrOutputSettings(new SWGSDRangel::SWGLimeSdrOutputSettings());
            deviceSettings.getLimeSdrOutputSettings()->fromJsonObject(limeSdrOutputSettingsJsonObject);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (*deviceHwType == "Perseus")
    {
        if (jsonObject.contains("perseusSettings") && jsonObject["perseusSettings"].isObject())
        {
            QJsonObject perseusSettingsJsonObject = jsonObject["perseusSettings"].toObject();
            deviceSettingsKeys = perseusSettingsJsonObject.keys();
            deviceSettings.setPerseusSettings(new SWGSDRangel::SWGPerseusSettings());
            deviceSettings.getPerseusSettings()->fromJsonObject(perseusSettingsJsonObject);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if ((*deviceHwType == "PlutoSDR") && (deviceSettings.getDirection() == 0))
    {
        if (jsonObject.contains("plutoSdrInputSettings") && jsonObject["plutoSdrInputSettings"].isObject())
        {
            QJsonObject plutoSdrInputSettingsJsonObject = jsonObject["plutoSdrInputSettings"].toObject();
            deviceSettingsKeys = plutoSdrInputSettingsJsonObject.keys();
            deviceSettings.setPlutoSdrInputSettings(new SWGSDRangel::SWGPlutoSdrInputSettings());
            deviceSettings.getPlutoSdrInputSettings()->fromJsonObject(plutoSdrInputSettingsJsonObject);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if ((*deviceHwType == "PlutoSDR") && (deviceSettings.getDirection() == 1))
    {
        if (jsonObject.contains("plutoSdrOutputSettings") && jsonObject["plutoSdrOutputSettings"].isObject())
        {
            QJsonObject plutoSdrOutputSettingsJsonObject = jsonObject["plutoSdrOutputSettings"].toObject();
            deviceSettingsKeys = plutoSdrOutputSettingsJsonObject.keys();
            deviceSettings.setPlutoSdrOutputSettings(new SWGSDRangel::SWGPlutoSdrOutputSettings());
            deviceSettings.getPlutoSdrOutputSettings()->fromJsonObject(plutoSdrOutputSettingsJsonObject);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (*deviceHwType == "RTLSDR")
    {
        if (jsonObject.contains("rtlSdrSettings") && jsonObject["rtlSdrSettings"].isObject())
        {
            QJsonObject rtlSdrSettingsJsonObject = jsonObject["rtlSdrSettings"].toObject();
            deviceSettingsKeys = rtlSdrSettingsJsonObject.keys();
            deviceSettings.setRtlSdrSettings(new SWGSDRangel::SWGRtlSdrSettings());
            deviceSettings.getRtlSdrSettings()->fromJsonObject(rtlSdrSettingsJsonObject);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (*deviceHwType == "SDRplay1")
    {
        if (jsonObject.contains("sdrPlaySettings") && jsonObject["sdrPlaySettings"].isObject())
        {
            QJsonObject sdrPlaySettingsJsonObject = jsonObject["sdrPlaySettings"].toObject();
            deviceSettingsKeys = sdrPlaySettingsJsonObject.keys();
            deviceSettings.setSdrPlaySettings(new SWGSDRangel::SWGSDRPlaySettings());
            deviceSettings.getSdrPlaySettings()->fromJsonObject(sdrPlaySettingsJsonObject);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if ((*deviceHwType == "SoapySDR")  && (deviceSettings.getDirection() == 0))
    {
        if (jsonObject.contains("soapySDRInputSettings") && jsonObject["soapySDRInputSettings"].isObject())
        {
            QJsonObject soapySdrInputSettingsJsonObject = jsonObject["soapySDRInputSettings"].toObject();
            deviceSettingsKeys = soapySdrInputSettingsJsonObject.keys();
            deviceSettings.setSoapySdrInputSettings(new SWGSDRangel::SWGSoapySDRInputSettings());
            deviceSettings.getSoapySdrInputSettings()->init(); // contains complex objects
            deviceSettings.getSoapySdrInputSettings()->fromJsonObject(soapySdrInputSettingsJsonObject);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if ((*deviceHwType == "SoapySDR")  && (deviceSettings.getDirection() == 1))
    {
        if (jsonObject.contains("soapySDROutputSettings") && jsonObject["soapySDROutputSettings"].isObject())
        {
            QJsonObject soapySdrOutputSettingsJsonObject = jsonObject["soapySDROutputSettings"].toObject();
            deviceSettingsKeys = soapySdrOutputSettingsJsonObject.keys();
            deviceSettings.setSoapySdrOutputSettings(new SWGSDRangel::SWGSoapySDROutputSettings());
            deviceSettings.getSoapySdrInputSettings()->init(); // contains complex objects
            deviceSettings.getSoapySdrOutputSettings()->fromJsonObject(soapySdrOutputSettingsJsonObject);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (*deviceHwType == "TestSource")
    {
        if (jsonObject.contains("testSourceSettings") && jsonObject["testSourceSettings"].isObject())
        {
            QJsonObject testSourceSettingsJsonObject = jsonObject["testSourceSettings"].toObject();
            deviceSettingsKeys = testSourceSettingsJsonObject.keys();
            deviceSettings.setTestSourceSettings(new SWGSDRangel::SWGTestSourceSettings());
            deviceSettings.getTestSourceSettings()->fromJsonObject(testSourceSettingsJsonObject);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if ((*deviceHwType == "TestMI") && (deviceSettings.getDirection() == 2))
    {
        if (jsonObject.contains("TestMISettings") && jsonObject["TestMISettings"].isObject())
        {
            QJsonObject testMISettingsJsonObject = jsonObject["TestMISettings"].toObject();
            deviceSettingsKeys = testMISettingsJsonObject.keys();

            if (deviceSettingsKeys.contains("streams") && testMISettingsJsonObject["streams"].isArray())
            {
                appendSettingsArrayKeys(testMISettingsJsonObject, "streams", deviceSettingsKeys);
            }

            deviceSettings.setTestMiSettings(new SWGSDRangel::SWGTestMISettings());
            deviceSettings.getTestMiSettings()->fromJsonObject(testMISettingsJsonObject);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if ((*deviceHwType == "XTRX") && (deviceSettings.getDirection() == 0))
    {
        if (jsonObject.contains("xtrxInputSettings") && jsonObject["xtrxInputSettings"].isObject())
        {
            QJsonObject xtrxInputSettingsJsonObject = jsonObject["xtrxInputSettings"].toObject();
            deviceSettingsKeys = xtrxInputSettingsJsonObject.keys();
            deviceSettings.setXtrxInputSettings(new SWGSDRangel::SWGXtrxInputSettings());
            deviceSettings.getXtrxInputSettings()->fromJsonObject(xtrxInputSettingsJsonObject);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if ((*deviceHwType == "XTRX") && (deviceSettings.getDirection() == 1))
    {
        if (jsonObject.contains("xtrxOutputSettings") && jsonObject["xtrxOutputSettings"].isObject())
        {
            QJsonObject xtrxOutputSettingsJsonObject = jsonObject["xtrxOutputSettings"].toObject();
            deviceSettingsKeys = xtrxOutputSettingsJsonObject.keys();
            deviceSettings.setXtrxOutputSettings(new SWGSDRangel::SWGXtrxOutputSettings());
            deviceSettings.getXtrxOutputSettings()->fromJsonObject(xtrxOutputSettingsJsonObject);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if ((*deviceHwType == "RemoteInput") && (deviceSettings.getDirection() == 0))
    {
        if (jsonObject.contains("remoteInputSettings") && jsonObject["remoteInputSettings"].isObject())
        {
            QJsonObject remoteInputSettingsJsonObject = jsonObject["remoteInputSettings"].toObject();
            deviceSettingsKeys = remoteInputSettingsJsonObject.keys();
            deviceSettings.setRemoteInputSettings(new SWGSDRangel::SWGRemoteInputSettings());
            deviceSettings.getRemoteInputSettings()->fromJsonObject(remoteInputSettingsJsonObject);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if ((*deviceHwType == "LocalInput") && (deviceSettings.getDirection() == 0))
    {
        if (jsonObject.contains("localInputSettings") && jsonObject["localInputSettings"].isObject())
        {
            QJsonObject localInputSettingsJsonObject = jsonObject["localInputSettings"].toObject();
            deviceSettingsKeys = localInputSettingsJsonObject.keys();
            deviceSettings.setLocalInputSettings(new SWGSDRangel::SWGLocalInputSettings());
            deviceSettings.getLocalInputSettings()->fromJsonObject(localInputSettingsJsonObject);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if ((*deviceHwType == "RemoteOutput") && (deviceSettings.getDirection() == 1))
    {
        if (jsonObject.contains("remoteOutputSettings") && jsonObject["remoteOutputSettings"].isObject())
        {
            QJsonObject remoteOutputSettingsJsonObject = jsonObject["remoteOutputSettings"].toObject();
            deviceSettingsKeys = remoteOutputSettingsJsonObject.keys();
            deviceSettings.setRemoteOutputSettings(new SWGSDRangel::SWGRemoteOutputSettings());
            deviceSettings.getRemoteOutputSettings()->fromJsonObject(remoteOutputSettingsJsonObject);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if ((*deviceHwType == "LocalOutput") && (deviceSettings.getDirection() == 1))
    {
        if (jsonObject.contains("localOutputSettings") && jsonObject["localOutputSettings"].isObject())
        {
            QJsonObject localOutputSettingsJsonObject = jsonObject["localOutputSettings"].toObject();
            deviceSettingsKeys = localOutputSettingsJsonObject.keys();
            deviceSettings.setLocalOutputSettings(new SWGSDRangel::SWGLocalOutputSettings());
            deviceSettings.getLocalOutputSettings()->fromJsonObject(localOutputSettingsJsonObject);
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool WebAPIRequestMapper::validateChannelSettings(
        SWGSDRangel::SWGChannelSettings& channelSettings,
        QJsonObject& jsonObject,
        QStringList& channelSettingsKeys)
{
    if (jsonObject.contains("direction")) {
        channelSettings.setDirection(jsonObject["direction"].toInt());
    } else {
        channelSettings.setDirection(0); // assume single Rx
    }

    if (jsonObject.contains("channelType") && jsonObject["channelType"].isString()) {
        channelSettings.setChannelType(new QString(jsonObject["channelType"].toString()));
    } else {
        return false;
    }

    QString *channelType = channelSettings.getChannelType();

    if (*channelType == "AMDemod")
    {
        if (channelSettings.getDirection() == 0)
        {
            QJsonObject amDemodSettingsJsonObject = jsonObject["AMDemodSettings"].toObject();
            channelSettingsKeys = amDemodSettingsJsonObject.keys();
            channelSettings.setAmDemodSettings(new SWGSDRangel::SWGAMDemodSettings());
            channelSettings.getAmDemodSettings()->fromJsonObject(amDemodSettingsJsonObject);
            return true;
        }
        else {
            return false;
        }
    }
    else if (*channelType == "AMMod")
    {
        if (channelSettings.getDirection() == 1)
        {
            QJsonObject amModSettingsJsonObject = jsonObject["AMModSettings"].toObject();
            channelSettingsKeys = amModSettingsJsonObject.keys();

            if (channelSettingsKeys.contains("cwKeyer"))
            {
                QJsonObject cwKeyerSettingsJsonObject;
                appendSettingsSubKeys(amModSettingsJsonObject, cwKeyerSettingsJsonObject, "cwKeyer", channelSettingsKeys);
            }

            channelSettings.setAmModSettings(new SWGSDRangel::SWGAMModSettings());
            channelSettings.getAmModSettings()->fromJsonObject(amModSettingsJsonObject);
            return true;
        }
        else {
            return false;
        }
    }
    else if (*channelType == "ATVMod")
    {
        if (channelSettings.getDirection() == 1)
        {
            QJsonObject atvModSettingsJsonObject = jsonObject["ATVModSettings"].toObject();
            channelSettingsKeys = atvModSettingsJsonObject.keys();
            channelSettings.setAtvModSettings(new SWGSDRangel::SWGATVModSettings());
            channelSettings.getAtvModSettings()->fromJsonObject(atvModSettingsJsonObject);
            return true;
        }
        else {
            return false;
        }
    }
    else if (*channelType == "BFMDemod")
    {
        if (channelSettings.getDirection() == 0)
        {
            QJsonObject bfmDemodSettingsJsonObject = jsonObject["BFMDemodSettings"].toObject();
            channelSettingsKeys = bfmDemodSettingsJsonObject.keys();
            channelSettings.setBfmDemodSettings(new SWGSDRangel::SWGBFMDemodSettings());
            channelSettings.getBfmDemodSettings()->fromJsonObject(bfmDemodSettingsJsonObject);
            return true;
        }
        else {
            return false;
        }
    }
    else if (*channelType == "DSDDemod")
    {
        if (channelSettings.getDirection() == 0)
        {
            QJsonObject dsdDemodSettingsJsonObject = jsonObject["DSDDemodSettings"].toObject();
            channelSettingsKeys = dsdDemodSettingsJsonObject.keys();
            channelSettings.setDsdDemodSettings(new SWGSDRangel::SWGDSDDemodSettings());
            channelSettings.getDsdDemodSettings()->fromJsonObject(dsdDemodSettingsJsonObject);
            return true;
        }
        else {
            return false;
        }
    }
    else if (*channelType == "FreeDVDemod")
    {
        if (channelSettings.getDirection() == 0)
        {
            QJsonObject freeDVDemodSettingsJsonObject = jsonObject["FreeDVDemodSettings"].toObject();
            channelSettingsKeys = freeDVDemodSettingsJsonObject.keys();
            channelSettings.setFreeDvDemodSettings(new SWGSDRangel::SWGFreeDVDemodSettings());
            channelSettings.getFreeDvDemodSettings()->fromJsonObject(freeDVDemodSettingsJsonObject);
            return true;
        }
        else {
            return false;
        }
    }
    else if (*channelType == "FreeDVMod")
    {
        if (channelSettings.getDirection() == 1)
        {
            QJsonObject freeDVModSettingsJsonObject = jsonObject["FreeDVModSettings"].toObject();
            channelSettingsKeys = freeDVModSettingsJsonObject.keys();
            channelSettings.setFreeDvModSettings(new SWGSDRangel::SWGFreeDVModSettings());
            channelSettings.getFreeDvModSettings()->fromJsonObject(freeDVModSettingsJsonObject);
            return true;
        }
        else {
            return false;
        }
    }
    else if (*channelType == "FreqTracker")
    {
        if (channelSettings.getDirection() == 0)
        {
            QJsonObject freqTrackerSettingsJsonObject = jsonObject["FreqTrackerSettings"].toObject();
            channelSettingsKeys = freqTrackerSettingsJsonObject.keys();
            channelSettings.setFreqTrackerSettings(new SWGSDRangel::SWGFreqTrackerSettings());
            channelSettings.getFreqTrackerSettings()->fromJsonObject(freqTrackerSettingsJsonObject);
            return true;
        }
        else {
            return false;
        }
    }
    else if (*channelType == "NFMDemod")
    {
        if (channelSettings.getDirection() == 0)
        {
            QJsonObject nfmDemodSettingsJsonObject = jsonObject["NFMDemodSettings"].toObject();
            channelSettingsKeys = nfmDemodSettingsJsonObject.keys();
            channelSettings.setNfmDemodSettings(new SWGSDRangel::SWGNFMDemodSettings());
            channelSettings.getNfmDemodSettings()->fromJsonObject(nfmDemodSettingsJsonObject);
            return true;
        }
        else {
            return false;
        }
    }
    else if (*channelType == "NFMMod")
    {
        if (channelSettings.getDirection() == 1)
        {
            QJsonObject nfmModSettingsJsonObject = jsonObject["NFMModSettings"].toObject();
            channelSettingsKeys = nfmModSettingsJsonObject.keys();

            if (channelSettingsKeys.contains("cwKeyer"))
            {
                QJsonObject cwKeyerSettingsJsonObject;
                appendSettingsSubKeys(nfmModSettingsJsonObject, cwKeyerSettingsJsonObject, "cwKeyer", channelSettingsKeys);
            }

            channelSettings.setNfmModSettings(new SWGSDRangel::SWGNFMModSettings());
            channelSettings.getNfmModSettings()->fromJsonObject(nfmModSettingsJsonObject);
            return true;
        }
        else {
            return false;
        }
    }
    else if (*channelType == "LocalSink")
    {
        if (channelSettings.getDirection() == 0)
        {
            QJsonObject localChannelSinkSettingsJsonObject = jsonObject["LocalSinkSettings"].toObject();
            channelSettingsKeys = localChannelSinkSettingsJsonObject.keys();
            channelSettings.setLocalSinkSettings(new SWGSDRangel::SWGLocalSinkSettings());
            channelSettings.getLocalSinkSettings()->fromJsonObject(localChannelSinkSettingsJsonObject);
            return true;
        }
        else {
            return false;
        }
    }
    else if (*channelType == "LocalSource")
    {
        if (channelSettings.getDirection() == 1)
        {
            QJsonObject localChannelSourceSettingsJsonObject = jsonObject["LocalSourceSettings"].toObject();
            channelSettingsKeys = localChannelSourceSettingsJsonObject.keys();
            channelSettings.setLocalSourceSettings(new SWGSDRangel::SWGLocalSourceSettings());
            channelSettings.getLocalSourceSettings()->fromJsonObject(localChannelSourceSettingsJsonObject);
            return true;
        }
        else {
            return false;
        }
    }
    else if (*channelType == "RemoteSink")
    {
        if (channelSettings.getDirection() == 0)
        {
            QJsonObject remoteChannelSinkSettingsJsonObject = jsonObject["RemoteSinkSettings"].toObject();
            channelSettingsKeys = remoteChannelSinkSettingsJsonObject.keys();
            channelSettings.setRemoteSinkSettings(new SWGSDRangel::SWGRemoteSinkSettings());
            channelSettings.getRemoteSinkSettings()->fromJsonObject(remoteChannelSinkSettingsJsonObject);
            return true;
        }
        else {
            return false;
        }
    }
    else if (*channelType == "RemoteSource")
    {
        if (channelSettings.getDirection() == 1)
        {
            QJsonObject remoteChannelSourceSettingsJsonObject = jsonObject["RemoteSourceSettings"].toObject();
            channelSettingsKeys = remoteChannelSourceSettingsJsonObject.keys();
            channelSettings.setRemoteSourceSettings(new SWGSDRangel::SWGRemoteSourceSettings());
            channelSettings.getRemoteSourceSettings()->fromJsonObject(remoteChannelSourceSettingsJsonObject);
            return true;
        }
        else {
            return false;
        }
    }
    else if (*channelType == "SSBDemod")
    {
        if (channelSettings.getDirection() == 0)
        {
            QJsonObject ssbDemodSettingsJsonObject = jsonObject["SSBDemodSettings"].toObject();
            channelSettingsKeys = ssbDemodSettingsJsonObject.keys();
            channelSettings.setSsbDemodSettings(new SWGSDRangel::SWGSSBDemodSettings());
            channelSettings.getSsbDemodSettings()->fromJsonObject(ssbDemodSettingsJsonObject);
            return true;
        }
        else {
            return false;
        }
    }
    else if (*channelType == "SSBMod")
    {
        if (channelSettings.getDirection() == 1)
        {
            QJsonObject ssbModSettingsJsonObject = jsonObject["SSBModSettings"].toObject();
            channelSettingsKeys = ssbModSettingsJsonObject.keys();

            if (channelSettingsKeys.contains("cwKeyer"))
            {
                QJsonObject cwKeyerSettingsJsonObject;
                appendSettingsSubKeys(ssbModSettingsJsonObject, cwKeyerSettingsJsonObject, "cwKeyer", channelSettingsKeys);
            }

            channelSettings.setSsbModSettings(new SWGSDRangel::SWGSSBModSettings());
            channelSettings.getSsbModSettings()->fromJsonObject(ssbModSettingsJsonObject);
            return true;
        }
        else {
            return false;
        }
    }
    else if (*channelType == "UDPSource")
    {
        if (channelSettings.getDirection() == 1)
        {
            QJsonObject udpSourceSettingsJsonObject = jsonObject["UDPSourceSettings"].toObject();
            channelSettingsKeys = udpSourceSettingsJsonObject.keys();
            channelSettings.setUdpSourceSettings(new SWGSDRangel::SWGUDPSourceSettings());
            channelSettings.getUdpSourceSettings()->fromJsonObject(udpSourceSettingsJsonObject);
            return true;
        }
        else {
            return false;
        }
    }
    else if (*channelType == "UDPSink")
    {
        if (channelSettings.getDirection() == 0)
        {
            QJsonObject udpSinkSettingsJsonObject = jsonObject["UDPSinkSettings"].toObject();
            channelSettingsKeys = udpSinkSettingsJsonObject.keys();
            channelSettings.setUdpSinkSettings(new SWGSDRangel::SWGUDPSinkSettings());
            channelSettings.getUdpSinkSettings()->fromJsonObject(udpSinkSettingsJsonObject);
            return true;
        }
        else {
            return false;
        }
    }
    else if (*channelType == "WFMDemod")
    {
        if (channelSettings.getDirection() == 0)
        {
            QJsonObject wfmDemodSettingsJsonObject = jsonObject["WFMDemodSettings"].toObject();
            channelSettingsKeys = wfmDemodSettingsJsonObject.keys();
            channelSettings.setWfmDemodSettings(new SWGSDRangel::SWGWFMDemodSettings());
            channelSettings.getWfmDemodSettings()->fromJsonObject(wfmDemodSettingsJsonObject);
            return true;
        }
        else {
            return false;
        }
    }
    else if (*channelType == "WFMMod")
    {
        if (channelSettings.getDirection() == 1)
        {
            QJsonObject wfmModSettingsJsonObject = jsonObject["WFMModSettings"].toObject();
            channelSettingsKeys = wfmModSettingsJsonObject.keys();

            if (channelSettingsKeys.contains("cwKeyer"))
            {
                QJsonObject cwKeyerSettingsJsonObject;
                appendSettingsSubKeys(wfmModSettingsJsonObject, cwKeyerSettingsJsonObject, "cwKeyer", channelSettingsKeys);
            }

            channelSettings.setWfmModSettings(new SWGSDRangel::SWGWFMModSettings());
            channelSettings.getWfmModSettings()->fromJsonObject(wfmModSettingsJsonObject);
            return true;
        }
        else {
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool WebAPIRequestMapper::validateAudioInputDevice(
        SWGSDRangel::SWGAudioInputDevice& audioInputDevice,
        QJsonObject& jsonObject,
        QStringList& audioInputDeviceKeys)
{
    if (jsonObject.contains("index")) {
        audioInputDevice.setIndex(jsonObject["index"].toInt());
    } else {
        audioInputDevice.setIndex(-1); // assume systam default
    }
    if (jsonObject.contains("sampleRate"))
    {
        audioInputDevice.setSampleRate(jsonObject["sampleRate"].toInt());
        audioInputDeviceKeys.append("sampleRate");
    }
    if (jsonObject.contains("volume"))
    {
        audioInputDevice.setVolume(jsonObject["volume"].toDouble());
        audioInputDeviceKeys.append("volume");
    }
    return true;
}

bool WebAPIRequestMapper::validateAudioOutputDevice(
        SWGSDRangel::SWGAudioOutputDevice& audioOutputDevice,
        QJsonObject& jsonObject,
        QStringList& audioOutputDeviceKeys)
{
    if (jsonObject.contains("index")) {
        audioOutputDevice.setIndex(jsonObject["index"].toInt());
    } else {
        audioOutputDevice.setIndex(-1); // assume systam default
    }
    if (jsonObject.contains("sampleRate"))
    {
        audioOutputDevice.setSampleRate(jsonObject["sampleRate"].toInt());
        audioOutputDeviceKeys.append("sampleRate");
    }
    if (jsonObject.contains("copyToUDP"))
    {
        audioOutputDevice.setCopyToUdp(jsonObject["copyToUDP"].toInt() == 0 ? 0 : 1);
        audioOutputDeviceKeys.append("copyToUDP");
    }
    if (jsonObject.contains("udpUsesRTP"))
    {
        audioOutputDevice.setUdpUsesRtp(jsonObject["udpUsesRTP"].toInt() == 0 ? 0 : 1);
        audioOutputDeviceKeys.append("udpUsesRTP");
    }
    if (jsonObject.contains("udpChannelMode"))
    {
        audioOutputDevice.setUdpChannelMode(jsonObject["udpChannelMode"].toInt());
        audioOutputDeviceKeys.append("udpChannelMode");
    }
    if (jsonObject.contains("udpChannelCodec"))
    {
        audioOutputDevice.setUdpChannelCodec(jsonObject["udpChannelCodec"].toInt());
        audioOutputDeviceKeys.append("udpChannelCodec");
    }
    if (jsonObject.contains("udpDecimationFactor"))
    {
        audioOutputDevice.setUdpDecimationFactor(jsonObject["udpDecimationFactor"].toInt());
        audioOutputDeviceKeys.append("udpDecimationFactor");
    }
    if (jsonObject.contains("udpAddress"))
    {
        audioOutputDevice.setUdpAddress(new QString(jsonObject["udpAddress"].toString()));
        audioOutputDeviceKeys.append("udpAddress");
    }
    if (jsonObject.contains("udpPort"))
    {
        audioOutputDevice.setUdpPort(jsonObject["udpPort"].toInt());
        audioOutputDeviceKeys.append("udpPort");
    }
    return true;
}

bool WebAPIRequestMapper::validateAMBEDevices(SWGSDRangel::SWGAMBEDevices& ambeDevices, QJsonObject& jsonObject)
{
    if (jsonObject.contains("nbDevices"))
    {
        int nbDevices = jsonObject["nbDevices"].toInt();

        if (jsonObject.contains("ambeDevices"))
        {
            QJsonArray ambeDevicesJson = jsonObject["ambeDevices"].toArray();

            if (nbDevices != ambeDevicesJson.size()) {
                return false;
            }

            ambeDevices.init();
            ambeDevices.setNbDevices(nbDevices);
            QList<SWGSDRangel::SWGAMBEDevice *> *ambeList = ambeDevices.getAmbeDevices();

            for (int i = 0; i < nbDevices; i++)
            {
                QJsonObject ambeDeviceJson = ambeDevicesJson.at(i).toObject();
                if (ambeDeviceJson.contains("deviceRef") && ambeDeviceJson.contains("delete"))
                {
                    ambeList->push_back(new SWGSDRangel::SWGAMBEDevice());
                    ambeList->back()->init();
                    ambeList->back()->setDeviceRef(new QString(ambeDeviceJson["deviceRef"].toString()));
                    ambeList->back()->setDelete(ambeDeviceJson["delete"].toInt());
                }
                else
                {
                    return false;
                }
            }

            return true;
        }
    }

    return false;
}

bool WebAPIRequestMapper::validateConfig(SWGSDRangel::SWGInstanceConfigResponse& config, QJsonObject& jsonObject, WebAPIAdapterInterface::ConfigKeys& configKeys)
{
    if (jsonObject.contains("preferences"))
    {
        SWGSDRangel::SWGPreferences *preferences = new SWGSDRangel::SWGPreferences();
        config.setPreferences(preferences);
        QJsonObject preferencesJson = jsonObject["preferences"].toObject();
        configKeys.m_preferencesKeys = preferencesJson.keys();
        preferences->fromJsonObject(preferencesJson);
    }

    if (jsonObject.contains("commands"))
    {
        QList<SWGSDRangel::SWGCommand *> *commands = new QList<SWGSDRangel::SWGCommand *>();
        config.setCommands(commands);
        QJsonArray commandsJson = jsonObject["commands"].toArray();
        QJsonArray::const_iterator commandsIt = commandsJson.begin();

        for (; commandsIt != commandsJson.end(); ++commandsIt)
        {
            QJsonObject commandJson = commandsIt->toObject();
            commands->append(new SWGSDRangel::SWGCommand());
            configKeys.m_commandKeys.append(WebAPIAdapterInterface::CommandKeys());
            configKeys.m_commandKeys.back().m_keys = commandJson.keys();
            commands->back()->fromJsonObject(commandJson);
        }
    }

    if (jsonObject.contains("workingPreset"))
    {
        SWGSDRangel::SWGPreset *workingPreset = new SWGSDRangel::SWGPreset();
        QJsonObject presetJson = jsonObject["workingPreset"].toObject();
        appendPresetKeys(workingPreset, presetJson, configKeys.m_workingPresetKeys);
    }

    return true;
}

bool WebAPIRequestMapper::appendPresetKeys(
    SWGSDRangel::SWGPreset *preset,
    const QJsonObject& presetJson,
    WebAPIAdapterInterface::PresetKeys& presetKeys
)
{
    if (presetJson.contains("centerFrequency"))
    {
        preset->setCenterFrequency(presetJson["centerFrequency"].toInt());
        presetKeys.m_keys.append("centerFrequency");
    }
    if (presetJson.contains("dcOffsetCorrection"))
    {
        preset->setDcOffsetCorrection(presetJson["dcOffsetCorrection"].toInt());
        presetKeys.m_keys.append("dcOffsetCorrection");
    }
    if (presetJson.contains("iqImbalanceCorrection"))
    {
        preset->setIqImbalanceCorrection(presetJson["iqImbalanceCorrection"].toInt());
        presetKeys.m_keys.append("iqImbalanceCorrection");
    }
    if (presetJson.contains("iqImbalanceCorrection"))
    {
        preset->setIqImbalanceCorrection(presetJson["sourcePreset"].toInt());
        presetKeys.m_keys.append("sourcePreset");
    }
    if (presetJson.contains("description"))
    {
        preset->setDescription(new QString(presetJson["description"].toString()));
        presetKeys.m_keys.append("description");
    }
    if (presetJson.contains("group"))
    {
        preset->setGroup(new QString(presetJson["group"].toString()));
        presetKeys.m_keys.append("group");
    }

    if (presetJson.contains("spectrumConfig"))
    {
        QJsonObject spectrumJson = presetJson["spectrumConfig"].toObject();
        presetKeys.m_spectrumKeys = spectrumJson.keys();
        SWGSDRangel::SWGGLSpectrum *spectrum = new SWGSDRangel::SWGGLSpectrum();
        preset->setSpectrumConfig(spectrum);
        spectrum->fromJsonObject(spectrumJson);
    }

    if (presetJson.contains("channelConfigs"))
    {
        QJsonArray channelsJson = presetJson["channelConfigs"].toArray();
        QJsonArray::const_iterator channelsIt = channelsJson.begin();
        QList<SWGSDRangel::SWGChannelConfig*> *channels = new QList<SWGSDRangel::SWGChannelConfig*>();
        preset->setChannelConfigs(channels);

        for (; channelsIt != channelsJson.end(); ++channelsIt)
        {
            QJsonObject channelJson = channelsIt->toObject();
            channels->append(new SWGSDRangel::SWGChannelConfig());
            presetKeys.m_channelsKeys.append(WebAPIAdapterInterface::ChannelKeys());

            if (!appendPresetChannelKeys(channels->back(), channelJson, presetKeys.m_channelsKeys.back())) {
                return false;
            }
        }
    }

    if (presetJson.contains("deviceConfigs"))
    {
        QJsonArray devicesJson = presetJson["deviceConfigs"].toArray();
        QJsonArray::const_iterator devicesIt = devicesJson.begin();
        QList<SWGSDRangel::SWGDeviceConfig*> *devices = new QList<SWGSDRangel::SWGDeviceConfig*>();
        preset->setDeviceConfigs(devices);

        for (; devicesIt != devicesJson.end(); ++devicesIt)
        {
            QJsonObject deviceJson = devicesIt->toObject();
            devices->append(new SWGSDRangel::SWGDeviceConfig());
            presetKeys.m_devicesKeys.append(WebAPIAdapterInterface::DeviceKeys());

            if (!appendPresetDeviceKeys(devices->back(), deviceJson, presetKeys.m_devicesKeys.back())) {
                return false;
            }
        }
    }

    return true;
}

bool WebAPIRequestMapper::appendPresetChannelKeys(
        SWGSDRangel::SWGChannelConfig *channel,
        const QJsonObject& channelJson,
        WebAPIAdapterInterface::ChannelKeys& channelKeys
)
{
    if (channelJson.contains("channelIdURI"))
    {
        QString *channelURI = new QString(channelJson["channelIdURI"].toString());
        channel->setChannelIdUri(channelURI);
        channelKeys.m_keys.append("channelIdURI");

        if (channelJson.contains("config") && m_channelURIToSettingsKey.contains(*channelURI))
        {
            SWGSDRangel::SWGChannelSettings *channelSettings = new SWGSDRangel::SWGChannelSettings();
            return getChannel(m_channelURIToSettingsKey[*channelURI], channelSettings, channelJson["config"].toObject(), channelKeys.m_channelKeys);
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool WebAPIRequestMapper::getChannel(
    const QString& channelSettingsKey,
    SWGSDRangel::SWGChannelSettings *channelSettings,
    const QJsonObject& channelSettingsJson,
    QStringList& channelSettingsKeys
)
{
    QStringList channelKeys = channelSettingsJson.keys();

    if (channelKeys.contains(channelSettingsKey) && channelSettingsJson[channelSettingsKey].isObject())
    {
        QJsonObject settingsJsonObject = channelSettingsJson[channelSettingsKey].toObject();
        channelSettingsKeys = settingsJsonObject.keys();

        if (channelSettingsKey == "AMDemodSettings")
        {
            channelSettings->setAmDemodSettings(new SWGSDRangel::SWGAMDemodSettings());
            channelSettings->getAmDemodSettings()->fromJsonObject(settingsJsonObject);
        }
        else if (channelSettingsKey == "AMModSettings")
        {
            channelSettings->setAmModSettings(new SWGSDRangel::SWGAMModSettings());
            channelSettings->getAmModSettings()->fromJsonObject(settingsJsonObject);
        }
        else if (channelSettingsKey == "ATVModSettings")
        {
            channelSettings->setAtvModSettings(new SWGSDRangel::SWGATVModSettings());
            channelSettings->getAtvModSettings()->fromJsonObject(settingsJsonObject);
        }
        else if (channelSettingsKey == "BFMDemodSettings")
        {
            channelSettings->setBfmDemodSettings(new SWGSDRangel::SWGBFMDemodSettings());
            channelSettings->getBfmDemodSettings()->fromJsonObject(settingsJsonObject);
        }
        else if (channelSettingsKey == "DSDDemodSettings")
        {
            channelSettings->setDsdDemodSettings(new SWGSDRangel::SWGDSDDemodSettings());
            channelSettings->getDsdDemodSettings()->fromJsonObject(settingsJsonObject);
        }
        else if (channelSettingsKey == "FreeDVDemodSettings")
        {
            channelSettings->setFreeDvDemodSettings(new SWGSDRangel::SWGFreeDVDemodSettings());
            channelSettings->getFreeDvDemodSettings()->fromJsonObject(settingsJsonObject);
        }
        else if (channelSettingsKey == "FreeDVModSettings")
        {
            channelSettings->setFreeDvModSettings(new SWGSDRangel::SWGFreeDVModSettings());
            channelSettings->getFreeDvModSettings()->fromJsonObject(settingsJsonObject);
        }
        else if (channelSettingsKey == "FreqTrackerSettings")
        {
            channelSettings->setFreqTrackerSettings(new SWGSDRangel::SWGFreqTrackerSettings());
            channelSettings->getFreqTrackerSettings()->fromJsonObject(settingsJsonObject);
        }
        else if (channelSettingsKey == "NFMDemodSettings")
        {
            channelSettings->setNfmDemodSettings(new SWGSDRangel::SWGNFMDemodSettings());
            channelSettings->getNfmDemodSettings()->fromJsonObject(settingsJsonObject);
        }
        else if (channelSettingsKey == "NFMModSettings")
        {
            channelSettings->setNfmModSettings(new SWGSDRangel::SWGNFMModSettings());
            channelSettings->getNfmModSettings()->fromJsonObject(settingsJsonObject);
        }
        else if (channelSettingsKey == "LocalSinkSettings")
        {
            channelSettings->setLocalSinkSettings(new SWGSDRangel::SWGLocalSinkSettings());
            channelSettings->getLocalSinkSettings()->fromJsonObject(settingsJsonObject);
        }
        else if (channelSettingsKey == "LocalSourceSettings")
        {
            channelSettings->setLocalSourceSettings(new SWGSDRangel::SWGLocalSourceSettings());
            channelSettings->getLocalSourceSettings()->fromJsonObject(settingsJsonObject);
        }
        else if (channelSettingsKey == "RemoteSinkSettings")
        {
            channelSettings->setRemoteSinkSettings(new SWGSDRangel::SWGRemoteSinkSettings());
            channelSettings->getRemoteSinkSettings()->fromJsonObject(settingsJsonObject);
        }
        else if (channelSettingsKey == "RemoteSourceSettings")
        {
            channelSettings->setRemoteSourceSettings(new SWGSDRangel::SWGRemoteSourceSettings());
            channelSettings->getRemoteSourceSettings()->fromJsonObject(settingsJsonObject);
        }
        else if (channelSettingsKey == "SSBDemodSettings")
        {
            channelSettings->setSsbDemodSettings(new SWGSDRangel::SWGSSBDemodSettings());
            channelSettings->getSsbDemodSettings()->fromJsonObject(settingsJsonObject);
        }
        else if (channelSettingsKey == "SSBModSettings")
        {
            channelSettings->setSsbModSettings(new SWGSDRangel::SWGSSBModSettings());
            channelSettings->getSsbModSettings()->fromJsonObject(settingsJsonObject);
        }
        else if (channelSettingsKey == "UDPSourceSettings")
        {
            channelSettings->setUdpSourceSettings(new SWGSDRangel::SWGUDPSourceSettings());
            channelSettings->getUdpSourceSettings()->fromJsonObject(settingsJsonObject);
        }
        else if (channelSettingsKey == "UDPSinkSettings")
        {
            channelSettings->setUdpSinkSettings(new SWGSDRangel::SWGUDPSinkSettings());
            channelSettings->getUdpSinkSettings()->fromJsonObject(settingsJsonObject);
        }
        else if (channelSettingsKey == "WFMDemodSettings")
        {
            channelSettings->setWfmDemodSettings(new SWGSDRangel::SWGWFMDemodSettings());
            channelSettings->getWfmDemodSettings()->fromJsonObject(settingsJsonObject);
        }
        else if (channelSettingsKey == "WFMModSettings")
        {
            channelSettings->setWfmModSettings(new SWGSDRangel::SWGWFMModSettings());
            channelSettings->getWfmModSettings()->fromJsonObject(settingsJsonObject);
        }
        else
        {
            return false;
        }

        return true;
    }
    else
    {
        return false;
    }
}

bool WebAPIRequestMapper::appendPresetDeviceKeys(
        SWGSDRangel::SWGDeviceConfig *device,
        const QJsonObject& deviceJson,
        WebAPIAdapterInterface::DeviceKeys& deviceKeys
)
{
    return true;
}

void WebAPIRequestMapper::appendSettingsSubKeys(
        const QJsonObject& parentSettingsJsonObject,
        QJsonObject& childSettingsJsonObject,
        const QString& parentKey,
        QStringList& keyList)
{
    childSettingsJsonObject = parentSettingsJsonObject[parentKey].toObject();
    QStringList childSettingsKeys = childSettingsJsonObject.keys();

    for (int i = 0; i < childSettingsKeys.size(); i++) {
        keyList.append(parentKey + QString(".") + childSettingsKeys.at(i));
    }
}

void WebAPIRequestMapper::appendSettingsArrayKeys(
        const QJsonObject& parentSettingsJsonObject,
        const QString& parentKey,
        QStringList& keyList)
{
    QJsonArray streams = parentSettingsJsonObject[parentKey].toArray();

    for (int istream = 0; istream < streams.count(); istream++)
    {
        QJsonValue v = streams.takeAt(istream);

        if (v.isObject())
        {
            QJsonObject streamSettingsJsonObject = v.toObject();
            QStringList streamSettingsKeys = streamSettingsJsonObject.keys();

            for (int i = 0; i < streamSettingsKeys.size(); i++) {
                keyList.append(tr("streams[%1].%2").arg(istream).arg(streamSettingsKeys[i]));
            }
        }
    }
}


void WebAPIRequestMapper::resetDeviceSettings(SWGSDRangel::SWGDeviceSettings& deviceSettings)
{
    deviceSettings.cleanup();
    deviceSettings.setDeviceHwType(nullptr);
    deviceSettings.setAirspySettings(nullptr);
    deviceSettings.setAirspyHfSettings(nullptr);
    deviceSettings.setBladeRf1InputSettings(nullptr);
    deviceSettings.setBladeRf1OutputSettings(nullptr);
    deviceSettings.setFcdProPlusSettings(nullptr);
    deviceSettings.setFcdProSettings(nullptr);
    deviceSettings.setFileInputSettings(nullptr);
    deviceSettings.setHackRfInputSettings(nullptr);
    deviceSettings.setHackRfOutputSettings(nullptr);
    deviceSettings.setLimeSdrInputSettings(nullptr);
    deviceSettings.setLimeSdrOutputSettings(nullptr);
    deviceSettings.setPerseusSettings(nullptr);
    deviceSettings.setPlutoSdrInputSettings(nullptr);
    deviceSettings.setPlutoSdrOutputSettings(nullptr);
    deviceSettings.setRtlSdrSettings(nullptr);
    deviceSettings.setRemoteOutputSettings(nullptr);
    deviceSettings.setRemoteInputSettings(nullptr);
    deviceSettings.setSdrPlaySettings(nullptr);
    deviceSettings.setTestSourceSettings(nullptr);
}

void WebAPIRequestMapper::resetDeviceReport(SWGSDRangel::SWGDeviceReport& deviceReport)
{
    deviceReport.cleanup();
    deviceReport.setDeviceHwType(nullptr);
    deviceReport.setAirspyHfReport(nullptr);
    deviceReport.setAirspyReport(nullptr);
    deviceReport.setFileInputReport(nullptr);
    deviceReport.setLimeSdrInputReport(nullptr);
    deviceReport.setLimeSdrOutputReport(nullptr);
    deviceReport.setPerseusReport(nullptr);
    deviceReport.setPlutoSdrInputReport(nullptr);
    deviceReport.setPlutoSdrOutputReport(nullptr);
    deviceReport.setRtlSdrReport(nullptr);
    deviceReport.setRemoteOutputReport(nullptr);
    deviceReport.setRemoteInputReport(nullptr);
    deviceReport.setSdrPlayReport(nullptr);
}

void WebAPIRequestMapper::resetChannelSettings(SWGSDRangel::SWGChannelSettings& channelSettings)
{
    channelSettings.cleanup();
    channelSettings.setChannelType(nullptr);
    channelSettings.setAmDemodSettings(nullptr);
    channelSettings.setAmModSettings(nullptr);
    channelSettings.setAtvModSettings(nullptr);
    channelSettings.setBfmDemodSettings(nullptr);
    channelSettings.setDsdDemodSettings(nullptr);
    channelSettings.setNfmDemodSettings(nullptr);
    channelSettings.setNfmModSettings(nullptr);
    channelSettings.setRemoteSinkSettings(nullptr);
    channelSettings.setRemoteSourceSettings(nullptr);
    channelSettings.setSsbDemodSettings(nullptr);
    channelSettings.setSsbModSettings(nullptr);
    channelSettings.setUdpSourceSettings(nullptr);
    channelSettings.setUdpSinkSettings(nullptr);
    channelSettings.setWfmDemodSettings(nullptr);
    channelSettings.setWfmModSettings(nullptr);
}

void WebAPIRequestMapper::resetChannelReport(SWGSDRangel::SWGChannelReport& channelReport)
{
    channelReport.cleanup();
    channelReport.setChannelType(nullptr);
    channelReport.setAmDemodReport(nullptr);
    channelReport.setAmModReport(nullptr);
    channelReport.setAtvModReport(nullptr);
    channelReport.setBfmDemodReport(nullptr);
    channelReport.setDsdDemodReport(nullptr);
    channelReport.setNfmDemodReport(nullptr);
    channelReport.setNfmModReport(nullptr);
    channelReport.setRemoteSourceReport(nullptr);
    channelReport.setSsbDemodReport(nullptr);
    channelReport.setSsbModReport(nullptr);
    channelReport.setUdpSourceReport(nullptr);
    channelReport.setUdpSinkReport(nullptr);
    channelReport.setWfmDemodReport(nullptr);
    channelReport.setWfmModReport(nullptr);
}

void WebAPIRequestMapper::resetAudioInputDevice(SWGSDRangel::SWGAudioInputDevice& audioInputDevice)
{
    audioInputDevice.cleanup();
    audioInputDevice.setName(nullptr);
}

void WebAPIRequestMapper::resetAudioOutputDevice(SWGSDRangel::SWGAudioOutputDevice& audioOutputDevice)
{
    audioOutputDevice.cleanup();
    audioOutputDevice.setName(nullptr);
    audioOutputDevice.setUdpAddress(nullptr);
}
