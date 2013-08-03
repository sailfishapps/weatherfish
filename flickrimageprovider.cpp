/*
 * Copyright (C) 2013 Lucien XU <sfietkonstantin@free.fr>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * "Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * The names of its contributors may not be used to endorse or promote 
 *     products derived from this software without specific prior written 
 *     permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 */ 

#include "flickrimageprovider.h"
#include <QtCore/QDateTime>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtCore/QDebug>

static const char *API_KEY = "34e11320d32b998b575d90ded1d24712";
static const char *WEATHER_PROJECT_GROUP = "1463451@N25";
static const char *FLICKR_API_URL = "http://ycpi.api.flickr.com/services/rest/";

static const char *LONGITUDE_KEY = "longitude";
static const char *LATITUDE_KEY = "latitude";
static const char *RADIUS_KEY = "radius";
static const char *INCLUDE_WEATHER_PROJECT_KEY = "include_weather_project";

static const char *STAT_KEY = "stat";
static const char *CODE_KEY = "code";
static const char *FAIL = "fail";

static const int COUNT = 50;

FlickrImageProvider::FlickrImageProvider(QObject *parent) :
    QObject(parent)
{
    qsrand(QDateTime::currentDateTime().toTime_t());
    m_city = 0;
    m_network = new QNetworkAccessManager(this);
}

City * FlickrImageProvider::city() const
{
    return m_city;
}

void FlickrImageProvider::setCity(City *city)
{
    if (m_city != city) {
        m_city = city;
        emit cityChanged();
    }
}

QString FlickrImageProvider::photo() const
{
    return m_photo;
}

QString FlickrImageProvider::author() const
{
    return m_author;
}

void FlickrImageProvider::request()
{

    m_photos.clear();
    setPhoto(QString());
    setAuthor(QString());

    if (!m_city) {
        return;
    }

    if (m_city->properties().contains("photos")) {
        foreach (QVariant photo, m_city->properties().value("photos").toList()) {
            QVariantMap photoMap = photo.toMap();
            FlickrPhoto flickrPhoto;
            flickrPhoto.photo = photoMap.value("photo").toString();
            flickrPhoto.authorId = photoMap.value("authorId").toString();
            m_photos.append(flickrPhoto);
        }
        selectRandomPhoto();
        return;
    }

    performPhotoRequest(m_city->longitude(), m_city->latitude(), 0.05);
}

void FlickrImageProvider::selectRandomPhoto()
{
    if (m_photos.count() == 0) {
        return;
    }
    int index = qrand() % COUNT;

    setPhoto(m_photos.at(index).photo);
    setAuthor(QString());
}


void FlickrImageProvider::requestAuthor()
{
    // TODO: do author
}

void FlickrImageProvider::setPhoto(const QString &photo)
{
    if (m_photo != photo) {
        m_photo = photo;
        emit photoChanged();
    }
}

void FlickrImageProvider::setAuthor(const QString &author)
{
    if (m_author != author) {
        m_author = author;
        emit authorChanged();
    }
}

void FlickrImageProvider::performPhotoRequest(float longitude, float latitude, float radius)
{
    float maxLongitude = longitude + radius;
    float minLongitude = longitude - radius;
    if (maxLongitude > 180.) {
        maxLongitude -= 180.;
    }
    if (minLongitude < -180.) {
        minLongitude += 180.;
    }

    float maxLatitude = latitude + radius;
    float minLatitude = latitude - radius;
    if (maxLatitude > 90.) {
        maxLatitude -= 90.;
    }
    if (minLatitude < -90.) {
        minLatitude += 90.;
    }

    QString bbox = QString("%1,%2,%3,%4").arg(minLongitude).arg(minLatitude).arg(maxLongitude).arg(maxLatitude);
    qDebug() << m_city->name() << "box:" << bbox;

    QUrl url(FLICKR_API_URL);
    QUrlQuery query;
    query.addQueryItem("api_key", API_KEY);
    query.addQueryItem("method", "flickr.photos.search");
    query.addQueryItem("format", "json");
    query.addQueryItem("nojsoncallback", "1");
    query.addQueryItem("sort", "interestingness-desc");
    query.addQueryItem("group_id", WEATHER_PROJECT_GROUP);
    query.addQueryItem("bbox", bbox);
    url.setQuery(query);

    QNetworkReply *reply = m_network->get(QNetworkRequest(url));
    reply->setProperty(LONGITUDE_KEY, longitude);
    reply->setProperty(LATITUDE_KEY, latitude);
    reply->setProperty(RADIUS_KEY, radius);
    reply->setProperty(INCLUDE_WEATHER_PROJECT_KEY, latitude);
    connect(reply, &QNetworkReply::finished, this, &FlickrImageProvider::slotPhotosListFinished);
}

void FlickrImageProvider::slotPhotosListFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    bool includeWeatherProject = reply->property(INCLUDE_WEATHER_PROJECT_KEY).toBool();
    if (!reply) {
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        qWarning() << "Network error:" << reply->errorString();
        return;
    }

    if (!m_city) {
        reply->deleteLater();
        qWarning() << "City was unset before the end of this request";
        return;
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();

    m_photos.clear();

    QJsonDocument document = QJsonDocument::fromJson(data);
    QJsonObject documentObject = document.object();
    if (documentObject.value(STAT_KEY).toString() == FAIL) {
        qWarning() << "Flickr API error:" << documentObject.value(CODE_KEY).toString();
        return;
    }

    QJsonObject photos = document.object().value("photos").toObject();
    QJsonArray list = photos.value("photo").toArray();

    if (list.count() < COUNT && includeWeatherProject) {
        float longitude = reply->property(LONGITUDE_KEY).toFloat();
        float latitude = reply->property(LATITUDE_KEY).toFloat();
        float radius = reply->property(RADIUS_KEY).toFloat() * 2;
        performPhotoRequest(longitude, latitude, radius);
        return;
    }

    qDebug() << m_city->name() << "found" << list.count() << "photos";
    QVariantList photosVariant;
    foreach (QJsonValue entry, list) {
        QJsonObject object = entry.toObject();
        QString farm = object.value("farm").toVariant().toString();
        QString server = object.value("server").toString();
        QString id = object.value("id").toString();
        QString secret = object.value("secret").toString();

        QString photo = QString("http://farm%1.staticflickr.com/%2/%3_%4_b.jpg").arg(farm, server,
                                                                                     id, secret);
        QString authorId = object.value("owner").toString();
        FlickrPhoto flickrPhoto;
        flickrPhoto.photo = photo;
        flickrPhoto.authorId = authorId;
        m_photos.append(flickrPhoto);

        QVariantMap photoMap;
        photoMap.insert("photo", photo);
        photoMap.insert("authorId", authorId);
        photosVariant.append(photoMap);
    }
    QVariantMap properties;
    properties.insert("photos", photosVariant);
    m_city->setProperties(properties);

    selectRandomPhoto();
}

void FlickrImageProvider::slotAuthorFinished()
{

}
