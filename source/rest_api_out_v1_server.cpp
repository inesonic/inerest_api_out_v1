/*-*-c++-*-*************************************************************************************************************
* Copyright 2016 - 2022 Inesonic, LLC.
*
* MIT License:
*   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
*   documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
*   rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
*   permit persons to whom the Software is furnished to do so, subject to the following conditions:
*   
*   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
*   Software.
*   
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*   WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
*   OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
*   OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
********************************************************************************************************************//**
* \file
*
* This file implements the \ref RestApiOutV1::Server class.
***********************************************************************************************************************/

#include <QtGlobal>
#include <QObject>
#include <QTimer>
#include <QCoreApplication>
#include <QString>
#include <QDateTime>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMutex>
#include <QMutexLocker>

#include <cstring>

#include <crypto_aes_cbc_encryptor.h>
#include <crypto_hmac.h>
#include <crypto_helpers.h>

#include "rest_api_out_v1_server.h"

/***********************************************************************************************************************
 * Server::RestApi
 */

namespace RestApiOutV1 {
    Server::RestApi::RestApi(Server* server):currentServer(server) {}


    Server::RestApi::~RestApi() {}


    bool Server::RestApi::isTimestampAccurate() {
        return currentServer->checkTimestamp(this);
    }


    void Server::RestApi::updateTimeDelta() {
        currentServer->updateTimeDelta(this);
    }


    void Server::RestApi::timestampUpdated() {}


    void Server::RestApi::timestampUpdateFailed() {}
}

/***********************************************************************************************************************
 * Server
 */

namespace RestApiOutV1 {
    static const Crypto::Hmac::Algorithm hashAlgorithm   = Crypto::Hmac::Algorithm::Sha256;
    static const unsigned                hmacBlockSize   = Crypto::Hmac::blockSize(hashAlgorithm);
    static const unsigned                hmacDigestSize  = Crypto::Hmac::digestSize(hashAlgorithm);
    static const unsigned                timestampLength = 8;

    const unsigned Server::secretLength = hmacBlockSize - timestampLength;
    const QString  Server::defaultUserAgent("Inesonic, LLC");
    const QString  Server::defaultTimeDeltaSlug("/td");

    Server::Server(
            QNetworkAccessManager* networkAccessManager,
            const QUrl&            serverSchemeAndHost,
            const QString&         timeDeltaSlug,
            QObject*               parent
        ):QObject(
            parent
        ),currentNetworkAccessManager(
            networkAccessManager
        ),currentSchemeAndHost(
            serverSchemeAndHost
        ),currentTimeDeltaSlug(
            timeDeltaSlug
        ),currentDefaultSecret(
            QByteArray()
        ) {
        currentUserAgent = defaultUserAgent;
        currentTimeDelta = 0;

        currentNetworkAccessManager->setRedirectPolicy(QNetworkRequest::RedirectPolicy::NoLessSafeRedirectPolicy);
        currentNetworkAccessManager->setStrictTransportSecurityEnabled(false);

        pendingReply = nullptr;
    }


    Server::Server(
            QNetworkAccessManager* networkAccessManager,
            const QUrl&            serverSchemeAndHost,
            const QByteArray&      defaultSecret,
            const QString&         timeDeltaSlug,
            QObject*               parent
        ):QObject(
            parent
        ),currentNetworkAccessManager(
            networkAccessManager
        ),currentSchemeAndHost(
            serverSchemeAndHost
        ),currentTimeDeltaSlug(
            timeDeltaSlug
        ) {
        setDefaultSecret(defaultSecret);
        currentUserAgent = defaultUserAgent;
        currentTimeDelta = 0;

        currentNetworkAccessManager->setRedirectPolicy(QNetworkRequest::RedirectPolicy::NoLessSafeRedirectPolicy);
        currentNetworkAccessManager->setStrictTransportSecurityEnabled(false);

        pendingReply = nullptr;
    }


    Server::~Server() {
        Crypto::scrub(currentDefaultSecret);
    }


    void Server::setDefaultSecret(const QByteArray& newDefaultSecret) {
        Q_ASSERT(static_cast<unsigned>(newDefaultSecret.size()) == secretLength);

        Crypto::scrub(currentDefaultSecret);
        currentDefaultSecret.resize(hmacBlockSize);
        memcpy(currentDefaultSecret.data(), newDefaultSecret.data(), secretLength);
    }


    QNetworkAccessManager* Server::networkAccessManager() const {
        return currentNetworkAccessManager;
    }


    void Server::setSchemeAndHost(const QUrl& newSchemeAndHost) {
        currentSchemeAndHost = newSchemeAndHost;
    }


    const QUrl& Server::schemeAndHost() const {
        return currentSchemeAndHost;
    }


    void Server::setUserAgent(const QString& newUserAgent) {
        currentUserAgent = newUserAgent;
    }


