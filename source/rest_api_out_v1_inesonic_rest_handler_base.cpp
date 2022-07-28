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
#include "rest_api_out_v1_inesonic_rest_handler_base.h"

namespace RestApiOutV1 {
    static const Crypto::Hmac::Algorithm hashAlgorithm   = Crypto::Hmac::Algorithm::Sha256;
    static const unsigned                hmacBlockSize   = Crypto::Hmac::blockSize(hashAlgorithm);
    static const unsigned                hmacDigestSize  = Crypto::Hmac::digestSize(hashAlgorithm);
    static const unsigned                timestampLength = 8;

    const unsigned InesonicRestHandlerBase::secretLength = hmacBlockSize - timestampLength;
    const unsigned InesonicRestHandlerBase::hashLength   = hmacDigestSize;

    InesonicRestHandlerBase::InesonicRestHandlerBase(Server* server):Server::RestApi(server) {}

    InesonicRestHandlerBase::InesonicRestHandlerBase(
            const QByteArray& secret,
            Server*           server
        ):Server::RestApi(
            server
        ) {
        setSecret(secret);
    }


    InesonicRestHandlerBase::~InesonicRestHandlerBase() {
        Crypto::scrub(currentSecret);
    }


    void InesonicRestHandlerBase::setSecret(const QByteArray& newSecret) {
        Q_ASSERT(static_cast<unsigned>(newSecret.size()) == secretLength);

        Crypto::scrub(currentSecret);
        currentSecret.resize(hmacBlockSize);
        memcpy(currentSecret.data(), newSecret.data(), secretLength);
    }


    QByteArray InesonicRestHandlerBase::calculateHash(const QByteArray& payload) {
        QByteArray result;

        unsigned long long currentTimestamp = QDateTime::currentSecsSinceEpoch();
        long long          timeDelta        = server()->timeDelta();
        unsigned long long hashSuffix       = (currentTimestamp + timeDelta) / 30;

        if (!currentSecret.isEmpty()) {
            std::uint64_t* rawSecret  = reinterpret_cast<std::uint64_t*>(currentSecret.data());
            rawSecret[secretLength / 8] = hashSuffix;

            Crypto::Hmac hmac(currentSecret, payload, hashAlgorithm);
            result = hmac.digest();
        } else {
            QByteArray     fullSecret = server()->currentDefaultSecret;
            std::uint64_t* rawSecret  = reinterpret_cast<std::uint64_t*>(fullSecret.data());
            rawSecret[secretLength / 8] = hashSuffix;

            Crypto::Hmac hmac(fullSecret, payload, hashAlgorithm);
            result = hmac.digest();
        }

        return result;
    }
}
