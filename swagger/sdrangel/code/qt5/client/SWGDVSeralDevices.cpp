/**
 * SDRangel
 * This is the web API of SDRangel SDR software. SDRangel is an Open Source Qt5/OpenGL 3.0+ GUI and server Software Defined Radio and signal analyzer in software. It supports Airspy, BladeRF, HackRF, LimeSDR, PlutoSDR, RTL-SDR, SDRplay RSP1 and FunCube
 *
 * OpenAPI spec version: 4.0.0
 * Contact: f4exb06@gmail.com
 *
 * NOTE: This class is auto generated by the swagger code generator program.
 * https://github.com/swagger-api/swagger-codegen.git
 * Do not edit the class manually.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "SWGDVSeralDevices.h"

#include "SWGHelpers.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QObject>
#include <QDebug>

namespace Swagger {


SWGDVSeralDevices::SWGDVSeralDevices(QString* json) {
    init();
    this->fromJson(*json);
}

SWGDVSeralDevices::SWGDVSeralDevices() {
    init();
}

SWGDVSeralDevices::~SWGDVSeralDevices() {
    this->cleanup();
}

void
SWGDVSeralDevices::init() {
    nbDevices = NULL;
dvSerialDevices = new QList<QString*>();
}

void
SWGDVSeralDevices::cleanup() {
    
if(dvSerialDevices != NULL) {
        QList<QString*>* arr = dvSerialDevices;
        foreach(QString* o, *arr) {
            delete o;
        }
        delete dvSerialDevices;
    }
}

SWGDVSeralDevices*
SWGDVSeralDevices::fromJson(QString &json) {
    QByteArray array (json.toStdString().c_str());
    QJsonDocument doc = QJsonDocument::fromJson(array);
    QJsonObject jsonObject = doc.object();
    this->fromJsonObject(jsonObject);
    return this;
}

void
SWGDVSeralDevices::fromJsonObject(QJsonObject &pJson) {
    setValue(&nbDevices, pJson["nbDevices"], "qint32", "");
setValue(&dvSerialDevices, pJson["dvSerialDevices"], "QList", "QString");
}

QString
SWGDVSeralDevices::asJson ()
{
    QJsonObject* obj = this->asJsonObject();
    
    QJsonDocument doc(*obj);
    QByteArray bytes = doc.toJson();
    return QString(bytes);
}

QJsonObject*
SWGDVSeralDevices::asJsonObject() {
    QJsonObject* obj = new QJsonObject();
    obj->insert("nbDevices", QJsonValue(nbDevices));

    
    QList<QString*>* dvSerialDevicesList = dvSerialDevices;
    QJsonArray dvSerialDevicesJsonArray;
    toJsonArray((QList<void*>*)dvSerialDevices, &dvSerialDevicesJsonArray, "dvSerialDevices", "QString");

    obj->insert("dvSerialDevices", dvSerialDevicesJsonArray);
    

    return obj;
}

qint32
SWGDVSeralDevices::getNbDevices() {
    return nbDevices;
}
void
SWGDVSeralDevices::setNbDevices(qint32 nbDevices) {
    this->nbDevices = nbDevices;
}

QList<QString*>*
SWGDVSeralDevices::getDvSerialDevices() {
    return dvSerialDevices;
}
void
SWGDVSeralDevices::setDvSerialDevices(QList<QString*>* dvSerialDevices) {
    this->dvSerialDevices = dvSerialDevices;
}



} /* namespace Swagger */
