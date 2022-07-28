/*-*-c++-*-*************************************************************************************************************
* Copyright 2016 Inesonic, LLC.
* All rights reserved.
********************************************************************************************************************//**
* \file
*
* This file implements the \ref RestApiOutV1::InesonicBinaryRestHandler class.
***********************************************************************************************************************/

#include <QtGlobal>
#include <QObject>
#include <QString>
#include <QByteArray>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QVariant>

#include <cstring>

#include <crypto_aes_cbc_encryptor.h>
#include <crypto_hmac.h>

#include "rest_api_out_v1_server.h"
#include "rest_api_out_v1_inesonic_rest_handler_base.h"
#include "rest_api_out_v1_inesonic_binary_rest_handler.h"

namespace RestApiOutV1 {
    InesonicBinaryRestHandler::InesonicBinaryRestHandler(
            Server*  server,
            QObject* parent
        ):QObject(
            parent
        ),InesonicRestHandlerBase(
            server
        ) {
        pendingReply = nullptr;
    }


    InesonicBinaryRestHandler::InesonicBinaryRestHandler(
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


    InesonicBinaryRestHandler::~InesonicBinaryRestHandler() {}


    void InesonicBinaryRestHandler::post(const QString& endpoint, const QByteArray& binaryPayload) {
        retriesRemaining = 1;

        currentUrl = server()->schemeAndHost();
        currentUrl.setPath(endpoint);

        currentPayload = binaryPayload;
        if (isTimestampAccurate()) {
            timestampUpdated();
        }
    }


    void InesonicBinaryRestHandler::responseReceived() {
        QNetworkReply::NetworkError networkError = pendingReply->error();

        pendingReply->deleteLater();

        if (networkError == QNetworkReply::NetworkError::NoError) {
            QByteArray receivedData       = pendingReply->readAll();
            QVariant   contentTypeVariant = pendingReply->header(QNetworkRequest::KnownHeaders::ContentTypeHeader);
            QString    contentType        = contentTypeVariant.isValid() ? contentTypeVariant.toString() : QString();

            pendingReply = nullptr;

            processResponse(receivedData, contentType);
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


    void InesonicBinaryRestHandler::processResponse(const QByteArray& jsonData, const QString& contentType) {
        emit responseReceived(jsonData, contentType);
    }


    void InesonicBinaryRestHandler::processRequestFailed(const QString& errorString) {
        emit requestFailed(errorString);
    }


    void InesonicBinaryRestHandler::timestampUpdated() {
        if (pendingReply != nullptr) {
            pendingReply->deleteLater();
        }

        QByteArray hash    = calculateHash(currentPayload);
        QByteArray message = currentPayload + hash;

        QNetworkRequest request(currentUrl);
        request.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, server()->userAgent());
        request.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/octet-stream");
        request.setHeader(QNetworkRequest::KnownHeaders::ContentLengthHeader, message.size());
        request.setTransferTimeout();

        pendingReply = server()->post(request, message);
        pendingReply->setParent(this);

        connect(
            pendingReply,
            &QNetworkReply::finished,
            this,
            static_cast<void (InesonicBinaryRestHandler::*)()>(&InesonicBinaryRestHandler::responseReceived)
        );
    }


    void InesonicBinaryRestHandler::timestampUpdateFailed() {
        if (pendingReply != nullptr) {
            pendingReply->deleteLater();
            pendingReply = nullptr;
        }

        processRequestFailed(QString("Failed to sync with server."));
    }
}
