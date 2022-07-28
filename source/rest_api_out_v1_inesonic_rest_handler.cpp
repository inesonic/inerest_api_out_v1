/*-*-c++-*-*************************************************************************************************************
* Copyright 2016 Inesonic, LLC.
* All rights reserved.
********************************************************************************************************************//**
* \file
*
* This file implements the \ref RestApiOutV1::InesonicRestHandler class.
***********************************************************************************************************************/

#include <QtGlobal>
#include <QObject>
#include <QString>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <cstring>

#include <crypto_aes_cbc_encryptor.h>
#include <crypto_hmac.h>

#include "rest_api_out_v1_server.h"
#include "rest_api_out_v1_inesonic_rest_handler_base.h"
#include "rest_api_out_v1_inesonic_rest_handler.h"

namespace RestApiOutV1 {
    InesonicRestHandler::InesonicRestHandler(
            Server*  server,
            QObject* parent
        ):QObject(
            parent
        ),InesonicRestHandlerBase(
            server
        ) {
        pendingReply = nullptr;
    }


    InesonicRestHandler::InesonicRestHandler(
            const QByteArray& secret,
            Server*           server,
            QObject*          parent
        ):QObject(
            parent
        ),InesonicRestHandlerBase(
            secret,
            server
        ) {
        pendingReply = nullptr;
    }


    InesonicRestHandler::~InesonicRestHandler() {}


    void InesonicRestHandler::post(const QString& endpoint, const QJsonDocument& jsonData) {
        retriesRemaining = 1;

        currentUrl     = QUrl(server()->schemeAndHost().toString() + endpoint);
        currentPayload = jsonData.toJson(QJsonDocument::JsonFormat::Compact);
        if (isTimestampAccurate()) {
            timestampUpdated();
        }
    }


    void InesonicRestHandler::post(const QString& endpoint, const QJsonObject& jsonData) {
        post(endpoint, QJsonDocument(jsonData));
    }


    void InesonicRestHandler::post(const QString& endpoint, const QJsonArray& jsonData) {
        post(endpoint, QJsonDocument(jsonData));
    }


    void InesonicRestHandler::responseReceived() {
        QNetworkReply::NetworkError networkError = pendingReply->error();

        pendingReply->deleteLater();

        if (networkError == QNetworkReply::NetworkError::NoError) {
            QByteArray receivedData = pendingReply->readAll();
            pendingReply = nullptr;

            QJsonParseError parseError;
            QJsonDocument   jsonDocument = QJsonDocument::fromJson(receivedData, &parseError);
            if (parseError.error == QJsonParseError::NoError) {
                processJsonResponse(jsonDocument);
            } else {
                processRequestFailed(QString("Response not JSON format"));
            }
        } else if (networkError == QNetworkReply::NetworkError::AuthenticationRequiredError &&
                   retriesRemaining > 0                                                        ) {
            pendingReply = nullptr;

            --retriesRemaining;
            updateTimeDelta();
        } else {
            QString errorMessage = pendingReply->errorString();
            pendingReply = nullptr;

            processRequestFailed(errorMessage);
        }
    }


    void InesonicRestHandler::processJsonResponse(const QJsonDocument& jsonData) {
        emit jsonResponse(jsonData);
    }


    void InesonicRestHandler::processRequestFailed(const QString& errorString) {
        emit requestFailed(errorString);
    }


    void InesonicRestHandler::timestampUpdated() {
        if (pendingReply != nullptr) {
            pendingReply->deleteLater();
        }

        QByteArray hash = calculateHash(currentPayload);

        QJsonObject jsonMessage;
        jsonMessage.insert("data", QString::fromLatin1(currentPayload.toBase64()));
        jsonMessage.insert("hash", QString::fromLatin1(hash.toBase64()));

        QByteArray message = QJsonDocument(jsonMessage).toJson(QJsonDocument::JsonFormat::Compact);

        QNetworkRequest request(currentUrl);
        request.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, server()->userAgent());
        request.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/json");
        request.setHeader(QNetworkRequest::KnownHeaders::ContentLengthHeader, message.size());
        request.setTransferTimeout();

        pendingReply = server()->post(request, message);
        pendingReply->setParent(this);

        connect(pendingReply, &QNetworkReply::finished, this, &InesonicRestHandler::responseReceived);
    }


    void InesonicRestHandler::timestampUpdateFailed() {
        if (pendingReply != nullptr) {
            pendingReply->deleteLater();
            pendingReply = nullptr;
        }

        processRequestFailed(QString("Failed to sync with server."));
    }
}
