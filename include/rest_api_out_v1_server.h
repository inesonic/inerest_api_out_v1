/*-*-c++-*-*************************************************************************************************************
* Copyright 2016 Inesonic, LLC.
* All rights reserved.
********************************************************************************************************************//**
* \file
*
* This header defines the \ref RestApiOutV1::Server class.
***********************************************************************************************************************/

/* .. sphinx-project inerest_api_out_v1 */

#ifndef REST_API_OUT_V1_SERVER_H
#define REST_API_OUT_V1_SERVER_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <cstdint>

#include "rest_api_out_v1_common.h"

namespace RestApiOutV1 {
    class InesonicRestHandlerBase;

    /**
     * Class that provides support for sending messages to generic Inesonic web hooks.
     */
    class REST_API_OUT_V1_PUBLIC_API Server:public QObject {
        friend class InesonicRestHandlerBase;

        Q_OBJECT

        public:
            /**
             * Class used as a basis for outbound REST API requests.
             */
            class RestApi {
                friend class Server;

                public:
                    /**
                     * Constructor
                     *
                     * \param[in] server The outbound REST API server.
                     */
                    RestApi(Server* server);

                    virtual ~RestApi();

                protected:
                    /**
                     * Method you can use to access the underlying server instance.
                     *
                     * \return Returns a pointer to the underlying server instance.
                     */
                    inline Server* server() const {
                        return currentServer;
                    }

                    /**
                     * Method you can call to check if the timestamp is currently being updated and to schedule a
                     * response later if the timestamp is being updated.
                     *
                     * \return Returns true if the timestamp is believed to be accurate.  Returns false if the
                     *         timestamp is in the process of being updated.
                     */
                    bool isTimestampAccurate();

                    /**
                     * Method you can call to trigger the time delta to be updated.
                     */
                    void updateTimeDelta();

                protected:
                    /**
                     * Method that is triggered when the timestamp is successfully updated. The default implementation
                     * does nothing.
                     */
                    virtual void timestampUpdated();

                    /**
                     * Method that is triggered when a timestamp update has failed. The default implementation does
                     * nothing.
                     */
                    virtual void timestampUpdateFailed();

                private:
                    /**
                     * The underlying server instance.
                     */
                    Server* currentServer;
            };

            /**
             * The default user agent string.
             */
            static const QString defaultUserAgent;

            /**
             * The default slug to use to determine time deltas between us and a remote server.
             */
            static const QString defaultTimeDeltaSlug;

            /**
             * The required length for secrets, in bytes.
             */
            static const unsigned secretLength;

            /**
             * Constructor
             *
             * \param[in] networkAccessManager The network settings manager.
             *
             * \param[in] serverSchemeAndHost  The server's scheme and host.
             *
             * \param[in] timeDeltaSlug        The URL used to determine time deltas.
             *
             * \param[in] parent               Pointer to the parent object.
             */
            Server(
                QNetworkAccessManager* networkAccessManager,
                const QUrl&            serverSchemeAndHost,
                const QString&         timeDeltaSlug = defaultTimeDeltaSlug,
                QObject*               parent = Q_NULLPTR
            );

            /**
             * Constructor
             *
             * \param[in] networkAccessManager The network settings manager.
             *
             * \param[in] serverSchemeAndHost  The server's scheme and host.
             *
             * \param[in] defaultSecret        The default secret to use for requests.
             *
             * \param[in] timeDeltaSlug        The URL used to determine time deltas.
             *
             * \param[in] parent               Pointer to the parent object.
             */
            Server(
                QNetworkAccessManager* networkAccessManager,
                const QUrl&            serverSchemeAndHost,
                const QByteArray&      defaultSecret,
                const QString&         timeDeltaSlug = defaultTimeDeltaSlug,
                QObject*               parent = Q_NULLPTR
            );

            ~Server() override;

            /**
             * Method you can use to set the default secret.
             *
             * \param[in] newDefaultSecret The new default secret to be used.
             */
            void setDefaultSecret(const QByteArray& newDefaultSecret);

            /**
             * Method you can use to get the current network access manager.
             *
             * \return Returns a pointer to the current network access manager.
             */
            QNetworkAccessManager* networkAccessManager() const;

