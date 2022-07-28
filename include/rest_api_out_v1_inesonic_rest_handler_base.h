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
* This header defines the \ref RestApiOutV1::InesonicRestHandlerBase class.
***********************************************************************************************************************/

/* .. sphinx-project inerest_api_out_v1 */

#ifndef REST_API_OUT_V1_INESONIC_REST_HANDLER_BASE_H
#define REST_API_OUT_V1_INESONIC_REST_HANDLER_BASE_H

#include <QObject>
#include <QString>
#include <QByteArray>

#include <cstdint>

#include "rest_api_out_v1_common.h"
#include "rest_api_out_v1_server.h"

namespace RestApiOutV1 {
    /**
     * Base class for Inesonic outbound REST API handlers.
     */
    class REST_API_OUT_V1_PUBLIC_API InesonicRestHandlerBase:public Server::RestApi {
        public:
            /**
             * The required length for secrets, in bytes.
             */
            static const unsigned secretLength;

            /**
             * Constructor
             *
             * \param[in] server The server instance this REST API will talk to.
             */
            InesonicRestHandlerBase(Server* server);

            /**
             * Constructor
             *
             * \param[in] secret The secret to be used by this REST API.
             *
             * \param[in] server The server instance this REST API will talk to.
             */
            InesonicRestHandlerBase(const QByteArray& secret, Server* server);

            ~InesonicRestHandlerBase() override;

            /**
             * Method you can use to set the REST API specific secret.
             *
             * \param[in] newSecret The new secret to be used.
             */
            void setSecret(const QByteArray& newSecret);

        protected:
            /**
             * The length of generated hashes, in bytes.
             */
            static const unsigned hashLength;

            /**
             * Method you can use to calculate the hash for a data payload.
             *
             * \param[in] payload The payload to calculate the time sensitive hash for.
             *
             * \return Returns the hash to be used with this payload.
             */
            QByteArray calculateHash(const QByteArray& payload);

        private:
            /**
             * The current secret to use for web requests.
             */
            QByteArray currentSecret;
    };
}

#endif
