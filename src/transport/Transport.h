/**
 * Copyright (c) 2015, Affinegy, Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#pragma once


#include <string>

class TransportListener;

class Transport
{
    Transport() {}
public:

    typedef enum {
        uninitialized,
        connected,
        disconnected,
        connecting,
        disconnecting,
        error
    } ConnectionState;

    typedef enum {
        none,
        unknown,
        not_connected,
        auth_failure,
        host_unreachable
    } ConnectionError;

    struct mosquitto * mosq_trans;

    Transport(TransportListener* listener);
    ~Transport();

    ConnectionError Run();
    void Stop();
    void SetCredentials(const std::string& username, const std::string& password);
    void Connect();
    void Subscribe(const std::string& topic);
    ConnectionError Send(const std::string& topic, const std::string& message);
    ConnectionState GetConnectionState() const;
    ConnectionError GetConnectionError() const;
    ConnectionState SetConnectionState(const ConnectionState& new_state);
    ConnectionError SetConnectionError(const ConnectionError& error);

    

protected:
    virtual ConnectionError RunOnce() = 0;

    virtual void  SetCredentialsImpl(const std::string& username, const std::string& password) = 0;
    virtual void  ConnectImpl() = 0;

    virtual void StopImpl() = 0;
    virtual void SubscribeImpl(const std::string& topic) = 0;
    virtual ConnectionError SendImpl(const std::string& topic, const std::string& message) = 0;

    void MessageReceived(
        const std::string& source,
        const std::string& message
    );


    void
    GlobalConnectionStateChanged(
        const ConnectionState& new_state,
        const ConnectionError& error
    );

    void
    RemoteSourcePresenceStateChanged(
        const std::string&     source,
        const ConnectionState& new_state,
        const ConnectionError& error
    );

private:

    ConnectionState     m_connection_state;
    ConnectionError     m_connection_error;
    TransportListener*  m_listener;
    mutable void*       m_mtx; // void* to hide pthread header
};


class TransportListener
{

public:

    virtual void MessageReceived(
        const std::string& source,
        const std::string& message
        ) = 0;


    virtual
    void
    GlobalConnectionStateChanged(
        const Transport::ConnectionState& new_state,
        const Transport::ConnectionError& error
        ) = 0;


    virtual
    void
    RemoteSourcePresenceStateChanged(
        const std::string&                source,
        const Transport::ConnectionState& new_state,
        const Transport::ConnectionError& error
        ) = 0;

};

