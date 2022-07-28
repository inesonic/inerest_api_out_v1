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
* This header defines the \ref RestApiOutV1::InesonicBinaryRestHandler class.
***********************************************************************************************************************/

/* .. sphinx-project inerest_api_out_v1 */

#ifndef REST_API_OUT_V1_INESONIC_BINARY_REST_HANDLER_H
#define REST_API_OUT_V1_INESONIC_BINARY_REST_HANDLER_H

#include <QObject>
#include <QString>

#include <cstdint>

#include "rest_api_out_v1_common.h"
#include "rest_api_out_v1_inesonic_rest_handler_base.h"

class QNetworkReply;

namespace RestApiOutV1 {
    /**
     * Inesonic generic binary REST API handler.
     */
    class REST_API_OUT_V1_PUBLIC_API InesonicBinaryRestHandler:public QObject, public InesonicRestHandlerBase {
        Q_OBJECT

        public:
            /**
             * Constructor
             *
             * \param[in] server The server instance this REST API will talk to.
             *
             * \param[in] parent Pointer to the parent object.
             */
            InesonicBinaryRestHandler(Server* server, QObject* parent = nullptr);

            /**
             * Constructor
             *
             * \param[in] secret The secret to be used by this REST API.
             *
             * \param[in] server The server instance this REST API will talk to.
             *
             * \param[in] parent Pointer to the parent object.
             */
            InesonicBinaryRestHandler(const QByteArray& secret, Server* server, QObject* parent = nullptr);

            ~InesonicBinaryRestHandler() override;

        public slots:
            /**
             * Slot you can use to send a message to a remote server.
             *
             * \param[in] endpoint   The endpoint to send the message to.
             *
             * \param[in] binaryData The binary payload to be sent.
             */
            void post(const QString& endpoint, const QByteArray& binaryData);

        signals:
            /**
             * Signal that is emitted when a response to the request is received.
             *
             * \param[out] binaryData  The received binary response.
             *
             * \param[out] contentType The response content type, if reported.
             */
            void responseReceived(const QByteArray& binaryData, const QString& contentType);

            /**
             * Signal that is emitted when the transmission fails.
             *
             * \param[out] errorString a string providing an error message.
             */
            void requestFailed(const QString& errorString);

        private slots:
            /**
             * Slot that is triggered upon receipt of a response or timeout.
             */
            void responseReceived();

        protected:
            /**
             * Method you can overload to process a received response.  The default implementation will trigger the
             * \ref responseReceived signal.
             *
             * \param[in] binaryData  The received binary response.
             *
             * \param[in] contentType The response content type, if reported.
             */
            virtual void processResponse(const QByteArray& binaryData, const QString& contentType);

            /**
             * Method you can overload to process a failed transmisison attempt.  The default implementation will
             * trigger the \ref requestFailed signal.
             *
             * \param[in] errorString a string providing an error message.
             */
            virtual void processRequestFailed(const QString& errorString);

            /**
             * Method that is triggered when the timestamp is successfully updated. The default implementation
             * reissues the request.
             */
            void timestampUpdated() override;

            /**
             * Method that is triggered when a timestamp update has failed. The default implementation triggers a call
             * to \ref processRequestFailed.
             */
            void timestampUpdateFailed() override;

        private:
            /**
             * The number of remaining retries for this request.
             */
            unsigned retriesRemaining;

            /**
             * Value holding the current request payload.
             */
            QByteArray currentPayload;

            /**
             * The full URL for the current pending request.
             */
            QUrl currentUrl;

            /**
             * The current pending network reply.
             */
            QNetworkReply* pendingReply;
    };
}

#endif