            /**
             * Method you can use to set the server's scheme and host.  Value should be of the form
             * "https://myserver.com".
             *
             * \param[in] newServerSchemeAndHost The nee server scheme and host.
             */
            void setSchemeAndHost(const QUrl& newSchemeAndHost);

            /**
             * Method you can use to determine the server's current scheme and host.
             *
             * \return Returns the current server's scheme and host.
             */
            const QUrl& schemeAndHost() const;

            /**
             * Method you can use to set the user agent string.
             *
             * \param[in] newUserAgent The new user agent string.
             */
            void setUserAgent(const QString& newUserAgent);

            /**
             * Method you can use to obtain the current user agent string.
             *
             * \return Returns the current user agent string
             */
            const QString& userAgent() const;

            /**
             * Method you can use to set the server's time delta slug.
             *
             * \param[in] newTimeDeltaSlug The slug to use to measure time deltas.
             */
            void setTimeDeltaSlug(const QString& newTimeDeltaSlug);

            /**
             * Method you can use to obtain the current time delta slug.
             *
             * \return Returns the currently selected time delta slug.
             */
            const QString& timeDeltaSlug() const;

            /**
             * Method you can use to obtain the full URL to the time delta endpoint.
             *
             * \return Returns the currently selected time delta endpoint URL.
             */
            QUrl timeDeltaUrl() const;

            /**
             * Method you can use to force the time delta.  This method is primarily intended for test purposes.
             *
             * \param[in] newTimeDelta The new time delta to be applied.
             */
            void setTimeDelta(long long newTimeDelta);

            /**
             * Method you can use to obtain the current measured time delta.
             *
             * \return Returns the current measured time delta.
             */
            long long timeDelta();

            /**
             * Method you can use to issue a post request.
             *
             * \param[in] request The network request to be sent.
             *
             * \param[in] payload The payload to be sent.
             *
             * \return Returns a newly created network reply instance.
             */
            inline QNetworkReply* post(const QNetworkRequest& request, const QByteArray& payload) {
                return currentNetworkAccessManager->post(request, payload);
            }

        signals:
            /**
             * Signal you can bind to in order to receive notification that the server time delta has changed.
             */
            void timeDeltaChanged();

            /**
             * Signal you can bind to in order to receive notification that the server time delta update failed.
             */
            void timeDeltaUpdateFailed();

        private slots:
            /**
             * Slot that is triggered when a reply or error occurs for an outbound message.
             */
            void responseReceived();

        private:
            friend class RestApi;

            /**
             * Method you can call to trigger the time delta to be updated.
             *
             * \param[in] restApi The REST API making this request.
             */
            void updateTimeDelta(RestApi* restApi);

            /**
             * Method you can call to check if a timestamp update is in progress.  If an update is in progress then
             * the provided REST API instance will be included on a list to receive a callback.
             *
             * \param[in] restApi The REST API making this request.
             *
             * \return Returns true if the timestamp is believed to be accurate.  Returns false if the timestamp is not
             *         accurate.
             */
            bool checkTimestamp(RestApi* restApi);

            /**
             * Value indicating the number of allowed retries.
             */
            static constexpr unsigned numberRetries = 2;

            /**
             * Method that issues a new time-delta request.
             */
            void issueTimeDeltaRequest();

            /**
             * Method that parses the received JSON response.
             *
             * \param[in] document The received JSON document.
             *
             * \return Returns true on success.  Returns false on error.
             */
            bool parseResponse(const QJsonDocument& document);

            /**
             * The network access manager to be used.
             */
            QNetworkAccessManager* currentNetworkAccessManager;

            /**
             * The current server URL and scheme.
             */
            QUrl currentSchemeAndHost;

            /**
             * The current time delta slug.
             */
            QString currentTimeDeltaSlug;

            /**
             * The current default secret to use for web requests.
             */
            QByteArray currentDefaultSecret;

            /**
             * The current user agent string.
             */
            QString currentUserAgent;

            /**
             * The last measured time delta.
             */
            long long currentTimeDelta;

            /**
             * Mutex used to prevent bad concurrent access.
             */
            QMutex requestMutex;

            /**
             * The network reply for the currently outstanding time delta network request.
             */
            QNetworkReply* pendingReply;

            /**
             * Value used to support retries to our server.
             */
            unsigned retriesRemaining;

            /**
             * A list of RestApi instances waiting for a timestamp update.
             */
            QList<RestApi*> waitingRestApis;
    };
}

#endif