    const QString& Server::userAgent() const {
        return currentUserAgent;
    }


    void Server::setTimeDeltaSlug(const QString& newTimeDeltaSlug) {
        currentTimeDeltaSlug = newTimeDeltaSlug;
    }


    const QString& Server::timeDeltaSlug() const {
        return currentTimeDeltaSlug;
    }


    QUrl Server::timeDeltaUrl() const {
        QUrl url = currentSchemeAndHost;
        url.setPath(currentTimeDeltaSlug);

        return url;
    }


    void Server::setTimeDelta(long long newTimeDelta) {
        currentTimeDelta = newTimeDelta;
    }


    long long Server::timeDelta() {
        return currentTimeDelta;
    }


    void Server::updateTimeDelta(RestApi* restApi) {
        QMutexLocker locker(&requestMutex);

        if (pendingReply == nullptr) {
            retriesRemaining = numberRetries;
            issueTimeDeltaRequest();
        }

        waitingRestApis.append(restApi);
    }


    bool Server::checkTimestamp(RestApi* restApi) {
        bool result;

        QMutexLocker locker(&requestMutex);
        if (pendingReply == nullptr) {
            result = true;
        } else {
            result = false;
            waitingRestApis.append(restApi);
        }

        return result;
    }


    void Server::issueTimeDeltaRequest() {
        QNetworkRequest request(timeDeltaUrl());

        QJsonObject jsonObject;
        jsonObject.insert("timestamp", static_cast<double>(QDateTime::currentSecsSinceEpoch()));

        QByteArray message = QJsonDocument(jsonObject).toJson(QJsonDocument::JsonFormat::Compact);

        request.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, currentUserAgent);
        request.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/json");
        request.setHeader(QNetworkRequest::KnownHeaders::ContentLengthHeader, message.size());
        request.setRawHeader("Accept", "*/*");

        request.setTransferTimeout();

        currentNetworkAccessManager->setRedirectPolicy(QNetworkRequest::RedirectPolicy::NoLessSafeRedirectPolicy);
        currentNetworkAccessManager->setStrictTransportSecurityEnabled(false);

        pendingReply = currentNetworkAccessManager->post(request, message);
        pendingReply->setParent(this);

        connect(pendingReply, &QNetworkReply::finished, this, &Server::responseReceived);
    }


    void Server::responseReceived() {
        QNetworkReply::NetworkError networkError = pendingReply->error();
        bool                        success;

        if (networkError == QNetworkReply::NetworkError::NoError) {
            QByteArray receivedData = pendingReply->readAll();

            QJsonParseError parseError;
            QJsonDocument   jsonDocument = QJsonDocument::fromJson(receivedData, &parseError);
            if (parseError.error == QJsonParseError::NoError) {
                success = parseResponse(jsonDocument);
            } else {
                success = false;
            }
        } else {
            success = false;
        }

        pendingReply->deleteLater();

        if (!success) {
            if (retriesRemaining > 0) {
                --retriesRemaining;
                issueTimeDeltaRequest();
            } else {
                requestMutex.lock();

                for (  QList<RestApi*>::const_iterator it  = waitingRestApis.constBegin(),
                                                       end = waitingRestApis.constEnd()
                     ; it != end
                     ; ++it
                    ) {
                    (*it)->timestampUpdateFailed();
                }

                waitingRestApis.clear();

                pendingReply = nullptr;
                requestMutex.unlock();

                emit timeDeltaUpdateFailed();
            }
        } else {
            requestMutex.lock();

            for (  QList<RestApi*>::const_iterator it  = waitingRestApis.constBegin(),
                                                   end = waitingRestApis.constEnd()
                 ; it != end
                 ; ++it
                ) {
                (*it)->timestampUpdated();
            }

            waitingRestApis.clear();

            pendingReply = nullptr;
            requestMutex.unlock();

            emit timeDeltaChanged();
        }
    }


    bool Server::parseResponse(const QJsonDocument& document) {
        static constexpr double timeDeltaMax = (1ULL << std::numeric_limits<double>::digits) - 1;

        bool success = false;

        if (document.isObject()) {
            QJsonObject jsonObject = document.object();
            if (jsonObject.size() == 2) {
                QJsonValue statusValue    = jsonObject.value("status");
                QJsonValue timeDeltaValue = jsonObject.value("time_delta");

                if (statusValue.isString() && timeDeltaValue.isDouble() && statusValue.toString() == "OK") {
                    double timeDelta = timeDeltaValue.toDouble();
                    if (timeDelta >= std::numeric_limits<long long>::lowest() && timeDelta <= timeDeltaMax) {
                        currentTimeDelta = static_cast<long long>(timeDelta);
                        success = true;
                    }
                }
            }
        }

        return success;
    }
}
