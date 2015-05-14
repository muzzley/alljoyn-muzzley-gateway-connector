#include "XMPPConnector.h"                                                      // TODO: internal documentation
#include <vector>
#include <iostream>
#include <sstream>
#include <cerrno>
#include <stdint.h>
#include <pthread.h>
#include <strophe.h>

#include <alljoyn/about/AnnouncementRegistrar.h>
#include <qcc/StringUtil.h>

#include <zlib.h>
#include "xmppconnutil.h"


using namespace ajn;
using namespace ajn::gw;
using namespace ajn::services;
using namespace qcc;

using std::string;
using std::vector;
using std::list;
using std::map;
using std::cout;
using std::endl;
using std::istringstream;
using std::ostringstream;

// Forward declaration of XmppTransport class due to circular dependencies
class ajn::gw::XmppTransport
{
public:
    class Listener
    {
    public:
        typedef enum {
            XMPP_CONNECT    = XMPP_CONN_CONNECT,
            XMPP_DISCONNECT = XMPP_CONN_DISCONNECT,
            XMPP_FAIL       = XMPP_CONN_FAIL
        } XmppConnectionStatus;

        typedef
        void
        (XmppTransport::Listener::* ConnectionCallback)(
            XmppConnectionStatus status,
            void*                userdata
            );
    };

    typedef enum {
        xmpp_uninitialized,
        xmpp_disconnected,
        xmpp_connected,
        xmpp_error,
        xmpp_aborting
    } XmppConnectionState;

    XmppTransport(
        XMPPConnector* connector,
        const string&  jabberId,
        const string&  password,
        const string&  roster,
        const string&  chatroom,
        const string&  resource,
        const bool     compress
        );
    ~XmppTransport();

    void
    SetConnectionCallback(
        XmppTransport::Listener*                    listener,
        XmppTransport::Listener::ConnectionCallback callback,
        void*                                       userdata
        );
    void
    RemoveConnectionCallback();

    void
    NameOwnerChanged(
        const char* wellKnownName,
        const char* uniqueName
        );

    QStatus Run();
    void Stop();

    void
    SendAdvertisement(
        const string&                           name,
        const vector<util::bus::BusObjectInfo>& busObjects
        );
    void
    SendAdvertisementLost(
        const string& name
        );
    void
    SendAnnounce(
        uint16_t                                   version,
        uint16_t                                   port,
        const string&                              busName,
        const AnnounceHandler::ObjectDescriptions& objectDescs,
        const AnnounceHandler::AboutData&          aboutData,
        const vector<util::bus::BusObjectInfo>&    busObjects
        );
    void
    SendJoinRequest(
        const string&                           remoteName,
        SessionPort                             sessionPort,
        const char*                             joiner,
        const SessionOpts&                      opts,
        const vector<util::bus::BusObjectInfo>& busObjects
        );
    void
    SendJoinResponse(
        const string& joinee,
        SessionId     sessionId
        );
    void
    SendSessionJoined(
        const string& joiner,
        const string& joinee,
        SessionPort   port,
        SessionId     remoteId,
        SessionId     localId
        );
    void
    SendSessionLost(
        const string& peer,
        SessionId     id
        );
    void
    SendMethodCall(
        const InterfaceDescription::Member* member,
        Message&                            message,
        const string&                       busName,
        const string&                       objectPath
        );
    void
    SendMethodReply(
        const string& destName,
        const string& destPath,
        Message& reply
        );
    void
    SendSignal(
        const InterfaceDescription::Member* member,
        const char*                         srcPath,
        Message&                            message
        );
    void
    SendGetRequest(
        const string& ifaceName,
        const string& propName,
        const string& destName,
        const string& destPath
        );
    void
    SendGetReply(
        const string& destName,
        const string& destPath,
        const MsgArg& replyArg
        );
    void
    SendSetRequest(
        const string& ifaceName,
        const string& propName,
        const MsgArg& msgArg,
        const string& destName,
        const string& destPath
        );
    void
    SendSetReply(
        const string& destName,
        const string& destPath,
        QStatus       replyStatus
        );
    void
    SendGetAllRequest(
        const InterfaceDescription::Member* member,
        const string& destName,
        const string& destPath
        );
    void
    SendGetAllReply(
        const string& destName,
        const string& destPath,
        const MsgArg& replyArgs
        );

    void
    SendMessage(
        const string& body,
        const string& messageType = ""
        );

private:
    vector<XMPPConnector::RemoteObjectDescription>
    ParseBusObjectInfo(
        istringstream& msgStream
        );

    void ReceiveAdvertisement(const string& from, const string& message);
    void ReceiveAdvertisementLost(const string& from, const string& message);
    void ReceiveAnnounce(const string& from, const string& message);
    void ReceiveJoinRequest(const string& from, const string& message);
    void ReceiveJoinResponse(const string& from, const string& message);
    void ReceiveSessionJoined(const string& from, const string& message);
    void ReceiveSessionLost(const string& from, const string& message);
    void ReceiveMethodCall(const string& from, const string& message);
    void ReceiveMethodReply(const string& from, const string& message);
    void ReceiveSignal(const string& from, const string& message);
    void ReceiveGetRequest(const string& from, const string& message);
    void ReceiveGetReply(const string& from, const string& message);
    void ReceiveSetRequest(const string& from, const string& message);
    void ReceiveSetReply(const string& from, const string& message);
    void ReceiveGetAllRequest(const string& from, const string& message);
    void ReceiveGetAllReply(const string& from, const string& message);

    static
    int
    XmppStanzaHandler(
        xmpp_conn_t* const   conn,
        xmpp_stanza_t* const stanza,
        void* const          userdata
        );

    static
    int
    XmppPresenceHandler(
        xmpp_conn_t* const   conn,
        xmpp_stanza_t* const stanza,
        void* const          userdata
        );

    static
    int
    XmppRosterHandler(
        xmpp_conn_t* const   conn,
        xmpp_stanza_t* const stanza,
        void* const          userdata
        );

    static
    void
    XmppConnectionHandler(
        xmpp_conn_t* const         conn,
        const xmpp_conn_event_t    event,
        const int                  error,
        xmpp_stream_error_t* const streamError,
        void* const                userdata
        );

private:
    XMPPConnector* m_connector;
    const string   m_jabberId;
    const string   m_password;
    const string   m_roster;
    const string   m_chatroom;                                                  // TODO: moving away from using chatrooms
    const string   m_resource;
    const bool     m_compress;
    XmppConnectionState m_connectionState;

    xmpp_ctx_t*                  m_xmppCtx;
    xmpp_conn_t*                 m_xmppConn;
    Listener*                    m_callbackListener;
    Listener::ConnectionCallback m_connectionCallback;
    void*                        m_callbackUserdata;

    map<string, string> m_wellKnownNameMap;

    BusAttachment m_propertyBus;


    static const string ALLJOYN_CODE_ADVERTISEMENT;
    static const string ALLJOYN_CODE_ADVERT_LOST;
    static const string ALLJOYN_CODE_ANNOUNCE;
    static const string ALLJOYN_CODE_JOIN_REQUEST;
    static const string ALLJOYN_CODE_JOIN_RESPONSE;
    static const string ALLJOYN_CODE_SESSION_JOINED;
    static const string ALLJOYN_CODE_SESSION_LOST;
    static const string ALLJOYN_CODE_METHOD_CALL;
    static const string ALLJOYN_CODE_METHOD_REPLY;
    static const string ALLJOYN_CODE_SIGNAL;
    static const string ALLJOYN_CODE_GET_PROPERTY;
    static const string ALLJOYN_CODE_GET_PROP_REPLY;
    static const string ALLJOYN_CODE_SET_PROPERTY;
    static const string ALLJOYN_CODE_SET_PROP_REPLY;
    static const string ALLJOYN_CODE_GET_ALL;
    static const string ALLJOYN_CODE_GET_ALL_REPLY;
};


class ajn::gw::RemoteBusAttachment :
    public BusAttachment
{
public:
    RemoteBusAttachment(
        const string&  remoteName,
        XmppTransport* transport
        ) :
        BusAttachment(remoteName.c_str(), true),
        m_transport(transport),
        m_remoteName(remoteName),
        m_wellKnownName(""),
        m_listener(this, transport),
        m_objects(),
        m_activeSessions(),
        m_aboutPropertyStore(NULL),
        m_aboutBusObject(NULL)
    {
        pthread_mutex_init(&m_activeSessionsMutex, NULL);

        RegisterBusListener(m_listener);
    }

    ~RemoteBusAttachment()
    {
        vector<RemoteBusObject*>::iterator it;
        for(it = m_objects.begin(); it != m_objects.end(); ++it)
        {
            UnregisterBusObject(**it);
            delete(*it);
        }

        if(m_aboutBusObject)
        {
            UnregisterBusObject(*m_aboutBusObject);
            m_aboutBusObject->Unregister();
            delete m_aboutBusObject;
            delete m_aboutPropertyStore;
        }

        UnregisterBusListener(m_listener);

        pthread_mutex_destroy(&m_activeSessionsMutex);
    }

    QStatus
    BindSessionPort(
        SessionPort port
        )
    {
        SessionOpts opts(SessionOpts::TRAFFIC_MESSAGES, true,
                SessionOpts::PROXIMITY_ANY, TRANSPORT_ANY);
        return BusAttachment::BindSessionPort(port, opts, m_listener);
    }

    QStatus
    JoinSession(
        const string& host,
        SessionPort   port,
        SessionId&    id
        )
    {
        FNLOG
        SessionOpts opts(SessionOpts::TRAFFIC_MESSAGES, true,
                SessionOpts::PROXIMITY_ANY, TRANSPORT_ANY);
        return BusAttachment::JoinSession(
                host.c_str(), port, &m_listener, id, opts);
    }

    QStatus
    RegisterSignalHandler(
        const InterfaceDescription::Member* member
        )
    {
        FNLOG
        const char* ifaceName = member->iface->GetName();
        if(ifaceName == strstr(ifaceName, "org.alljoyn.Bus."))
        {
            return ER_OK;
        }

        // Unregister first in case we already registered for this particular
        //  interface member.
        UnregisterSignalHandler(
                this,
                static_cast<MessageReceiver::SignalHandler>(
                &RemoteBusAttachment::AllJoynSignalHandler),
                member, NULL);

        return BusAttachment::RegisterSignalHandler(
                this,
                static_cast<MessageReceiver::SignalHandler>(
                &RemoteBusAttachment::AllJoynSignalHandler),
                member, NULL);
    }

    void
    AllJoynSignalHandler(
        const InterfaceDescription::Member* member,
        const char*                         srcPath,
        Message&                            message
        )
    {
        LOG_DEBUG("Received signal from %s", message->GetSender());
        m_transport->SendSignal(member, srcPath, message);
    }

    QStatus
    AddRemoteObject(
        const string&                       path,
        vector<const InterfaceDescription*> interfaces
        )
    {
        QStatus err = ER_OK;
        RemoteBusObject* newObj = new RemoteBusObject(this, path, m_transport); // TODO: why pointers?

        err = newObj->ImplementInterfaces(interfaces);
        if(err != ER_OK)
        {
            delete newObj;
            return err;
        }

        err = RegisterBusObject(*newObj);
        if(err != ER_OK)
        {
            LOG_RELEASE("Failed to register remote bus object: %s",
                    QCC_StatusText(err));
            delete newObj;
            return err;
        }

        m_objects.push_back(newObj);
        return err;
    }

    string
    RequestWellKnownName(
        const string& remoteName
        )
    {
        if(remoteName.find_first_of(":") != 0)
        {
            // Request and advertise the new attachment's name
            QStatus err = RequestName(remoteName.c_str(),
                    DBUS_NAME_FLAG_ALLOW_REPLACEMENT|DBUS_NAME_FLAG_DO_NOT_QUEUE
                    );
            if(err != ER_OK)
            {
                LOG_RELEASE("Could not acquire well known name %s: %s",
                        remoteName.c_str(), QCC_StatusText(err));
                m_wellKnownName = "";
            }
            else
            {
                m_wellKnownName = remoteName;
            }
        }
        else
        {
            // We have a device advertising its unique name.
            m_wellKnownName = GetUniqueName().c_str();
        }

        return m_wellKnownName;
    }

    void
    AddSession(
        SessionId     localId,
        const string& peer,
        SessionPort   port,
        SessionId     remoteId
        )
    {
        SessionInfo info = {peer, port, remoteId};

        pthread_mutex_lock(&m_activeSessionsMutex);
        m_activeSessions[localId] = info;
        pthread_mutex_unlock(&m_activeSessionsMutex);
    }

    void
    RemoveSession(
        SessionId localId
        )
    {
        pthread_mutex_lock(&m_activeSessionsMutex);
        m_activeSessions.erase(localId);
        pthread_mutex_unlock(&m_activeSessionsMutex);
    }

    SessionId
    GetLocalSessionId(
        SessionId remoteId
        )
    {
        SessionId retval = 0;

        pthread_mutex_lock(&m_activeSessionsMutex);
        map<SessionId, SessionInfo>::iterator it;
        for(it = m_activeSessions.begin(); it != m_activeSessions.end(); ++it)
        {
            if(it->second.remoteId == remoteId)
            {
                retval = it->first;
                break;
            }
        }
        pthread_mutex_unlock(&m_activeSessionsMutex);

        return retval;
    }

    SessionId
    GetSessionIdByPeer(
        const string& peer
        )
    {
        SessionId retval = 0;

        pthread_mutex_lock(&m_activeSessionsMutex);
        map<SessionId, SessionInfo>::iterator it;
        for(it = m_activeSessions.begin(); it != m_activeSessions.end(); ++it)
        {
            if(it->second.peer == peer)
            {
                retval = it->first;
                break;
            }
        }
        pthread_mutex_unlock(&m_activeSessionsMutex);

        return retval;
    }

    string
    GetPeerBySessionId(
        SessionId id
        )
    {
        string retval = "";

        pthread_mutex_lock(&m_activeSessionsMutex);
        map<SessionId, SessionInfo>::iterator it = m_activeSessions.find(id);
        if(it != m_activeSessions.end())
        {
            retval = it->second.peer;
        }
        pthread_mutex_unlock(&m_activeSessionsMutex);

        return retval;
    }

    void
    RelayAnnouncement(
        uint16_t                                   version,
        uint16_t                                   port,
        const string&                              busName,
        const AnnounceHandler::ObjectDescriptions& objectDescs,
        const AnnounceHandler::AboutData&          aboutData
        )
    {
        QStatus err = ER_OK;
        LOG_DEBUG("Relaying announcement for %s", m_wellKnownName.c_str());

        if(m_aboutBusObject)
        {
            // Already announced. Announcement must have been updated.
            UnregisterBusObject(*m_aboutBusObject);
            delete m_aboutBusObject;
            delete m_aboutPropertyStore;
        }

        // Set up our About bus object
        m_aboutPropertyStore = new AboutPropertyStore();
        err = m_aboutPropertyStore->SetAnnounceArgs(aboutData);
        if(err != ER_OK)
        {
            LOG_RELEASE("Failed to set About announcement args for %s: %s",
                    m_wellKnownName.c_str(), QCC_StatusText(err));
            delete m_aboutPropertyStore;
            m_aboutPropertyStore = 0;
            return;
        }

        m_aboutBusObject = new AboutBusObject(this, *m_aboutPropertyStore);
        err = m_aboutBusObject->AddObjectDescriptions(objectDescs);
        if(err != ER_OK)
        {
            LOG_RELEASE("Failed to add About object descriptions for %s: %s",
                    m_wellKnownName.c_str(), QCC_StatusText(err));
            return;
        }

        // Bind and register the announced session port
        err = BindSessionPort(port);
        if(err != ER_OK)
        {
            LOG_RELEASE("Failed to bind About announcement session port for %s: %s",
                    m_wellKnownName.c_str(), QCC_StatusText(err));
            return;
        }
        err = m_aboutBusObject->Register(port);
        if(err != ER_OK)
        {
            LOG_RELEASE("Failed to register About announcement port for %s: %s",
                    m_wellKnownName.c_str(), QCC_StatusText(err));
            return;
        }

        // Register the bus object
        err = RegisterBusObject(*m_aboutBusObject);
        if(err != ER_OK)
        {
            LOG_RELEASE("Failed to register AboutService bus object: %s",
                    QCC_StatusText(err));
        }

        // Make the announcement
        err = m_aboutBusObject->Announce();
        if(err != ER_OK)
        {
            LOG_RELEASE("Failed to relay announcement for %s: %s",
                    m_wellKnownName.c_str(), QCC_StatusText(err));
            return;
        }
    }

    void
    RelaySignal(
        const string&         destination,
        SessionId             sessionId,
        const string&         objectPath,
        const string&         ifaceName,
        const string&         ifaceMember,
        const vector<MsgArg>& msgArgs
        )
    {
        LOG_DEBUG("Relaying signal on %s", m_remoteName.c_str());

        // Find the bus object to relay the signal
        RemoteBusObject* busObject = 0;
        vector<RemoteBusObject*>::iterator objIter;
        for(objIter = m_objects.begin(); objIter != m_objects.end(); ++objIter)
        {
            if(objectPath == (*objIter)->GetPath())
            {
                busObject = *objIter;
                break;
            }
        }

        if(busObject)
        {
            busObject->SendSignal(
                    destination, sessionId,
                    ifaceName, ifaceMember, msgArgs);
        }
        else
        {
            LOG_RELEASE("Could not find bus object to relay signal");
        }
    }

    void
    SignalReplyReceived(
        const string& objectPath,
        const string& replyStr
        )
    {
        vector<RemoteBusObject*>::iterator it;
        for(it = m_objects.begin(); it != m_objects.end(); ++it)
        {
            if(objectPath == (*it)->GetPath())
            {
                (*it)->SignalReplyReceived(replyStr);
                break;
            }
        }
    }

    void
    SignalSessionJoined(
        SessionId result
        )
    {
        m_listener.SignalSessionJoined(result);
    }

    string WellKnownName() const { return m_wellKnownName; }
    string RemoteName() const { return m_remoteName; }

private:
    class AboutPropertyStore :
        public PropertyStore
    {
    public:
        AboutPropertyStore()
        {}

        QStatus
        SetAnnounceArgs(
            const AnnounceHandler::AboutData& aboutData
            )
        {
            // Construct the property store args
            vector<MsgArg> dictArgs;
            AnnounceHandler::AboutData::const_iterator aboutIter;
            for(aboutIter = aboutData.begin();
                aboutIter != aboutData.end();
                ++aboutIter)
            {
                MsgArg dictEntry("{sv}",
                        aboutIter->first.c_str(), &aboutIter->second);
                dictEntry.Stabilize();                                          //cout << dictEntry.ToString(4) << endl;

                dictArgs.push_back(dictEntry);
            }

            QStatus err = m_announceArgs.Set("a{sv}",
                    dictArgs.size(), &dictArgs[0]);
            m_announceArgs.Stabilize();                                         //cout << m_announceArgs.ToString(4) << endl;

            return err;
        }

        QStatus
        ReadAll(
            const char* languageTag,
            Filter      filter,
            MsgArg&     all
            )
        {
            all = m_announceArgs;                                               //cout << "ReadAll called:" << all.ToString() << endl;
            return ER_OK;
        }

    private:
        MsgArg m_announceArgs;
    };

    class AboutBusObject :
        public AboutService
    {
    public:
        AboutBusObject(
            RemoteBusAttachment* bus,
            AboutPropertyStore& propertyStore
            ) :
            AboutService(*bus, propertyStore)
        {}

        QStatus
        AddObjectDescriptions(
            const AnnounceHandler::ObjectDescriptions& objectDescs
            )
        {
            QStatus err = ER_OK;

            AnnounceHandler::ObjectDescriptions::const_iterator it;
            for(it = objectDescs.begin(); it != objectDescs.end(); ++it)
            {
                if(it->first == "/About")
                {
                    continue;
                }

                err = AddObjectDescription(it->first, it->second);
                if(err != ER_OK)
                {
                    LOG_RELEASE("Failed to add object description for %s: %s",
                            it->first.c_str(), QCC_StatusText(err));
                    break;
                }
            }

            return err;
        }
    };

    class RemoteBusListener :
        public BusListener,
        public SessionListener,
        public SessionPortListener
    {
    public:
        RemoteBusListener(
            RemoteBusAttachment* bus,
            XmppTransport*       transport
            ) :
            m_bus(bus),
            m_transport(transport),
            m_sessionJoinedSignalReceived(false),
            m_remoteSessionId(0),
            m_pendingSessionJoiners()
        {
            pthread_mutex_init(&m_sessionJoinedMutex, NULL);
            pthread_cond_init(&m_sessionJoinedWaitCond, NULL);
        }

        virtual
        ~RemoteBusListener()
        {
            pthread_mutex_destroy(&m_sessionJoinedMutex);
            pthread_cond_destroy(&m_sessionJoinedWaitCond);
        }

        bool
        AcceptSessionJoiner(
            SessionPort        sessionPort,
            const char*        joiner,
            const SessionOpts& opts
            )
        {
            m_bus->EnableConcurrentCallbacks();

            // Gather interfaces to be implemented on the remote end
            vector<util::bus::BusObjectInfo> busObjects;
            ProxyBusObject proxy(*m_bus, joiner, "/", 0);
            util::bus::GetBusObjectsRecursive(busObjects, proxy);

            // Lock the session join mutex
            pthread_mutex_lock(&m_sessionJoinedMutex);
            m_sessionJoinedSignalReceived = false;
            m_remoteSessionId = 0;

            // Send the session join request via XMPP
            m_transport->SendJoinRequest(
                    m_bus->RemoteName(), sessionPort, joiner, opts, busObjects);

            // Wait for the XMPP response signal
            timespec wait_time = {time(NULL)+10, 0};
            while(!m_sessionJoinedSignalReceived)
            {
                if(ETIMEDOUT == pthread_cond_timedwait(
                        &m_sessionJoinedWaitCond,
                        &m_sessionJoinedMutex,
                        &wait_time))
                {
                    break;
                }
            }

            bool returnVal = (m_remoteSessionId != 0);
            if(returnVal)
            {
                m_pendingSessionJoiners[joiner] = m_remoteSessionId;
            }

            pthread_mutex_unlock(&m_sessionJoinedMutex);

            return returnVal;
        }

        void
        SessionJoined(
            SessionPort port,
            SessionId   id,
            const char* joiner)
        {
            // Find the id of the remote session
            SessionId remoteSessionId = 0;
            pthread_mutex_lock(&m_sessionJoinedMutex);
            map<string, SessionId>::iterator idIter =
                    m_pendingSessionJoiners.find(joiner);
            if(idIter != m_pendingSessionJoiners.end())
            {
                remoteSessionId = idIter->second;
                m_pendingSessionJoiners.erase(idIter);
            }
            pthread_mutex_unlock(&m_sessionJoinedMutex);

            // Register as a session listener
            m_bus->SetSessionListener(id, this);

            // Store the remote/local session id pair
            m_bus->AddSession(id, joiner, port, remoteSessionId);

            // Send the session Id back across the XMPP server
            m_transport->SendSessionJoined(
                    joiner, m_bus->RemoteName(), port, remoteSessionId, id);

            LOG_DEBUG("Session joined between %s (remote) and %s (local). Port: %d Id: %d",
                    joiner, m_bus->RemoteName().c_str(), port, id);
        }

        void
        SessionLost(
            SessionId         id,
            SessionLostReason reason
            )
        {
            LOG_DEBUG("Session lost. Attachment: %s Id: %d",
                    m_bus->RemoteName().c_str(), id);

            string peer = m_bus->GetPeerBySessionId(id);
            if(!peer.empty())
            {
                m_bus->RemoveSession(id);
                m_transport->SendSessionLost(peer, id);
            }
        }

        void
        SignalSessionJoined(
            SessionId result
            )
        {
            pthread_mutex_lock(&m_sessionJoinedMutex);
            m_sessionJoinedSignalReceived = true;
            m_remoteSessionId = result;
            pthread_cond_signal(&m_sessionJoinedWaitCond);
            pthread_mutex_unlock(&m_sessionJoinedMutex);
        }

    private:
        RemoteBusAttachment* m_bus;
        XmppTransport*       m_transport;

        bool m_sessionJoinedSignalReceived;
        SessionId m_remoteSessionId;
        pthread_mutex_t m_sessionJoinedMutex;
        pthread_cond_t m_sessionJoinedWaitCond;

        map<string, SessionId> m_pendingSessionJoiners;
    };

    class RemoteBusObject :
        public BusObject
    {
    public:
        RemoteBusObject(
            RemoteBusAttachment* bus,
            const string&        path,
            XmppTransport*       transport
            ) :
            BusObject(path.c_str()),
            m_bus(bus),
            m_transport(transport),
            m_interfaces(),
            m_replyHandler()
        {}

        virtual
        ~RemoteBusObject()
        {}

        void
        AllJoynMethodHandler(
            const InterfaceDescription::Member* member,
            Message&                            message
            )
        {
            LOG_DEBUG("Received method call: %s", member->name.c_str());
            bool replyReceived = false;
            string replyStr;

            {
                ScopedTransactionLocker transLock(&m_replyHandler);

                m_transport->SendMethodCall(
                        member, message, m_bus->RemoteName(), GetPath());

                // Wait for the XMPP response signal
                transLock.ReceiveReply(replyReceived, replyStr);
            }

            if(replyReceived)
            {
                vector<MsgArg> replyArgs =
                        util::msgarg::VectorFromString(replyStr);
                QStatus err = MethodReply(
                        message, &replyArgs[0], replyArgs.size());
                if(err != ER_OK)
                {
                    LOG_RELEASE("Failed to reply to method call: %s",
                            QCC_StatusText(err));
                }
            }
        }

        QStatus
        ImplementInterfaces(
            const vector<const InterfaceDescription*>& interfaces
            )
        {
            vector<const InterfaceDescription*>::const_iterator it;
            for(it = interfaces.begin(); it != interfaces.end(); ++it)
            {
                QStatus err = AddInterface(**it);
                if(ER_OK != err)
                {
                    LOG_RELEASE("Failed to add interface %s: %s",
                            (*it)->GetName(), QCC_StatusText(err));
                    return err;
                }

                m_interfaces.push_back(*it);

                // Register method handlers
                size_t numMembers = (*it)->GetMembers();
                InterfaceDescription::Member** interfaceMembers =
                        new InterfaceDescription::Member*[numMembers];
                numMembers = (*it)->GetMembers(
                        (const InterfaceDescription::Member**)interfaceMembers,
                        numMembers);

                for(uint32_t i = 0; i < numMembers; ++i)
                {
                    if(interfaceMembers[i]->memberType == MESSAGE_SIGNAL)
                    {
                        err = m_bus->RegisterSignalHandler(interfaceMembers[i]);
                    }
                    else
                    {
                        err = AddMethodHandler(interfaceMembers[i],
                                static_cast<MessageReceiver::MethodHandler>(
                                &RemoteBusObject::AllJoynMethodHandler));       //cout << "Registered method handler for " << m_bus->RemoteName() << GetPath() << ":" << interfaceMembers[i]->name << endl;
                    }
                    if(err != ER_OK)
                    {
                        LOG_RELEASE("Failed to add method handler for %s: %s",
                                interfaceMembers[i]->name.c_str(), QCC_StatusText(err));
                    }
                }

                delete[] interfaceMembers;
            }

            return ER_OK;
        }

        void
        SendSignal(
            const string&         destination,
            SessionId             sessionId,
            const string&         ifaceName,
            const string&         ifaceMember,
            const vector<MsgArg>& msgArgs
            )
        {
            QStatus err = ER_FAIL;

            // Get the InterfaceDescription::Member
            vector<const InterfaceDescription*>::iterator ifaceIter;
            for(ifaceIter = m_interfaces.begin();
                ifaceIter != m_interfaces.end();
                ++ifaceIter)
            {
                if(ifaceName == (*ifaceIter)->GetName())
                {
                    size_t numMembers = (*ifaceIter)->GetMembers();
                    InterfaceDescription::Member** members =
                            new InterfaceDescription::Member*[numMembers];
                    numMembers = (*ifaceIter)->GetMembers(
                            (const InterfaceDescription::Member**)members,
                            numMembers);
                    for(uint32_t i = 0; i < numMembers; ++i)
                    {
                        if(ifaceMember == members[i]->name.c_str())
                        {
                            err = Signal(
                                    (destination.empty() ?
                                    NULL : destination.c_str()), sessionId,
                                    *members[i], &msgArgs[0], msgArgs.size());
                            if(err != ER_OK)
                            {
                                LOG_RELEASE("Failed to send signal: %s",
                                        QCC_StatusText(err));
                                err = ER_OK;
                            }
                            break;
                        }
                    }

                    delete[] members;
                    break;
                }
            }

            if(err != ER_OK)
            {
                LOG_RELEASE("Could not find interface member of signal to relay! %s",
                        QCC_StatusText(err));
            }
        }

        void
        SignalReplyReceived(
            const string& replyStr
            )
        {
            m_replyHandler.SetReply(replyStr);
        }

    protected:
        QStatus
        Get(
            const char* ifaceName,
            const char* propName,
            MsgArg&     val
            )
        {
            LOG_DEBUG("Received AllJoyn Get request for %s: %s",
                    ifaceName, propName);
            bool replyReceived = false;
            string replyStr;

            {
                ScopedTransactionLocker transLock(&m_replyHandler);

                m_transport->SendGetRequest(
                        ifaceName, propName, m_bus->RemoteName(), GetPath());

                // Wait for the XMPP response signal
                transLock.ReceiveReply(replyReceived, replyStr);
            }

            if(replyReceived)
            {
                MsgArg arg(util::msgarg::FromString(replyStr));
                if(arg.Signature() == "v") {
                    val = *arg.v_variant.val;
                } else {
                    val = arg;
                }
                val.Stabilize();
                return ER_OK;
            }
            else
            {
                return ER_BUS_NO_SUCH_PROPERTY;
            }
        }

        QStatus
        Set(
            const char* ifaceName,
            const char* propName,
            MsgArg&     val
            )
        {
            LOG_DEBUG("Received AllJoyn Set request for %s: %s",
                    ifaceName, propName);
            bool replyReceived = false;
            string replyStr;

            {
                ScopedTransactionLocker transLock(&m_replyHandler);

                m_transport->SendSetRequest(
                        ifaceName, propName, val, m_bus->RemoteName(), GetPath());

                // Wait for the XMPP response signal
                transLock.ReceiveReply(replyReceived, replyStr);
            }

            if(replyReceived)
            {
                return static_cast<QStatus>(strtol(replyStr.c_str(), NULL, 10));
            }
            else
            {
                return ER_BUS_NO_SUCH_PROPERTY;
            }
        }

        void
        GetAllProps(
            const InterfaceDescription::Member* member,
            Message&                            msg
            )
        {
            LOG_DEBUG("Received AllJoyn GetAllProps request for %s: %s",
                    member->iface->GetName(), member->name.c_str());
            bool replyReceived = false;
            string replyStr;

            {
                ScopedTransactionLocker transLock(&m_replyHandler);

                m_transport->SendGetAllRequest(
                        member, m_bus->RemoteName(), GetPath());

                // Wait for the XMPP response signal
                transLock.ReceiveReply(replyReceived, replyStr);
            }

            if(replyReceived)
            {
                MsgArg result = util::msgarg::FromString(replyStr);
                QStatus err = MethodReply(msg, &result, 1);
                if(err != ER_OK)
                {
                    LOG_RELEASE("Failed to send method reply to GetAllProps request: %s",
                            QCC_StatusText(err));
                }
            }
        }

        void
        ObjectRegistered()
        {
            string remoteName = m_bus->RemoteName();
            LOG_DEBUG("%s %s registered",
                    (remoteName.at(0) == ':' ? bus->GetUniqueName().c_str() : remoteName.c_str()),
                    GetPath());
        }

    private:
        // Forward declaration for friend class status
        class ScopedTransactionLocker;

        // Holds the mutexes needed to perform AllJoyn conversations over XMPP
        class ReplyHandler
        {
            friend class ScopedTransactionLocker;

        public:
            ReplyHandler() :
                m_replyReceived(false),
                m_replyStr()
            {
                pthread_mutex_init(&m_transactionMutex, NULL);
                pthread_mutex_init(&m_replyReceivedMutex, NULL);
                pthread_cond_init(&m_replyReceivedWaitCond, NULL);
            }
            ~ReplyHandler()
            {
                pthread_cond_destroy(&m_replyReceivedWaitCond);
                pthread_mutex_destroy(&m_replyReceivedMutex);
                pthread_mutex_destroy(&m_transactionMutex);
            }

            void
            SetReply(
                string const& replyStr
                )
            {
                pthread_mutex_lock(&m_replyReceivedMutex);
                m_replyReceived = true;
                m_replyStr = replyStr;
                pthread_cond_signal(&m_replyReceivedWaitCond);
                pthread_mutex_unlock(&m_replyReceivedMutex);
            }

        private:
            bool   m_replyReceived;
            string m_replyStr;

            pthread_mutex_t m_transactionMutex;
            pthread_mutex_t m_replyReceivedMutex;
            pthread_cond_t  m_replyReceivedWaitCond;
        };

        // Facilitates AllJoyn transactions over XMPP via scoped mutex locking
        class ScopedTransactionLocker
        {
        public:
            ScopedTransactionLocker(
                ReplyHandler* replyHandler
                ) :
                m_replyHandler(replyHandler)
            {
                pthread_mutex_lock(&m_replyHandler->m_transactionMutex);
                pthread_mutex_lock(&m_replyHandler->m_replyReceivedMutex);
            }
            ~ScopedTransactionLocker()
            {
                pthread_mutex_unlock(&m_replyHandler->m_replyReceivedMutex);
                pthread_mutex_unlock(&m_replyHandler->m_transactionMutex);
            }

            void
            ReceiveReply(
                bool&   replyReceived,
                string& replyStr
                )
            {
                timespec wait_time = {time(NULL)+10, 0};
                while(!m_replyHandler->m_replyReceived)
                {
                    if(ETIMEDOUT == pthread_cond_timedwait(
                            &m_replyHandler->m_replyReceivedWaitCond,
                            &m_replyHandler->m_replyReceivedMutex,
                            &wait_time))
                    {
                        break;
                    }
                }

                replyReceived = m_replyHandler->m_replyReceived;
                replyStr = m_replyHandler->m_replyStr;

                m_replyHandler->m_replyReceived = false;
                m_replyHandler->m_replyStr.clear();
            }

        private:
            ReplyHandler* m_replyHandler;
        };


        RemoteBusAttachment*                m_bus;
        XmppTransport*                      m_transport;
        vector<const InterfaceDescription*> m_interfaces;
        ReplyHandler                        m_replyHandler;
    };

    XmppTransport*           m_transport;
    string                   m_remoteName;
    string                   m_wellKnownName;
    RemoteBusListener        m_listener;
    vector<RemoteBusObject*> m_objects;

    struct SessionInfo
    {
        string      peer;
        SessionPort port;
        SessionId   remoteId;
    };
    map<SessionId, SessionInfo> m_activeSessions;
    pthread_mutex_t             m_activeSessionsMutex;

    AboutPropertyStore*       m_aboutPropertyStore;
    AboutBusObject*           m_aboutBusObject;
};


class ajn::gw::AllJoynListener :
    public BusListener,
    public SessionPortListener,
    public AnnounceHandler,
    public ProxyBusObject::Listener,
    public util::bus::GetBusObjectsAsyncReceiver
{
public:
    AllJoynListener(
        XMPPConnector* connector,
        XmppTransport* transport,
        BusAttachment* bus
        ) :
        BusListener(),
        m_connector(connector),
        m_transport(transport),
        m_bus(bus)
    {
    }

    virtual
    ~AllJoynListener()
    {
        m_bus->UnregisterAllHandlers(this);
    }

    void
    GetBusObjectsCallback(
        ProxyBusObject*                     obj,
        vector<util::bus::BusObjectInfo>    busObjects,
        void*                               context
        )
    {
        FNLOG

        // Send the advertisement via XMPP
        m_transport->SendAdvertisement(
            obj->GetServiceName().c_str(), busObjects);

        IntrospectCallbackContext* ctx =
            static_cast<IntrospectCallbackContext*>(context);
        delete ctx->proxy;
        delete ctx;
    }

    void
    GetBusObjectsAnnouncementCallback(
        ProxyBusObject*                     obj,
        vector<util::bus::BusObjectInfo>    busObjects,
        void*                               context
        )
    {
        FNLOG
        IntrospectCallbackContext* ctx =
            static_cast<IntrospectCallbackContext*>(context);

        // Send the announcement via XMPP
        m_transport->SendAnnounce(
            ctx->AnnouncementInfo.version,
            ctx->AnnouncementInfo.port,
            ctx->AnnouncementInfo.busName,
            ctx->AnnouncementInfo.objectDescs,
            ctx->AnnouncementInfo.aboutData,
            busObjects);

        delete ctx->proxy;
        delete ctx;
    }

    void
    IntrospectCallback(
        QStatus         status,
        ProxyBusObject* obj,
        void*           context
        )
    {
        FNLOG
        /** TODO: REQUIRED
         * Register sessionless signal handlers for announcing/advertising apps
         * (need to implement the required interfaces on m_bus). Other method/
         * signal handlers are registered when a session is joined. This fix
         * MIGHT allow notifications to be handled naturally.
         */
        IntrospectCallbackContext* ctx =
                static_cast<IntrospectCallbackContext*>(context);

        if ( status != ER_OK )
        {
            LOG_RELEASE("Failed to introspect advertised attachment %s: %s",
                ctx->proxy->GetServiceName().c_str(), QCC_StatusText(status));
            delete ctx->proxy;
            delete ctx;
            return;
        }

        m_bus->EnableConcurrentCallbacks();

        if(ctx->introspectReason == IntrospectCallbackContext::advertisement)
        {
            if(status == ER_OK)
            {
                GetBusObjectsAsync(
                    ctx->proxy,
                    const_cast<GetBusObjectsAsyncReceiver*>(static_cast<const GetBusObjectsAsyncReceiver* const>(this)),
                    static_cast<GetBusObjectsAsyncReceiver::CallbackHandler>(&ajn::gw::AllJoynListener::GetBusObjectsCallback),
                    ctx
                );
                // NOTE: Don't delete the ctx and ctx->proxy here because they
                //  will be deleted in the callback
                return;
            }
        }
        else if(ctx->introspectReason ==
                IntrospectCallbackContext::announcement)
        {
            if(status == ER_OK)
            {
                vector<util::bus::BusObjectInfo> busObjects;
                GetBusObjectsAsync(
                    ctx->proxy,
                    const_cast<GetBusObjectsAsyncReceiver*>(static_cast<const GetBusObjectsAsyncReceiver* const>(this)),
                    static_cast<GetBusObjectsAsyncReceiver::CallbackHandler>(&ajn::gw::AllJoynListener::GetBusObjectsAnnouncementCallback),
                    ctx
                );
                // NOTE: Don't delete the ctx and ctx->proxy here because they
                //  will be deleted in the callback
                return;
            }
            else
            {
                LOG_RELEASE("Failed to introspect Announcing attachment: %s: %s",
                        obj->GetServiceName().c_str(), QCC_StatusText(status));
            }
        }

        // Clean up
        if(ctx->sessionId != 0)
        {
            m_bus->LeaveSession(ctx->sessionId);
        }
        delete ctx->proxy;
        delete ctx;
    }

    void
    FoundAdvertisedName(
        const char*   name,
        TransportMask transport,
        const char*   namePrefix
        )
    {
        // Do not re-advertise these
        if(name == strstr(name, "org.alljoyn.BusNode") ||
           name == strstr(name, "org.alljoyn.sl")      ||
           name == strstr(name, "org.alljoyn.About.sl"))
        {
            return;
        }

        // Do not send if we are the ones transmitting the advertisement
        if(m_connector->OwnsWellKnownName(name))
        {
            return;
        }

        FNLOG

        LOG_DEBUG("Found advertised name: %s", name);

        m_bus->EnableConcurrentCallbacks();

        // Get the objects and interfaces implemented by the advertising device
        ProxyBusObject* proxy = new ProxyBusObject(*m_bus, name, "/", 0);
        if(!proxy->IsValid())
        {
            LOG_RELEASE("Invalid ProxyBusObject for %s", name);
            delete proxy;
            return;
        }

        IntrospectCallbackContext* ctx = new IntrospectCallbackContext();
        ctx->introspectReason = IntrospectCallbackContext::advertisement;
        ctx->sessionId = 0;
        ctx->proxy = proxy;
        QStatus err = proxy->IntrospectRemoteObjectAsync(
                this,
                static_cast<ProxyBusObject::Listener::IntrospectCB>(
                &AllJoynListener::IntrospectCallback),
                ctx);
        if(err != ER_OK)
        {
            LOG_RELEASE("Failed asynchronous introspect for advertised attachment: %s",
                    QCC_StatusText(err));
            delete proxy;
            delete ctx;
            return;
        }
    }

    void
    LostAdvertisedName(
        const char*   name,
        TransportMask transport,
        const char*   namePrefix
        )
    {
        // These are not re-advertised by us
        if(name == strstr(name, "org.alljoyn.BusNode") ||
           name == strstr(name, "org.alljoyn.sl")     ||
           name == strstr(name, "org.alljoyn.About.sl"))
        {
            return;
        }
        FNLOG

        LOG_DEBUG("Lost advertised name: %s", name);
        m_transport->SendAdvertisementLost(name);
    }

    void
    NameOwnerChanged(
        const char* busName,
        const char* previousOwner,
        const char* newOwner
        )
    {
        FNLOG
        /**
         * TODO: REQUIRED
         * If owner changed to nobody, an Announcing app may have gone offline.
         * Send the busName to the XMPP server so that any remote connectors can
         * take down their copies of these apps.
         */

        if(!busName) { return; }

        //cout << "Detected name owner change: " << busName << "\n  " <<
        //        (previousOwner?previousOwner:"<NOBODY>") << " -> " <<
        //        (newOwner?newOwner:"<NOBODY>") << endl;
        m_transport->NameOwnerChanged(busName, newOwner);
    }

    void
    Announce(
        uint16_t                  version,
        uint16_t                  port,
        const char*               busName,
        const ObjectDescriptions& objectDescs,
        const AboutData&          aboutData
        )
    {
        // Is this announcement coming from us?
        if(m_connector->OwnsWellKnownName(busName))
        {
            return;
        }
        FNLOG

        LOG_DEBUG("Received Announce: %s", busName);
        m_bus->EnableConcurrentCallbacks();

        // Get the objects and interfaces implemented by the announcing device
        SessionId sid = 0;
        SessionOpts opts(SessionOpts::TRAFFIC_MESSAGES, true,
                SessionOpts::PROXIMITY_ANY, TRANSPORT_ANY);
        QStatus err = m_bus->JoinSession(busName, port, NULL, sid, opts);
        if(err != ER_OK && err != ER_ALLJOYN_JOINSESSION_REPLY_ALREADY_JOINED)
        {
            LOG_RELEASE("Failed to join session with Announcing device: %s",
                    QCC_StatusText(err));
            return;
        }

        ProxyBusObject* proxy = new ProxyBusObject(*m_bus, busName, "/", 0);
        if(!proxy->IsValid())
        {
            LOG_RELEASE("Invalid ProxyBusObject for %s", busName);
            delete proxy;
            return;
        }

        IntrospectCallbackContext* ctx = new IntrospectCallbackContext();
        ctx->introspectReason = IntrospectCallbackContext::announcement;
        ctx->AnnouncementInfo.version     = version;
        ctx->AnnouncementInfo.port        = port;
        ctx->AnnouncementInfo.busName     = busName;
        ctx->AnnouncementInfo.objectDescs = objectDescs;
        ctx->AnnouncementInfo.aboutData   = aboutData;
        ctx->sessionId = sid;
        ctx->proxy = proxy;
        err = proxy->IntrospectRemoteObjectAsync(
                this,
                static_cast<ProxyBusObject::Listener::IntrospectCB>(
                &AllJoynListener::IntrospectCallback),
                ctx);
        if(err != ER_OK)
        {
            LOG_RELEASE("Failed asynchronous introspect for announcing attachment: %s",
                    QCC_StatusText(err));
            delete ctx;
            delete proxy;
            m_bus->LeaveSession(sid);
            return;
        }
    }

private:
    struct IntrospectCallbackContext
    {
        enum
        {
            advertisement,
            announcement
        } introspectReason;

        struct
        {
            uint16_t           version;
            uint16_t           port;
            string             busName;
            ObjectDescriptions objectDescs;
            AboutData          aboutData;
        } AnnouncementInfo;

        SessionId       sessionId;
        ProxyBusObject* proxy;
    };

    XMPPConnector* m_connector;
    XmppTransport* m_transport;
    BusAttachment* m_bus;
};


XmppTransport::XmppTransport(
    XMPPConnector* connector,
    const string&  jabberId,
    const string&  password,
    const string&  roster,
    const string&  chatroom,
    const string&  resource,
    const bool     compress
    ) :
    m_connector(connector),
    m_jabberId(jabberId),
    m_password(password),
    m_roster(roster),
    m_chatroom(chatroom),
    m_resource(resource),
    m_compress(compress),
    m_connectionState(xmpp_uninitialized),
    m_callbackListener(NULL),
    m_connectionCallback(NULL),
    m_callbackUserdata(NULL),
    m_propertyBus("propertyBus")
{
    LOG_DEBUG("JID: %s Chatroom: %s Resource: %s", jabberId.c_str(), chatroom.c_str(), resource.c_str());
    xmpp_initialize();
    m_xmppCtx = xmpp_ctx_new(NULL, NULL);
    m_xmppConn = xmpp_conn_new(m_xmppCtx);

    m_propertyBus.Start();
    m_propertyBus.Connect();
}

XmppTransport::~XmppTransport()
{
    m_propertyBus.Disconnect();
    m_propertyBus.Stop();

    xmpp_conn_release(m_xmppConn);
    xmpp_ctx_free(m_xmppCtx);
    xmpp_shutdown();
}

void
XmppTransport::SetConnectionCallback(
    XmppTransport::Listener*                    listener,
    XmppTransport::Listener::ConnectionCallback callback,
    void*                                       userdata
    )
{
    m_callbackListener = listener;
    m_connectionCallback = callback;
    m_callbackUserdata = userdata;
}
void
XmppTransport::RemoveConnectionCallback()
{
    m_callbackListener = NULL;
    m_connectionCallback = NULL;
    m_callbackUserdata = NULL;
}

void
XmppTransport::NameOwnerChanged(
    const char* wellKnownName,
    const char* uniqueName
    )
{
    if(!wellKnownName)
    {
        return;
    }

    if(!uniqueName)
    {
        m_wellKnownNameMap.erase(wellKnownName);
    }
    else
    {
        m_wellKnownNameMap[wellKnownName] = uniqueName;
    }
}

QStatus
XmppTransport::Run()
{
    xmpp_conn_set_jid(m_xmppConn, (m_jabberId + "/" + m_resource).c_str());
    xmpp_conn_set_pass(m_xmppConn, m_password.c_str());
    // If we're using a chat room then listen to messages, otherwise
    //  assume directed communication (to m_roster)
    if ( m_chatroom.empty() ) {
        xmpp_handler_add(
            m_xmppConn, XmppStanzaHandler, NULL, NULL, "chat", this);
    }
    else {
        xmpp_handler_add(
            m_xmppConn, XmppStanzaHandler, NULL, "message", NULL, this);
    }
    xmpp_handler_add(
            m_xmppConn, XmppPresenceHandler, NULL, "presence", NULL, this);
    xmpp_handler_add(
            m_xmppConn, XmppRosterHandler, "jabber:iq:roster", "iq", NULL, this);
    if(0 != xmpp_connect_client(
            m_xmppConn, NULL, 0, XmppConnectionHandler, this))
    {
        LOG_RELEASE("Failed to connect to XMPP server.");
        return ER_FAIL;
    }

    // Listen for XMPP
    xmpp_run(m_xmppCtx);

    return ER_OK;
}

void
XmppTransport::Stop()
{
    m_connectionState = xmpp_aborting;
    xmpp_disconnect(m_xmppConn);
    xmpp_handler_delete(m_xmppConn, XmppStanzaHandler);
    xmpp_handler_delete(m_xmppConn, XmppPresenceHandler);
    xmpp_handler_delete(m_xmppConn, XmppRosterHandler);
}

void
XmppTransport::SendAdvertisement(
    const string&                           name,
    const vector<util::bus::BusObjectInfo>& busObjects
    )
{
    FNLOG
    // Find the unique name of the advertising attachment
    string uniqueName = name;
    map<string, string>::iterator wknIter = m_wellKnownNameMap.find(name);
    if(wknIter != m_wellKnownNameMap.end())
    {
        uniqueName = wknIter->second;
    }

    // Construct the text that will be the body of our message
    ostringstream msgStream;
    msgStream << ALLJOYN_CODE_ADVERTISEMENT << "\n";
    msgStream << uniqueName << "\n";
    msgStream << name << "\n";
    vector<util::bus::BusObjectInfo>::const_iterator it;
    for(it = busObjects.begin(); it != busObjects.end(); ++it)
    {
        msgStream << it->path << "\n";
        vector<const InterfaceDescription*>::const_iterator if_it;
        for(if_it = it->interfaces.begin();
            if_it != it->interfaces.end();
            ++if_it)
        {
            msgStream << (*if_it)->GetName() << "\n";
            msgStream << (*if_it)->Introspect().c_str() << "\n";
        }

        msgStream << "\n";
    }

    SendMessage(msgStream.str(), ALLJOYN_CODE_ADVERTISEMENT);
}

void
XmppTransport::SendAdvertisementLost(
    const string& name
    )
{
    FNLOG
    // Construct the text that will be the body of our message
    ostringstream msgStream;
    msgStream << ALLJOYN_CODE_ADVERT_LOST << "\n";
    msgStream << name << "\n";

    SendMessage(msgStream.str(), ALLJOYN_CODE_ADVERT_LOST);
}

void
XmppTransport::SendAnnounce(
    uint16_t                                   version,
    uint16_t                                   port,
    const string&                              busName,
    const AnnounceHandler::ObjectDescriptions& objectDescs,
    const AnnounceHandler::AboutData&          aboutData,
    const vector<util::bus::BusObjectInfo>&    busObjects
    )
{
    FNLOG
    // Find the unique name of the announcing attachment
    string uniqueName = busName;
    map<string, string>::iterator wknIter =
            m_wellKnownNameMap.find(busName);
    if(wknIter != m_wellKnownNameMap.end())
    {
        uniqueName = wknIter->second;
    }

    // Construct the text that will be the body of our message
    ostringstream msgStream;
    msgStream << ALLJOYN_CODE_ANNOUNCE << "\n";
    msgStream << uniqueName << "\n";
    msgStream << version << "\n";
    msgStream << port << "\n";
    msgStream << busName << "\n";

    AnnounceHandler::ObjectDescriptions::const_iterator objIter;
    for(objIter = objectDescs.begin(); objIter != objectDescs.end(); ++objIter)
    {
        msgStream << objIter->first.c_str() << "\n";
        vector<String>::const_iterator val_iter;
        for(val_iter = objIter->second.begin();
            val_iter != objIter->second.end();
            ++val_iter)
        {
            msgStream << val_iter->c_str() << "\n";
        }
    }

    msgStream << "\n";

    AnnounceHandler::AboutData::const_iterator aboutIter;
    for(aboutIter = aboutData.begin();
        aboutIter != aboutData.end();
        ++aboutIter)
    {
        msgStream << aboutIter->first.c_str() << "\n";
        msgStream << util::msgarg::ToString(aboutIter->second) << "\n\n";
    }

    msgStream << "\n";

    vector<util::bus::BusObjectInfo>::const_iterator it;
    for(it = busObjects.begin(); it != busObjects.end(); ++it)
    {
        msgStream << it->path << "\n";
        vector<const InterfaceDescription*>::const_iterator if_it;
        for(if_it = it->interfaces.begin();
            if_it != it->interfaces.end();
            ++if_it)
        {
            msgStream << (*if_it)->GetName() << "\n";
            msgStream << (*if_it)->Introspect().c_str() << "\n";
        }

        msgStream << "\n";
    }

    SendMessage(msgStream.str(), ALLJOYN_CODE_ANNOUNCE);
}

void
XmppTransport::SendJoinRequest(
    const string&                           remoteName,
    SessionPort                             sessionPort,
    const char*                             joiner,
    const SessionOpts&                      opts,
    const vector<util::bus::BusObjectInfo>& busObjects
    )
{
    FNLOG
    // Construct the text that will be the body of our message
    ostringstream msgStream;
    msgStream << ALLJOYN_CODE_JOIN_REQUEST << "\n";
    msgStream << remoteName << "\n";
    msgStream << sessionPort << "\n";
    msgStream << joiner << "\n";

    // Send the objects/interfaces to be implemented on the remote end
    vector<util::bus::BusObjectInfo>::const_iterator objIter;
    for(objIter = busObjects.begin(); objIter != busObjects.end(); ++objIter)
    {
        msgStream << objIter->path << "\n";
        vector<const InterfaceDescription*>::const_iterator ifaceIter;
        for(ifaceIter = objIter->interfaces.begin();
            ifaceIter != objIter->interfaces.end();
            ++ifaceIter)
        {
            string ifaceNameStr = (*ifaceIter)->GetName();
            if(ifaceNameStr != "org.freedesktop.DBus.Peer"           &&
               ifaceNameStr != "org.freedesktop.DBus.Introspectable" &&
               ifaceNameStr != "org.freedesktop.DBus.Properties"     &&
               ifaceNameStr != "org.allseen.Introspectable")
            {
                msgStream << ifaceNameStr << "\n";
                msgStream << (*ifaceIter)->Introspect().c_str() << "\n";
            }
        }

        msgStream << "\n";
    }

    SendMessage(msgStream.str(), ALLJOYN_CODE_JOIN_REQUEST);
}

void
XmppTransport::SendJoinResponse(
    const string& joinee,
    SessionId     sessionId
    )
{
    FNLOG
    // Send the status back to the original session joiner
    ostringstream msgStream;
    msgStream << ALLJOYN_CODE_JOIN_RESPONSE << "\n";
    msgStream << joinee << "\n";
    msgStream << sessionId << "\n";

    SendMessage(msgStream.str(), ALLJOYN_CODE_JOIN_RESPONSE);
}

void
XmppTransport::SendSessionJoined(
    const string& joiner,
    const string& joinee,
    SessionPort   port,
    SessionId     remoteId,
    SessionId     localId
    )
{
    FNLOG
    // Construct the text that will be the body of our message
    ostringstream msgStream;
    msgStream << ALLJOYN_CODE_SESSION_JOINED << "\n";
    msgStream << joiner << "\n";
    msgStream << joinee << "\n";
    msgStream << port << "\n";
    msgStream << remoteId << "\n";
    msgStream << localId << "\n";

    SendMessage(msgStream.str(), ALLJOYN_CODE_SESSION_JOINED);
}

void
XmppTransport::SendSessionLost(
    const string& peer,
    SessionId     id
    )
{
    FNLOG
    // Construct the text that will be the body of our message
    ostringstream msgStream;
    msgStream << ALLJOYN_CODE_SESSION_LOST << "\n";
    msgStream << peer << "\n";
    msgStream << id << "\n";

    SendMessage(msgStream.str(), ALLJOYN_CODE_SESSION_LOST);
}

void
XmppTransport::SendMethodCall(
    const InterfaceDescription::Member* member,
    Message&                            message,
    const string&                       busName,
    const string&                       objectPath
    )
{
    FNLOG
    size_t numArgs = 0;
    const MsgArg* msgArgs = 0;
    message->GetArgs(numArgs, msgArgs);

    // Construct the text that will be the body of our message
    ostringstream msgStream;
    msgStream << ALLJOYN_CODE_METHOD_CALL << "\n";
    msgStream << message->GetSender() << "\n";
    msgStream << busName << "\n";
    msgStream << objectPath << "\n";
    msgStream << member->iface->GetName() << "\n";
    msgStream << member->name << "\n";
    msgStream << message->GetSessionId() << "\n";
    msgStream << util::msgarg::ToString(msgArgs, numArgs) << "\n";

    SendMessage(msgStream.str(), ALLJOYN_CODE_METHOD_CALL);
}

void
XmppTransport::SendMethodReply(
    const string& destName,
    const string& destPath,
    Message&      reply
    )
{
    FNLOG
    size_t numReplyArgs;
    const MsgArg* replyArgs = 0;
    reply->GetArgs(numReplyArgs, replyArgs);

    ostringstream msgStream;
    msgStream << ALLJOYN_CODE_METHOD_REPLY << "\n";
    msgStream << destName << "\n";
    msgStream << destPath << "\n";
    msgStream << util::msgarg::ToString(replyArgs, numReplyArgs) << "\n";

    SendMessage(msgStream.str(), ALLJOYN_CODE_METHOD_REPLY);
}

void
XmppTransport::SendSignal(
    const InterfaceDescription::Member* member,
    const char*                         srcPath,
    Message&                            message
    )
{
    FNLOG
    // Find the unique name of the signal sender
    string senderUniqueName = message->GetSender();
    map<string, string>::iterator wknIter =
            m_wellKnownNameMap.find(senderUniqueName);
    if(wknIter != m_wellKnownNameMap.end())
    {
        senderUniqueName = wknIter->second;
    }

    // Get the MsgArgs
    size_t numArgs = 0;
    const MsgArg* msgArgs = 0;
    message->GetArgs(numArgs, msgArgs);

    // Construct the text that will be the body of our message
    ostringstream msgStream;
    msgStream << ALLJOYN_CODE_SIGNAL << "\n";
    msgStream << senderUniqueName << "\n";
    msgStream << message->GetDestination() << "\n";
    msgStream << message->GetSessionId() << "\n";
    msgStream << message->GetObjectPath() << "\n";
    msgStream << member->iface->GetName() << "\n";
    msgStream << member->name << "\n";
    msgStream << util::msgarg::ToString(msgArgs, numArgs) << "\n";

    SendMessage(msgStream.str(), ALLJOYN_CODE_SIGNAL);
}

void
XmppTransport::SendGetRequest(
    const string& ifaceName,
    const string& propName,
    const string& destName,
    const string& destPath
    )
{
    FNLOG
    // Construct the text that will be the body of our message
    ostringstream msgStream;
    msgStream << ALLJOYN_CODE_GET_PROPERTY << "\n";
    msgStream << destName << "\n";
    msgStream << destPath << "\n";
    msgStream << ifaceName << "\n";
    msgStream << propName << "\n";

    SendMessage(msgStream.str(), ALLJOYN_CODE_GET_PROPERTY);
}

void
XmppTransport::SendGetReply(
    const string& destName,
    const string& destPath,
    const MsgArg& replyArg
    )
{
    FNLOG
    // Return the reply
    ostringstream msgStream;
    msgStream << ALLJOYN_CODE_GET_PROP_REPLY << "\n";
    msgStream << destName << "\n";
    msgStream << destPath << "\n";
    msgStream << util::msgarg::ToString(replyArg) << "\n";

    SendMessage(msgStream.str(), ALLJOYN_CODE_GET_PROP_REPLY);
}

void
XmppTransport::SendSetRequest(
    const string& ifaceName,
    const string& propName,
    const MsgArg& msgArg,
    const string& destName,
    const string& destPath
    )
{
    FNLOG
    // Construct the text that will be the body of our message
    ostringstream msgStream;
    msgStream << ALLJOYN_CODE_SET_PROPERTY << "\n";
    msgStream << destName << "\n";
    msgStream << destPath << "\n";
    msgStream << ifaceName << "\n";
    msgStream << propName << "\n";
    msgStream << util::msgarg::ToString(msgArg) << "\n";

    SendMessage(msgStream.str(), ALLJOYN_CODE_SET_PROPERTY);
}

void
XmppTransport::SendSetReply(
    const string& destName,
    const string& destPath,
    QStatus       replyStatus
    )
{
    FNLOG
    // Return the reply
    ostringstream msgStream;
    msgStream << ALLJOYN_CODE_SET_PROP_REPLY << "\n";
    msgStream << destName << "\n";
    msgStream << destPath << "\n";
    msgStream << static_cast<uint32_t>(replyStatus) << "\n";

    SendMessage(msgStream.str(), ALLJOYN_CODE_SET_PROP_REPLY);
}

void
XmppTransport::SendGetAllRequest(
    const InterfaceDescription::Member* member,
    const string& destName,
    const string& destPath
    )
{
    FNLOG
    // Construct the text that will be the body of our message
    ostringstream msgStream;
    msgStream << ALLJOYN_CODE_GET_ALL << "\n";
    msgStream << destName << "\n";
    msgStream << destPath << "\n";
    msgStream << member->iface->GetName() << "\n";
    msgStream << member->name << "\n";

    SendMessage(msgStream.str(), ALLJOYN_CODE_GET_ALL);
}

void
XmppTransport::SendGetAllReply(
    const string& destName,
    const string& destPath,
    const MsgArg& replyArgs
    )
{
    FNLOG
    // Construct the text that will be the body of our message
    ostringstream msgStream;
    msgStream << ALLJOYN_CODE_GET_ALL_REPLY << "\n";
    msgStream << destName << "\n";
    msgStream << destPath << "\n";
    msgStream << util::msgarg::ToString(replyArgs) << "\n";

    SendMessage(msgStream.str(), ALLJOYN_CODE_GET_ALL_REPLY);
}

void
XmppTransport::SendMessage(
    const string& body,
    const string& messageType
    )
{
    FNLOG
    string bodyHex;
    if ( m_compress )
    {
        bodyHex = util::str::Compress(body);
    }
    else
    {
        bodyHex = body;
        util::str::EscapeXml(bodyHex);
    }

    xmpp_stanza_t* messageStanza = xmpp_stanza_new(m_xmppCtx);
    xmpp_stanza_set_name(messageStanza, "message");
    if ( m_chatroom.empty() ) {
        // TODO: When roster is actually a roster and not a destination
        //  we will need to instead add a parameter to this function
        //  and pass it in. We will also need to keep track of which AllJoyn
        //  bits are travelling to/from each destination. That will be done
        //  outside this class, however, so this function will blindly send
        //  to whoever was passed in as the destination.
        xmpp_stanza_set_attribute(messageStanza, "to", m_roster.c_str());
        xmpp_stanza_set_type(messageStanza, "chat");
    }
    else{
        xmpp_stanza_set_attribute(messageStanza, "to", m_chatroom.c_str());
        xmpp_stanza_set_type(messageStanza, "groupchat");
    }

    xmpp_stanza_t* bodyStanza = xmpp_stanza_new(m_xmppCtx);
    xmpp_stanza_set_name(bodyStanza, "body");

    xmpp_stanza_t* textStanza = xmpp_stanza_new(m_xmppCtx);
    xmpp_stanza_set_text(textStanza, bodyHex.c_str());

    xmpp_stanza_add_child(bodyStanza, textStanza);
    xmpp_stanza_release(textStanza);
    xmpp_stanza_add_child(messageStanza, bodyStanza);
    xmpp_stanza_release(bodyStanza);

    char* buf = NULL;
    size_t buflen = 0;
    xmpp_stanza_to_text(messageStanza, &buf, &buflen);
    LOG_DEBUG("Sending XMPP %smessage.", (messageType.empty() ? "" : (messageType+" ").c_str()));
    LOG_VERBOSE("Message: %s", buf);
    xmpp_free(m_xmppCtx, buf);

    xmpp_send(m_xmppConn, messageStanza);
    xmpp_stanza_release(messageStanza);
}

vector<XMPPConnector::RemoteObjectDescription>
XmppTransport::ParseBusObjectInfo(
    istringstream& msgStream
    )
{
    FNLOG
    vector<XMPPConnector::RemoteObjectDescription> results;
    XMPPConnector::RemoteObjectDescription thisObj;
    string interfaceName = "";
    string interfaceDescription = "";

    string line = "";
    while(getline(msgStream, line))
    {
        if(line.empty())
        {
            if(!interfaceDescription.empty())
            {
                // We've reached the end of an interface description.
                //util::str::UnescapeXml(interfaceDescription);
                thisObj.interfaces[interfaceName] = interfaceDescription;

                interfaceName.clear();
                interfaceDescription.clear();
            }
            else
            {
                // We've reached the end of a bus object.
                results.push_back(thisObj);

                thisObj.path.clear();
                thisObj.interfaces.clear();
            }
        }
        else
        {
            if(thisObj.path.empty())
            {
                thisObj.path = line.c_str();
            }
            else if(interfaceName.empty())
            {
                interfaceName = line;
            }
            else
            {
                interfaceDescription.append(line + "\n");
            }
        }
    }

    return results;
}

void
XmppTransport::ReceiveAdvertisement(
    const string& from,
    const string& message
    )
{
    FNLOG
    istringstream msgStream(message);
    string line;

    // First line is the type (advertisement)
    if(0 == getline(msgStream, line)){ return; }
    if(line != ALLJOYN_CODE_ADVERTISEMENT){ return; }

    // Second line is the name to advertise
    string remoteName, advertisedName;
    if(0 == getline(msgStream, remoteName)){ return; }                          //cout << "received XMPP advertised name: " << remoteName << endl; cout << message << endl;
    if(0 == getline(msgStream, advertisedName)){ return; }

    LOG_DEBUG("Received remote advertisement: %s", remoteName.c_str());

    vector<XMPPConnector::RemoteObjectDescription> objects =
            ParseBusObjectInfo(msgStream);
    RemoteBusAttachment* bus = m_connector->GetRemoteAttachment(
            from, remoteName, &objects);
    if(!bus)
    {
        return;
    }

    // Request and advertise our name
    string wkn = bus->WellKnownName();
    if(wkn.empty())
    {
        wkn = bus->RequestWellKnownName(advertisedName);
        if(wkn.empty())
        {
            m_connector->DeleteRemoteAttachment(from, bus);
            return;
        }
    }

    LOG_DEBUG("Advertising name: %s", wkn.c_str());
    QStatus err = bus->AdvertiseName(wkn.c_str(), TRANSPORT_ANY);
    if(err != ER_OK)
    {
        LOG_RELEASE("Failed to advertise %s: %s", wkn.c_str(),
                QCC_StatusText(err));
        m_connector->DeleteRemoteAttachment(from, bus);
        return;
    }
}

void
XmppTransport::ReceiveAdvertisementLost(
    const string& from,
    const string& message
    )
{
    FNLOG
    istringstream msgStream(message);
    string line, name;

    // First line is the type (advertisement)
    if(0 == getline(msgStream, line)){ return; }
    if(line != ALLJOYN_CODE_ADVERT_LOST){ return; }

    // Second line is the advertisement lost
    if(0 == getline(msgStream, name)){ return; }

    // Get the local bus attachment advertising this name
    RemoteBusAttachment* bus =
            m_connector->GetRemoteAttachmentByAdvertisedName(from, name);
    if(bus)
    {
        m_connector->DeleteRemoteAttachment(from, bus);
    }
}

void
XmppTransport::ReceiveAnnounce(
    const string& from,
    const string& message
    )
{
    FNLOG
    istringstream msgStream(message);
    string line, remoteName, versionStr, portStr, busName;

    // First line is the type (announcement)
    if(0 == getline(msgStream, line)){ return; }
    if(line != ALLJOYN_CODE_ANNOUNCE){ return; }

    // Get the info from the message
    if(0 == getline(msgStream, remoteName)){ return; }
    if(0 == getline(msgStream, versionStr)){ return; }
    if(0 == getline(msgStream, portStr)){ return; }
    if(0 == getline(msgStream, busName)){ return; }

    LOG_DEBUG("Received remote announcement: %s", busName.c_str());

    // The object descriptions follow
    AnnounceHandler::ObjectDescriptions objDescs;
    qcc::String objectPath = "";
    vector<qcc::String> interfaceNames;
    while(0 != getline(msgStream, line))
    {
        if(line.empty())
        {
            objDescs[objectPath] = interfaceNames;
            break;
        }

        if(objectPath.empty())
        {
            objectPath = line.c_str();
        }
        else
        {
            if(line[0] == '/')
            {
                // end of the object description
                objDescs[objectPath] = interfaceNames;

                interfaceNames.clear();
                objectPath = line.c_str();
            }
            else
            {
                interfaceNames.push_back(line.c_str());
            }
        }
    }

    // Then come the properties
    AnnounceHandler::AboutData aboutData;
    string propName = "", propDesc = "";
    while(0 != getline(msgStream, line))
    {
        if(line.empty())
        {
            if(propName.empty())
            {
                break;
            }

            // reached the end of a property
            aboutData[propName.c_str()] = util::msgarg::FromString(propDesc);

            propName.clear();
            propDesc.clear();
        }

        if(propName.empty())
        {
            propName = line;
        }
        else
        {
            propDesc += line;
        }
    }

    // Then the bus objects
    vector<XMPPConnector::RemoteObjectDescription> objects =
            ParseBusObjectInfo(msgStream);

    // Find or create the BusAttachment with the given app name
    RemoteBusAttachment* bus =
            m_connector->GetRemoteAttachment(from, remoteName, &objects);
    if(bus)
    {
        // Request and announce our name
        string wkn = bus->WellKnownName();
        if(wkn.empty())
        {
            wkn = bus->RequestWellKnownName(busName);
            if(wkn.empty())
            {
                m_connector->DeleteRemoteAttachment(from, bus);
                return;
            }
        }

        bus->RelayAnnouncement(
                strtoul(versionStr.c_str(), NULL, 10),
                strtoul(portStr.c_str(), NULL, 10),
                wkn,
                objDescs,
                aboutData);
    }
}

void
XmppTransport::ReceiveJoinRequest(
    const string& from,
    const string& message
    )
{
    FNLOG
    istringstream msgStream(message);
    string line, joiner, joinee, portStr;

    // First line is the type (join request)
    if(0 == getline(msgStream, line)){ return; }
    if(line != ALLJOYN_CODE_JOIN_REQUEST){ return; }

    // Next is the session port, joinee, and the joiner
    if(0 == getline(msgStream, joinee)){ return; }
    if(0 == getline(msgStream, portStr)){ return; }
    if(0 == getline(msgStream, joiner)){ return; }

    // Then follow the interfaces implemented by the joiner
    vector<XMPPConnector::RemoteObjectDescription> objects =
            ParseBusObjectInfo(msgStream);

    // Get or create a bus attachment to join from
    RemoteBusAttachment* bus = m_connector->GetRemoteAttachment(
            from, joiner, &objects);

    SessionId id = 0;
    if(!bus)
    {
        LOG_RELEASE("Failed to create bus attachment to proxy session!");
    }
    else
    {
        // Try to join a session of our own
        SessionPort port = strtoul(portStr.c_str(), NULL, 10);

        QStatus err = bus->JoinSession(joinee, port, id);
        if(err == ER_ALLJOYN_JOINSESSION_REPLY_ALREADY_JOINED)
        {
            id = bus->GetSessionIdByPeer(joinee);
            LOG_RELEASE("Session already joined between %s (local) and %s (remote). Port: %d Id: %d",
                    joiner.c_str(), joinee.c_str(), port, id);
        }
        else if(err != ER_OK)
        {
            LOG_RELEASE("Join session request rejected: %s",
                    QCC_StatusText(err));
        }
        else
        {
            LOG_DEBUG("Session joined between %s (local) and %s (remote). Port: %d Id: %d",
                    joiner.c_str(), joinee.c_str(), port, id);

            // Register signal handlers for the interfaces we're joining with   // TODO: this info could be sent via XMPP from the connector joinee instead of introspected again
            vector<util::bus::BusObjectInfo> joineeObjects;                     // TODO: do this before joining?
            ProxyBusObject proxy(*bus, joinee.c_str(), "/", id);
            util::bus::GetBusObjectsRecursive(joineeObjects, proxy);

            vector<util::bus::BusObjectInfo>::iterator objectIter;
            for(objectIter = joineeObjects.begin();
                objectIter != joineeObjects.end();
                ++objectIter)
            {
                vector<const InterfaceDescription*>::iterator ifaceIter;
                for(ifaceIter = objectIter->interfaces.begin();
                    ifaceIter != objectIter->interfaces.end();
                    ++ifaceIter)
                {
                    string interfaceName = (*ifaceIter)->GetName();

                    // Register signal listeners here.                          // TODO: sessionless signals? register on advertise/announce
                    size_t numMembers = (*ifaceIter)->GetMembers();
                    InterfaceDescription::Member** ifaceMembers =
                            new InterfaceDescription::Member*[numMembers];
                    numMembers = (*ifaceIter)->GetMembers(
                            (const InterfaceDescription::Member**)ifaceMembers,
                            numMembers);
                    for(uint32_t i = 0; i < numMembers; ++i)
                    {
                        if(ifaceMembers[i]->memberType == MESSAGE_SIGNAL)
                        {
                            err = bus->RegisterSignalHandler(ifaceMembers[i]);
                            if(err != ER_OK)
                            {
                                LOG_RELEASE("Could not register signal handler for %s: %s",
                                        interfaceName.c_str(), ifaceMembers[i]->name.c_str());
                            }
                        }
                    }
                    delete[] ifaceMembers;
                }
            }
        }
    }

    SendJoinResponse(joinee, id);
}

void
XmppTransport::ReceiveJoinResponse(
    const string& from,
    const string& message
    )
{
    FNLOG
    istringstream msgStream(message);
    string line, appName, remoteSessionId;

    // First line is the type (join response)
    if(0 == getline(msgStream, line)){ return; }
    if(line != ALLJOYN_CODE_JOIN_RESPONSE){ return; }

    // Get the info from the message
    if(0 == getline(msgStream, appName)){ return; }
    if(0 == getline(msgStream, remoteSessionId)){ return; }

    // Find the BusAttachment with the given app name
    RemoteBusAttachment* bus = m_connector->GetRemoteAttachment(from, appName);
    if(!bus)
    {
        LOG_RELEASE("Failed to find bus attachment to handle join response!");
    }
    else
    {
        bus->SignalSessionJoined(strtoul(remoteSessionId.c_str(), NULL, 10));
    }
}

void
XmppTransport::ReceiveSessionJoined(
    const string& from,
    const string& message
    )
{
    FNLOG
    istringstream msgStream(message);
    string line, joiner, joinee, portStr, remoteIdStr, localIdStr;

    // First line is the type (session joined)
    if(0 == getline(msgStream, line)){ return; }
    if(line != ALLJOYN_CODE_SESSION_JOINED){ return; }

    // Get the info from the message
    if(0 == getline(msgStream, joiner)){ return; }
    if(0 == getline(msgStream, joinee)){ return; }
    if(0 == getline(msgStream, portStr)){ return; }
    if(0 == getline(msgStream, localIdStr)){ return; }
    if(0 == getline(msgStream, remoteIdStr)){ return; }

    if(localIdStr.empty() || remoteIdStr.empty())
    {
        return;
    }

    // Find the BusAttachment with the given app name
    RemoteBusAttachment* bus = m_connector->GetRemoteAttachment(from, joiner);
    if(!bus)
    {
        LOG_RELEASE("Failed to find bus attachment to handle joined session!");
    }
    else
    {
        bus->AddSession(
                strtoul(localIdStr.c_str(), NULL, 10),
                joinee,
                strtoul(portStr.c_str(), NULL, 10),
                strtoul(remoteIdStr.c_str(), NULL, 10));
    }
}

void
XmppTransport::ReceiveSessionLost(
    const string& from,
    const string& message
    )
{
    FNLOG
    istringstream msgStream(message);
    string line, appName, idStr;

    // First line is the type (session joined)
    if(0 == getline(msgStream, line)){ return; }
    if(line != ALLJOYN_CODE_SESSION_LOST){ return; }

    // Get the info from the message
    if(0 == getline(msgStream, appName)){ return; }
    if(0 == getline(msgStream, idStr)){ return; }

    // Leave the local session
    RemoteBusAttachment* bus = m_connector->GetRemoteAttachment(from, appName);
    if(bus)
    {
        SessionId localId = bus->GetLocalSessionId(
                strtoul(idStr.c_str(), NULL, 10));

        LOG_DEBUG("Ending session. Attachment: %s Id: %d", appName.c_str(), localId);

        QStatus err = bus->LeaveSession(localId);
        if(err != ER_OK && err != ER_ALLJOYN_LEAVESESSION_REPLY_NO_SESSION)
        {
            LOG_RELEASE("Failed to end session: %s", QCC_StatusText(err));
        }
        else
        {
            bus->RemoveSession(localId);
        }
    }
}

void
XmppTransport::ReceiveMethodCall(
    const string& from,
    const string& message
    )
{
    FNLOG
    // Parse the required information
    istringstream msgStream(message);
    string line, remoteName, destName, destPath,
            ifaceName, memberName, remoteSessionId;

    // First line is the type (method call)
    if(0 == getline(msgStream, line)){ return; }
    if(line != ALLJOYN_CODE_METHOD_CALL){ return; }

    if(0 == getline(msgStream, remoteName)){ return; }
    if(0 == getline(msgStream, destName)){ return; }
    if(0 == getline(msgStream, destPath)){ return; }
    if(0 == getline(msgStream, ifaceName)){ return; }
    if(0 == getline(msgStream, memberName)){ return; }
    if(0 == getline(msgStream, remoteSessionId)){ return; }

    // The rest is the message arguments
    string messageArgsString = "";
    while(0 != getline(msgStream, line))
    {
        messageArgsString += line + "\n";
    }
    //util::str::UnescapeXml(messageArgsString);

    // Find the bus attachment with this busName
    RemoteBusAttachment* bus = m_connector->GetRemoteAttachment(from, remoteName);
    if(!bus)
    {
        LOG_RELEASE("No bus attachment to handle incoming method call. Message: %s",
                message.c_str());
        return;
    }

    // Call the method
    SessionId localSid = bus->GetLocalSessionId(
            strtoul(remoteSessionId.c_str(), NULL, 10));
    ProxyBusObject proxy(*bus, destName.c_str(), destPath.c_str(), localSid);
    QStatus err = proxy.IntrospectRemoteObject();
    if(err != ER_OK)
    {
        LOG_RELEASE("Failed to introspect remote object to relay method call: %s",
                QCC_StatusText(err));
        return;
    }

    vector<MsgArg> messageArgs =
            util::msgarg::VectorFromString(messageArgsString);
    Message reply(*bus);
    err = proxy.MethodCall(ifaceName.c_str(), memberName.c_str(),
            &messageArgs[0], messageArgs.size(), reply, 5000);
    if(err != ER_OK)
    {
        LOG_RELEASE("Failed to relay method call: %s", QCC_StatusText(err));
        return;
    }

    SendMethodReply(destName, destPath, reply);
}

void
XmppTransport::ReceiveMethodReply(
    const string& from,
    const string& message
    )
{
    FNLOG
    // Parse the required information
    istringstream msgStream(message);
    string line, remoteName, objPath;

    // First line is the type (method reply)
    if(0 == getline(msgStream, line)){ return; }
    if(line != ALLJOYN_CODE_METHOD_REPLY){ return; }

    if(0 == getline(msgStream, remoteName)){ return; }
    if(0 == getline(msgStream, objPath)){ return; }

    // The rest is the message arguments
    string messageArgsString = "";
    while(0 != getline(msgStream, line))
    {
        messageArgsString += line + "\n";
    }
    //util::str::UnescapeXml(messageArgsString);

    // Find the bus attachment with this busName
    RemoteBusAttachment* bus = m_connector->GetRemoteAttachment(from, remoteName);
    if(!bus)
    {
        LOG_RELEASE("No bus attachment to handle incoming method call. Message: %s",
                message.c_str());
        return;
    }

    // Tell the attachment we received a message reply
    bus->SignalReplyReceived(objPath, messageArgsString);
}

void
XmppTransport::ReceiveSignal(
    const string& from,
    const string& message
    )
{
    FNLOG
    // Parse the required information
    istringstream msgStream(message);
    string line, senderName, destination,
            remoteSessionId, objectPath, ifaceName, ifaceMember;

    // First line is the type (signal)
    if(0 == getline(msgStream, line)){ return; }
    if(line != ALLJOYN_CODE_SIGNAL){ return; }

    // Get the bus name and remote session ID
    if(0 == getline(msgStream, senderName)){ return; }
    if(0 == getline(msgStream, destination)){ return; }
    if(0 == getline(msgStream, remoteSessionId)){ return; }
    if(0 == getline(msgStream, objectPath)){ return; }
    if(0 == getline(msgStream, ifaceName)){ return; }
    if(0 == getline(msgStream, ifaceMember)){ return; }

    // The rest is the message arguments
    string messageArgsString = "";
    while(0 != getline(msgStream, line))
    {
        messageArgsString += line + "\n";
    }
    //util::str::UnescapeXml(messageArgsString);

    // Find the bus attachment with this busName
    RemoteBusAttachment* bus = m_connector->GetRemoteAttachment(from, senderName);
    if(!bus)
    {
        LOG_RELEASE("No bus attachment to handle incoming signal. Sender: %s",
                senderName.c_str());
        return;
    }

    // Relay the signal
    vector<MsgArg> msgArgs = util::msgarg::VectorFromString(messageArgsString);
    SessionId localSessionId = bus->GetLocalSessionId(
            strtoul(remoteSessionId.c_str(), NULL, 10));
    bus->RelaySignal(
            destination, localSessionId, objectPath,
            ifaceName, ifaceMember, msgArgs);
}

void
XmppTransport::ReceiveGetRequest(
    const string& from,
    const string& message
    )
{
    FNLOG
    // Parse the required information
    istringstream msgStream(message);
    string line, destName, destPath, ifaceName, propName;

    // First line is the type (get request)
    if(0 == getline(msgStream, line)){ return; }
    if(line != ALLJOYN_CODE_GET_PROPERTY){ return; }

    if(0 == getline(msgStream, destName)){ return; }
    if(0 == getline(msgStream, destPath)){ return; }
    if(0 == getline(msgStream, ifaceName)){ return; }
    if(0 == getline(msgStream, propName)){ return; }

    LOG_DEBUG("Retrieving property: Destination: %s (%s) Iface: %s Property: %s",
            destName.c_str(), destPath.c_str(), ifaceName.c_str(), propName.c_str());

    // Get the property
    ProxyBusObject proxy(m_propertyBus, destName.c_str(), destPath.c_str(), 0);
    QStatus err = proxy.IntrospectRemoteObject();
    if(err != ER_OK)
    {
        LOG_RELEASE("Failed to introspect remote object to relay get request: %s",
                QCC_StatusText(err));
        return;
    }

    MsgArg value;
    err = proxy.GetProperty(ifaceName.c_str(), propName.c_str(), value, 5000);  //cout << "Got property value:\n" << util::msgarg::ToString(value) << endl;
    if(err != ER_OK)
    {
        LOG_RELEASE("Failed to relay Get request: %s", QCC_StatusText(err));
        return;                                                                 // TODO: send the actual response status back
    }

    // Return the reply
    SendGetReply(destName, destPath, value);
}

void
XmppTransport::ReceiveGetReply(
    const string& from,
    const string& message
    )
{
    FNLOG
    // Parse the required information
    istringstream msgStream(message);
    string line, remoteName, objPath;

    // First line is the type (get reply)
    if(0 == getline(msgStream, line)){ return; }
    if(line != ALLJOYN_CODE_GET_PROP_REPLY){ return; }

    if(0 == getline(msgStream, remoteName)){ return; }
    if(0 == getline(msgStream, objPath)){ return; }

    // The rest is the property value
    string messageArgString = "";
    while(0 != getline(msgStream, line))
    {
        messageArgString += line + "\n";
    }
    //util::str::UnescapeXml(messageArgString);

    // Find the bus attachment with this busName
    RemoteBusAttachment* bus = m_connector->GetRemoteAttachment(from, remoteName);
    if(!bus)
    {
        LOG_RELEASE("No bus attachment to handle incoming Get reply.");
        return;
    }

    // Tell the attachment we received a message reply
    bus->SignalReplyReceived(objPath, messageArgString);
}

void
XmppTransport::ReceiveSetRequest(
    const string& from,
    const string& message
    )
{
    FNLOG
    // Parse the required information
    istringstream msgStream(message);
    string line, destName, destPath, ifaceName, propName;

    // First line is the type (get request)
    if(0 == getline(msgStream, line)){ return; }
    if(line != ALLJOYN_CODE_SET_PROPERTY){ return; }

    if(0 == getline(msgStream, destName)){ return; }
    if(0 == getline(msgStream, destPath)){ return; }
    if(0 == getline(msgStream, ifaceName)){ return; }
    if(0 == getline(msgStream, propName)){ return; }

    // The rest is the property value
    string messageArgString = "";
    while(0 != getline(msgStream, line))
    {
        messageArgString += line + "\n";
    }

    LOG_DEBUG("Setting property: Destination: %s (%s) Interface: %s Property: %s",
            destName.c_str(), destPath.c_str(), ifaceName.c_str(), propName.c_str());

    // Set the property
    ProxyBusObject proxy(m_propertyBus, destName.c_str(), destPath.c_str(), 0);
    QStatus err = proxy.IntrospectRemoteObject();
    if(err != ER_OK)
    {
        LOG_RELEASE("Failed to introspect remote object to relay set request: %s",
                QCC_StatusText(err));
    }

    if(err == ER_OK)
    {
        MsgArg value = util::msgarg::FromString(messageArgString);
        err = proxy.SetProperty(
                ifaceName.c_str(), propName.c_str(), value, 5000);
        if(err != ER_OK)
        {
            LOG_RELEASE("Failed to relay Set request: %s",
                    QCC_StatusText(err));
        }
    }

    // Return the reply
    SendSetReply(destName, destPath, err);
}

void
XmppTransport::ReceiveSetReply(
    const string& from,
    const string& message
    )
{
    FNLOG
    // Parse the required information
    istringstream msgStream(message);
    string line, remoteName, objPath, status;

    // First line is the type (get reply)
    if(0 == getline(msgStream, line)){ return; }
    if(line != ALLJOYN_CODE_SET_PROP_REPLY){ return; }

    if(0 == getline(msgStream, remoteName)){ return; }
    if(0 == getline(msgStream, objPath)){ return; }
    if(0 == getline(msgStream, status)){ return; }

    // Find the bus attachment with this busName
    RemoteBusAttachment* bus = m_connector->GetRemoteAttachment(from, remoteName);
    if(!bus)
    {
        LOG_RELEASE("No bus attachment to handle incoming Set reply.");
        return;
    }

    // Tell the attachment we received a message reply
    bus->SignalReplyReceived(objPath, status);
}

void
XmppTransport::ReceiveGetAllRequest(
    const string& from,
    const string& message
    )
{
    FNLOG
    // Parse the required information
    istringstream msgStream(message);
    string line, destName, destPath, ifaceName, memberName;

    // First line is the type (get request)
    if(0 == getline(msgStream, line)){ return; }
    if(line != ALLJOYN_CODE_GET_ALL){ return; }

    if(0 == getline(msgStream, destName)){ return; }
    if(0 == getline(msgStream, destPath)){ return; }
    if(0 == getline(msgStream, ifaceName)){ return; }
    if(0 == getline(msgStream, memberName)){ return; }

    LOG_DEBUG("Retrieving property: Destination: %s (%s) Interface: %s Property: %s",
            destName.c_str(), destPath.c_str(), ifaceName.c_str(), memberName.c_str());

    // Call the method
    ProxyBusObject proxy(m_propertyBus, destName.c_str(), destPath.c_str(), 0);
    QStatus err = proxy.IntrospectRemoteObject();
    if(err != ER_OK)
    {
        LOG_RELEASE("Failed to introspect remote object to relay GetAll request: %s",
                QCC_StatusText(err));
        return;
    }

    MsgArg values;
    err = proxy.GetAllProperties(ifaceName.c_str(), values, 5000);              //cout << "Got all properties:\n" << util::msgarg::ToString(values, 2) << endl;
    if(err != ER_OK)
    {
        LOG_RELEASE("Failed to get all properties: %s",
               QCC_StatusText(err));
        return;                                                                 // TODO: send the actual response status back
    }

    // Return the reply
    SendGetAllReply(destName, destPath, values);
}

void
XmppTransport::ReceiveGetAllReply(
    const string& from,
    const string& message
    )
{
    FNLOG
    // Parse the required information
    istringstream msgStream(message);
    string line, remoteName, objPath;

    // First line is the type (get reply)
    if(0 == getline(msgStream, line)){ return; }
    if(line != ALLJOYN_CODE_GET_ALL_REPLY){ return; }

    if(0 == getline(msgStream, remoteName)){ return; }
    if(0 == getline(msgStream, objPath)){ return; }

    // The rest is the property values
    string messageArgsString = "";
    while(0 != getline(msgStream, line))
    {
        messageArgsString += line + "\n";
    }
    //util::str::UnescapeXml(messageArgsString);

    // Find the bus attachment with this busName
    RemoteBusAttachment* bus = m_connector->GetRemoteAttachment(from, remoteName);
    if(!bus)
    {
        LOG_RELEASE("No bus attachment to handle incoming GetAll reply.");
        return;
    }

    // Tell the attachment we received a message reply
    bus->SignalReplyReceived(objPath, messageArgsString);
}

int
XmppTransport::XmppStanzaHandler(
    xmpp_conn_t* const   conn,
    xmpp_stanza_t* const stanza,
    void* const          userdata
    )
{
    XmppTransport* transport = static_cast<XmppTransport*>(userdata);

    // Get the stanza as text so we can parse it
    char* buf = 0;
    size_t buflen = 0;
    int result = xmpp_stanza_to_text( stanza, &buf, &buflen );
    if ( XMPP_EOK != result ) {
        LOG_RELEASE("Failed to get message/chat stanza as text! %d", result);
        return 1;
    }
    string message(buf);
    xmpp_free(xmpp_conn_get_context(conn), buf);

    // Ignore if it isn't in our roster
    // TODO: Eventually this will be a full roster, right now we support only a single destination
    const char* fromAttr = xmpp_stanza_get_attribute(stanza, "from");
    if ( transport->m_roster != fromAttr ) {
        LOG_DEBUG("Ignoring message/chat from non-roster entity: %s", fromAttr);
        return 1;
    }

    // Logging
    LOG_DEBUG("Received message/chat stanza");
    LOG_DEBUG("From: %s", fromAttr);
    LOG_VERBOSE("Stanza: %s", message.c_str());

    FNLOG
    if ( 0 == strcmp("message", xmpp_stanza_get_name(stanza)) ||
         0 == strcmp("chat", xmpp_stanza_get_name(stanza)) )
    {
        xmpp_stanza_t* body = NULL;
        if(0 != (body = xmpp_stanza_get_child_by_name(stanza, "body")) &&
           XMPP_EOK == xmpp_stanza_to_text(body, &buf, &buflen))
        {
            string message(buf);
            xmpp_free(xmpp_conn_get_context(conn), buf);

            // Strip the tags from the message
            if(0 != message.find("<body>") &&
                message.length() !=
                message.find("</body>")+strlen("</body>"))
            {
                // Received an empty message
                LOG_RELEASE("Received an empty message.")
                return 1;
            }
            message = message.substr(strlen("<body>"),
                    message.length()-strlen("<body></body>"));

            // Decompress the message
            if ( transport->m_compress )
            {
                message = util::str::Decompress(message);
            }
            else
            {
                util::str::UnescapeXml(message);
            }

            // Handle the content of the message
            string typeCode =
                    message.substr(0, message.find_first_of('\n'));
            LOG_DEBUG("Received XMPP message: %s", typeCode.c_str());

            if(typeCode == ALLJOYN_CODE_ADVERTISEMENT)
            {
                transport->ReceiveAdvertisement(fromAttr, message);
            }
            else if(typeCode == ALLJOYN_CODE_ADVERT_LOST)
            {
                transport->ReceiveAdvertisementLost(fromAttr, message);
            }
            else if(typeCode == ALLJOYN_CODE_ANNOUNCE)
            {
                transport->ReceiveAnnounce(fromAttr, message);
            }
            else if(typeCode == ALLJOYN_CODE_METHOD_CALL)
            {
                transport->ReceiveMethodCall(fromAttr, message);
            }
            else if(typeCode == ALLJOYN_CODE_METHOD_REPLY)
            {
                transport->ReceiveMethodReply(fromAttr, message);
            }
            else if(typeCode == ALLJOYN_CODE_SIGNAL)
            {
                transport->ReceiveSignal(fromAttr, message);
            }
            else if(typeCode == ALLJOYN_CODE_JOIN_REQUEST)
            {
                transport->ReceiveJoinRequest(fromAttr, message);
            }
            else if(typeCode == ALLJOYN_CODE_JOIN_RESPONSE)
            {
                transport->ReceiveJoinResponse(fromAttr, message);
            }
            else if(typeCode == ALLJOYN_CODE_SESSION_JOINED)
            {
                transport->ReceiveSessionJoined(fromAttr, message);
            }
            else if(typeCode == ALLJOYN_CODE_SESSION_LOST)
            {
                transport->ReceiveSessionLost(fromAttr, message);
            }
            else if(typeCode == ALLJOYN_CODE_GET_PROPERTY)
            {
                transport->ReceiveGetRequest(fromAttr, message);
            }
            else if(typeCode == ALLJOYN_CODE_GET_PROP_REPLY)
            {
                transport->ReceiveGetReply(fromAttr, message);
            }
            else if(typeCode == ALLJOYN_CODE_GET_ALL)
            {
                transport->ReceiveGetAllRequest(fromAttr, message);
            }
            else if(typeCode == ALLJOYN_CODE_GET_ALL_REPLY)
            {
                transport->ReceiveGetAllReply(fromAttr, message);
            }
            else
            {
                // Find and call the user-registered callback for this type
                map<string, XMPPConnector::MessageCallback>::iterator it =
                        transport->m_connector->
                        m_messageCallbackMap.find(typeCode);
                if(it != transport->m_connector->m_messageCallbackMap.end())
                {
                    size_t endOfTypePos = message.find_first_of('\n');
                    string sentMessage = ((endOfTypePos >= message.size()) ?
                            "" : message.substr(endOfTypePos+1));

                    XMPPConnector::MessageCallback callback = it->second;
                    (callback.receiver->*(callback.messageHandler))(
                            fromAttr, typeCode, sentMessage, callback.userdata);
                }
                else
                {
                    LOG_RELEASE("Received unrecognized message type: %s",
                            typeCode.c_str());
                }
            }
        }
        else
        {
            LOG_RELEASE("Could not parse body from XMPP message.");
        }
    }

    return 1;
}

int
XmppTransport::XmppPresenceHandler(
    xmpp_conn_t* const   conn,
    xmpp_stanza_t* const stanza,
    void* const          userdata
    )
{
    XmppTransport* transport = static_cast<XmppTransport*>(userdata);

    FNLOG
    if ( 0 == strcmp("presence", xmpp_stanza_get_name(stanza)) )
    {
/*        // First determine if it's a probe and answer it if necessary
        const char* type = xmpp_stanza_get_attribute(stanza, "type");
        if ( type && string(type) == "probe" ) {
            const char* from = xmpp_stanza_get_attribute(stanza, "from");
            const char* to = 
            return 1;
        }
*/
        // Get the stanza as text so we can parse it
        // NOTE: Doing this first before checking to ignore so that
        //  we output the stanza info to the log when debugging issues
        //  with presence notifications.
        char* buf = 0;
        size_t buflen = 0;
        int result = xmpp_stanza_to_text( stanza, &buf, &buflen );
        if ( XMPP_EOK != result ) {
            LOG_RELEASE("Failed to get presence stanza as text! %d", result);
            return 1;
        }
        LOG_VERBOSE("Stanza: %s", buf);
        string message(buf);
        xmpp_free(xmpp_conn_get_context(conn), buf);

        // Ignore if it isn't in our roster
        // TODO: Eventually this will be a full roster, right now we support only a single destination
        const char* fromAttr = xmpp_stanza_get_attribute(stanza, "from");
        if ( transport->m_roster != fromAttr ) {
            LOG_DEBUG("Ignoring presence from non-roster entity: %s", fromAttr);
            return 1;
        }

        // Logging
        LOG_DEBUG("Received Presence Stanza");
        LOG_DEBUG("From: %s", fromAttr);

        // Check if it's online/offline presence notification
        // Example of a presence stanza:
        //  <presence xml:lang=""
        //   to="test-connector@xmpp.chariot.global/test-connector"
        //   from="test-connector@muc.xmpp.chariot.global/test">
        //      <priority>1</priority>
        //      <c hash="sha-1" xmlns="http://jabber.org/protocol/caps"
        //       ver="71LAP/wlWGfun7j+Q4FCSSuAhQw=" node="http://pidgin.im/"/>
        //      <x xmlns="vcard-temp:x:update">
        //          <photo>677584b5b6a6f6a91cad4cfad0b13a4949d53faa</photo>
        //      </x>
        //      <x xmlns="http://jabber.org/protocol/muc#user">
        //          <item role="participant" affiliation="none"/>
        //      </x>
        //  </presence>
        const string participant_lookup("role=\"participant\""); // TODO: we should do something more robust than this
        const string moderator_lookup("role=\"moderator\""); // TODO: we should do something more robust than this
        if ( message.npos != message.find(participant_lookup) ||
            message.npos != message.find(moderator_lookup) )
        {
            // The remote entity has come online

            // Create local bus attachments and listeners to begin listening and interacting with the local AllJoyn bus
            BusAttachment* attachment = transport->m_connector->CreateBusAttachment(fromAttr);
            AllJoynListener* listener = transport->m_connector->GetBusListener(fromAttr);

            if ( !attachment || !listener )
            {
                LOG_RELEASE("Failed to create bus attachment or listener!");
                return 1;
            }

            // Start listening for advertisements
            QStatus err = attachment->FindAdvertisedName("");
            if(err != ER_OK)
            {
                LOG_RELEASE("Could not find advertised names for %s: %s",
                        fromAttr,
                        QCC_StatusText(err));
            }

            // Listen for announcements
            err = AnnouncementRegistrar::RegisterAnnounceHandler(
                    *attachment, *listener, NULL, 0);
            if(err != ER_OK)
            {
                LOG_RELEASE("Could not register Announcement handler for %s: %s",
                        fromAttr,
                        QCC_StatusText(err));
            }
        }
        else
        {
            // The remote entity has gone offline

            // Delete all the remote bus attachments
            pthread_mutex_lock(&transport->m_connector->m_remoteAttachmentsMutex);
            for ( map<string, list<RemoteBusAttachment*> >::iterator connections_it(transport->m_connector->m_remoteAttachments.begin());
                transport->m_connector->m_remoteAttachments.end() != connections_it; ++connections_it )
            {
                if ( connections_it->first == fromAttr )
                {
                    for(list<RemoteBusAttachment*>::iterator it = connections_it->second.begin();
                        it != connections_it->second.end(); ++it)
                    {
                        delete(*it);
                    }
                }
            }
            transport->m_connector->m_remoteAttachments.clear();
            pthread_mutex_unlock(&transport->m_connector->m_remoteAttachmentsMutex);
            pthread_mutex_destroy(&transport->m_connector->m_remoteAttachmentsMutex);

            // Get the local bus attachment and bus listener
            BusAttachment* attachment = transport->m_connector->GetBusAttachment(fromAttr);
            AllJoynListener* listener = transport->m_connector->GetBusListener(fromAttr);

            if ( !attachment || !listener )
            {
                // NOTE: This is okay, as we legitimately get presence notifications of unavailable
                return 1;
            }

            // Stop listening for advertisements and announcements
            AnnouncementRegistrar::UnRegisterAnnounceHandler(
                *attachment, *listener, NULL, 0);
            attachment->CancelFindAdvertisedName("");

            // Delete the local bus attachment and listener
            transport->m_connector->DeleteBusAttachment(fromAttr);
        }
    }

    return 1;
}

int
XmppTransport::XmppRosterHandler(
    xmpp_conn_t* const   conn,
    xmpp_stanza_t* const stanza,
    void* const          userdata
    )
{
    XmppTransport* transport = static_cast<XmppTransport*>(userdata);

    // TODO: Implement. Right now this is just a placeholder

    FNLOG
    if ( 0 == strcmp("iq", xmpp_stanza_get_name(stanza)) )
    {
        LOG_DEBUG("Received Roster Stanza");

        char* buf = 0;
        size_t buflen = 0;
        int result = xmpp_stanza_to_text( stanza, &buf, &buflen );
        if ( XMPP_EOK != result ) {
            LOG_RELEASE("Failed to get roster stanza as text! %d", result);
            return 1;
        }
        //LOG_VERBOSE("Stanza: %s", buf);
        string message(buf);
        xmpp_free(xmpp_conn_get_context(conn), buf);

        //string fromLocal = transport->m_chatroom+"/"+transport->m_resource;
        const char* fromAttr = xmpp_stanza_get_attribute(stanza, "from");
        LOG_VERBOSE("From: %s", fromAttr);
        const char* toAttr = xmpp_stanza_get_attribute(stanza, "to");
        LOG_VERBOSE("To: %s", toAttr);
    }

    return 1;
}

void
XmppTransport::XmppConnectionHandler(
    xmpp_conn_t* const         conn,
    const xmpp_conn_event_t    event,
    const int                  error,
    xmpp_stream_error_t* const streamError,
    void* const                userdata
    )
{
    FNLOG
    XmppTransport* transport = static_cast<XmppTransport*>(userdata);
    XmppConnectionState prevConnState = transport->m_connectionState;

    switch(event)
    {
    case XMPP_CONN_CONNECT:
    {
        transport->m_connectionState = xmpp_connected;

        // Set up our presence message
        xmpp_stanza_t* presence = xmpp_stanza_new(xmpp_conn_get_context(conn));
        xmpp_stanza_set_name(presence, "presence");
        xmpp_stanza_set_attribute(presence, "from",
                (transport->m_jabberId + "/" + transport->m_resource).c_str());

        // If a chat room was specified then build that into the
        //  presence message
        if ( !transport->m_chatroom.empty() ) {
            // Set the "to" field for this presence message to the chatroom
            xmpp_stanza_set_attribute(presence, "to",
                (transport->m_chatroom+"/"+transport->m_resource).c_str());

            // Create a child object of the presence message called 'x' to
            //  specify the XML namespace and hold the history object
            xmpp_stanza_t* x = xmpp_stanza_new(xmpp_conn_get_context(conn));
            xmpp_stanza_set_name(x, "x");
            xmpp_stanza_set_attribute(x, "xmlns", "http://jabber.org/protocol/muc");

            // Create a child object of 'x' called 'history' and set it to 0
            //  so that we don't keep the chat history
            xmpp_stanza_t* history = xmpp_stanza_new(xmpp_conn_get_context(conn));
            xmpp_stanza_set_name(history, "history");
            xmpp_stanza_set_attribute(history, "maxchars", "0");

            // Add the child XML nodes
            xmpp_stanza_add_child(x, history);
            xmpp_stanza_release(history);
            xmpp_stanza_add_child(presence, x);
            xmpp_stanza_release(x);
        }

        // Logging
        LOG_DEBUG("Sending XMPP presence message");
        char* buf = NULL;
        size_t buflen = 0;
        xmpp_stanza_to_text(presence, &buf, &buflen);
        LOG_VERBOSE("Presence Message: %s", buf);
        xmpp_free(xmpp_conn_get_context(conn), buf);

        // Send our presence message
        xmpp_send(conn, presence);
        xmpp_stanza_release(presence);
        break;
    }
    case XMPP_CONN_DISCONNECT:
    {
        LOG_RELEASE("Disconnected from XMPP server.");
        if ( xmpp_aborting == transport->m_connectionState )
        {
            LOG_RELEASE("Exiting.");

            // Stop the XMPP event loop
            xmpp_stop(xmpp_conn_get_context(conn));
        }
        else if ( prevConnState != xmpp_uninitialized )
        {
            // TODO: Try to restart the connection
            transport->m_connectionState = xmpp_disconnected;

            // TODO: For now we will quit the application because of this, but
            //  eventually we will try to restart the connection
            xmpp_stop(xmpp_conn_get_context(conn));
        }
        else
        {
            LOG_RELEASE("Login failed.");

            // Go ahead and quit the application by stopping the XMPP event loop
            xmpp_stop(xmpp_conn_get_context(conn));
        }
        break;
    }
    case XMPP_CONN_FAIL:
    default:
    {
        LOG_RELEASE("XMPP error occurred. Exiting.");

        transport->m_connectionState = xmpp_error;

        // Stop the XMPP event loop
        xmpp_stop(xmpp_conn_get_context(conn));
        break;
    }
    }

    // Call the connection callback
    if(transport->m_callbackListener && transport->m_connectionCallback)
    {
        (transport->m_callbackListener->*(transport->m_connectionCallback))(
                static_cast<Listener::XmppConnectionStatus>(event),
                transport->m_callbackUserdata);
    }
}

const string XmppTransport::ALLJOYN_CODE_ADVERTISEMENT  = "__ADVERTISEMENT";
const string XmppTransport::ALLJOYN_CODE_ADVERT_LOST    = "__ADVERT_LOST";
const string XmppTransport::ALLJOYN_CODE_ANNOUNCE       = "__ANNOUNCE";
const string XmppTransport::ALLJOYN_CODE_METHOD_CALL    = "__METHOD_CALL";
const string XmppTransport::ALLJOYN_CODE_METHOD_REPLY   = "__METHOD_REPLY";
const string XmppTransport::ALLJOYN_CODE_SIGNAL         = "__SIGNAL";
const string XmppTransport::ALLJOYN_CODE_JOIN_REQUEST   = "__JOIN_REQUEST";
const string XmppTransport::ALLJOYN_CODE_JOIN_RESPONSE  = "__JOIN_RESPONSE";
const string XmppTransport::ALLJOYN_CODE_SESSION_JOINED = "__SESSION_JOINED";
const string XmppTransport::ALLJOYN_CODE_SESSION_LOST   = "__SESSION_LOST";
const string XmppTransport::ALLJOYN_CODE_GET_PROPERTY   = "__GET_PROPERTY";
const string XmppTransport::ALLJOYN_CODE_GET_PROP_REPLY = "__GET_PROP_REPLY";
const string XmppTransport::ALLJOYN_CODE_SET_PROPERTY   = "__SET_PROPERTY";
const string XmppTransport::ALLJOYN_CODE_SET_PROP_REPLY = "__SET_PROP_REPLY";
const string XmppTransport::ALLJOYN_CODE_GET_ALL        = "__GET_ALL";
const string XmppTransport::ALLJOYN_CODE_GET_ALL_REPLY  = "__GET_ALL_REPLY";


XMPPConnector::XMPPConnector(
    BusAttachment* bus,
    const string&  appName,
    const string&  xmppJid,
    const string&  xmppPassword,
    const string&  xmppRoster,
    const string&  xmppChatroom,
    const string&  xmppResource,
    const bool     compress
    ) :
#ifndef NO_AJ_GATEWAY
    GatewayConnector(bus, appName.c_str()),
#endif // !NO_AJ_GATEWAY
    m_initialized(false),
    m_remoteAttachments()
{
    // TODO: Remove this bus, we can create a GUID some other way
    string resource(xmppResource);
    if ( resource.empty() ) {
        resource = bus->GetGlobalGUIDString().c_str();
    }
    m_xmppTransport = new XmppTransport( this,
            xmppJid, xmppPassword, xmppRoster, xmppChatroom,
            resource, compress);

    pthread_mutex_init(&m_remoteAttachmentsMutex, NULL);
}

XMPPConnector::~XMPPConnector()
{
    pthread_mutex_lock(&m_remoteAttachmentsMutex);
    for ( map<string, list<RemoteBusAttachment*> >::iterator connections_it(m_remoteAttachments.begin());
        m_remoteAttachments.end() != connections_it; ++connections_it )
    {
        for(list<RemoteBusAttachment*>::iterator it = connections_it->second.begin();
            it != connections_it->second.end(); ++it)
        {
            delete(*it);
        }
    }
    m_remoteAttachments.clear();
    pthread_mutex_unlock(&m_remoteAttachmentsMutex);
    pthread_mutex_destroy(&m_remoteAttachmentsMutex);

    // Unregister and delete all the local bus attachments and listeners
    while ( m_buses.size() > 0 )
    {
        string from(m_buses.begin()->first);
        DeleteBusAttachment(from);
    }

    delete m_xmppTransport;
}

QStatus
XMPPConnector::Init()
{
    QStatus err = ER_OK;

    if(!m_initialized)
    {
        QStatus err = GatewayConnector::init();
        if(err == ER_OK)
        {
            m_initialized = true;
        }
    }

    return err;
}

void
XMPPConnector::AddSessionPortMatch(
    const string& interfaceName,
    SessionPort   port
    )
{
    m_sessionPortMap[interfaceName].push_back(port);
}

QStatus
XMPPConnector::Start()
{
    FNLOG
    if(!m_initialized)
    {
        LOG_RELEASE("XMPPConnector not initialized");
        return ER_INIT_FAILED;
    }

    class XmppListener :
        public XmppTransport::Listener
    {
    public:
        void ConnectionCallback(
            XmppConnectionStatus status,
            void*                userdata
            )
        {
            XMPPConnector* connector = static_cast<XMPPConnector*>(userdata);

            switch(status)
            {
            case XMPP_CONNECT:
            {
                // Update connection status and get remote profiles
                QStatus err = connector->updateConnectionStatus(GW_CS_CONNECTED);
                if(err == ER_OK)
                {
                    connector->mergedAclUpdated();
                }

                break;
            }
            case XMPP_DISCONNECT:
            case XMPP_FAIL:
            default:
            {
                connector->updateConnectionStatus(GW_CS_NOT_CONNECTED);

                break;
            }
            }
        }
    } xmppListener;

    m_xmppTransport->SetConnectionCallback(
            &xmppListener,
            static_cast<XmppTransport::Listener::ConnectionCallback>(
            &XmppListener::ConnectionCallback), this);

    // Listen for XMPP messages. Blocks until transport.Stop() is called.
    QStatus err = m_xmppTransport->Run();

    m_xmppTransport->RemoveConnectionCallback();

    return err;
}

void XMPPConnector::Stop()
{
    FNLOG
    m_xmppTransport->Stop();
}

bool
XMPPConnector::OwnsWellKnownName(
    const string& name
    )
{
    bool result = false;

    pthread_mutex_lock(&m_remoteAttachmentsMutex);
    for ( map<string, list<RemoteBusAttachment*> >::const_iterator connections_it(m_remoteAttachments.begin());
        m_remoteAttachments.end() != connections_it; ++connections_it )
    {
        for(list<RemoteBusAttachment*>::const_iterator it = connections_it->second.begin(); it != connections_it->second.end(); ++it)
        {
            if(name == (*it)->WellKnownName())
            {
                result = true;
                break;
            }
        }
    }
    pthread_mutex_unlock(&m_remoteAttachmentsMutex);
    return result;
}

void
XMPPConnector::RegisterMessageHandler(
    const string&                   key,
    MessageReceiver*                receiver,
    MessageReceiver::MessageHandler messageHandler,
    void*                           userdata
    )
{
    if(!receiver || !messageHandler)
    {
        return;
    }

    MessageCallback& callback = m_messageCallbackMap[key];
    callback.receiver = receiver;
    callback.messageHandler = messageHandler;
    callback.userdata = userdata;
}

void
XMPPConnector::UnregisterMessageHandler(
    const string& key
    )
{
    m_messageCallbackMap.erase(key);
}

void
XMPPConnector::SendMessage(
    const string& key,
    const string& message
    )
{
    m_xmppTransport->SendMessage(key+"\n"+message, key);
}

#ifndef NO_AJ_GATEWAY
void
XMPPConnector::mergedAclUpdated()
{
    LOG_DEBUG("Merged Acl updated");
    GatewayMergedAcl* mergedAcl = new GatewayMergedAcl();
    QStatus status = getMergedAclAsync(mergedAcl);
    if(ER_OK != status)
    {
        delete mergedAcl;
    }
}

void
XMPPConnector::shutdown()
{
    Stop();
}

void
XMPPConnector::receiveGetMergedAclAsync(
    QStatus unmarshalStatus,
    GatewayMergedAcl* response
    )
{
    // If there is nothing to remote, disconnect from the server
    if(response->m_ExposedServices.empty() && response->m_RemotedApps.empty())
    {
        //Stop();
    }

    delete response;
}
#endif // !NO_AJ_GATEWAY

RemoteBusAttachment*
XMPPConnector::GetRemoteAttachment(
    const string&                          from,
    const string&                          remoteName,
    const vector<RemoteObjectDescription>* objects
    )
{
    FNLOG;
    RemoteBusAttachment* result = NULL;
    pthread_mutex_lock(&m_remoteAttachmentsMutex);

    map<string, list<RemoteBusAttachment*> >::iterator connection_pair(m_remoteAttachments.find(from));
    if ( m_remoteAttachments.end() != connection_pair )
    {
        list<RemoteBusAttachment*>::iterator it;
        for(it = connection_pair->second.begin(); it != connection_pair->second.end(); ++it)
        {
            if(remoteName == (*it)->RemoteName())
            {
                result = *it;
                break;
            }
        }
    }

    if(!result && objects)
    {
        LOG_DEBUG("Creating new remote bus attachment: %s", remoteName.c_str());

        // We did not find a match. Create the new attachment.
        QStatus err = ER_OK;
        map<string, vector<SessionPort> > portsToBind;
        result = new RemoteBusAttachment(remoteName, m_xmppTransport);

        vector<RemoteObjectDescription>::const_iterator objIter;
        for(objIter = objects->begin(); objIter != objects->end(); ++objIter)
        {
            string objPath = objIter->path;
            vector<const InterfaceDescription*> interfaces;

            // Get the interface descriptions
            map<string, string>::const_iterator ifaceIter;
            for(ifaceIter = objIter->interfaces.begin();
                ifaceIter != objIter->interfaces.end();
                ++ifaceIter)
            {
                string ifaceName = ifaceIter->first;
                string ifaceXml  = ifaceIter->second;

                err = result->CreateInterfacesFromXml(ifaceXml.c_str());
                if(err == ER_OK)
                {
                    const InterfaceDescription* newInterface =
                            result->GetInterface(ifaceName.c_str());
                    if(newInterface)
                    {
                        interfaces.push_back(newInterface);

                        // Any SessionPorts to bind?
                        map<string, vector<SessionPort> >::iterator spMapIter =
                                m_sessionPortMap.find(ifaceName);
                        if(spMapIter != m_sessionPortMap.end())
                        {
                            portsToBind[ifaceName] = spMapIter->second;
                        }
                    }
                    else
                    {
                        err = ER_FAIL;
                    }
                }

                if(err != ER_OK)
                {
                    LOG_RELEASE("Failed to create InterfaceDescription %s: %s",
                            ifaceName.c_str(), QCC_StatusText(err));
                    break;
                }
            }
            if(err != ER_OK)
            {
                break;
            }

            // Add the bus object.
            err = result->AddRemoteObject(objPath, interfaces);
            if(err != ER_OK)
            {
                LOG_RELEASE("Failed to add remote object %s: %s", objPath.c_str(),
                        QCC_StatusText(err));
                break;
            }
        }

        // Start and connect the new attachment.
        if(err == ER_OK)
        {
            err = result->Start();
            if(err != ER_OK)
            {
                LOG_RELEASE("Failed to start new bus attachment %s: %s",
                        remoteName.c_str(), QCC_StatusText(err));
            }
        }
        if(err == ER_OK)
        {
            err = result->Connect();
            if(err != ER_OK)
            {
                LOG_RELEASE("Failed to connect new bus attachment %s: %s",
                        remoteName.c_str(), QCC_StatusText(err));
            }
        }

        // Bind any necessary SessionPorts
        if(err == ER_OK)
        {
            map<string, vector<SessionPort> >::iterator spMapIter;
            for(spMapIter = portsToBind.begin();
                spMapIter != portsToBind.end();
                ++spMapIter)
            {
                vector<SessionPort>::iterator spIter;
                for(spIter = spMapIter->second.begin();
                    spIter != spMapIter->second.end();
                    ++spIter)
                {
                    LOG_DEBUG("Binding session port %d for interface %s",
                            *spIter, spMapIter->first.c_str());
                    err = result->BindSessionPort(*spIter);
                    if(err != ER_OK)
                    {
                        LOG_RELEASE("Failed to bind session port: %s",
                                QCC_StatusText(err));
                        break;
                    }
                }

                if(err != ER_OK)
                {
                    break;
                }
            }
        }

        if(err == ER_OK)
        {
            m_remoteAttachments[from].push_back(result);
        }
        else
        {
            delete result;
            result = NULL;
        }
    }

    pthread_mutex_unlock(&m_remoteAttachmentsMutex);
    return result;
}

RemoteBusAttachment*
XMPPConnector::GetRemoteAttachmentByAdvertisedName(
    const string& from,
    const string& advertisedName
    )
{
    RemoteBusAttachment* result = NULL;

    pthread_mutex_lock(&m_remoteAttachmentsMutex);
    map<string, list<RemoteBusAttachment*> >::iterator connection_pair(m_remoteAttachments.find(from));
    if ( m_remoteAttachments.end() == connection_pair )
    {
        pthread_mutex_unlock(&m_remoteAttachmentsMutex);
        return NULL;
    }
    list<RemoteBusAttachment*>::iterator it;
    for(it = connection_pair->second.begin(); it != connection_pair->second.end(); ++it)
    {
        if(advertisedName == (*it)->WellKnownName())
        {
            result = *it;
            break;
        }
    }
    pthread_mutex_unlock(&m_remoteAttachmentsMutex);

    return result;
}

void XMPPConnector::DeleteRemoteAttachment(
    const string&         from,
    RemoteBusAttachment*& attachment
    )
{
    if(!attachment)
    {
        return;
    }

    string name = attachment->RemoteName();

    attachment->Disconnect();
    attachment->Stop();
    attachment->Join();

    pthread_mutex_lock(&m_remoteAttachmentsMutex);
    map<string, list<RemoteBusAttachment*> >::iterator connection_pair(m_remoteAttachments.find(from));
    if ( m_remoteAttachments.end() == connection_pair )
    {
        pthread_mutex_unlock(&m_remoteAttachmentsMutex);
        return;
    }
    connection_pair->second.remove(attachment);
    pthread_mutex_unlock(&m_remoteAttachmentsMutex);

    delete(attachment);
    attachment = NULL;

    LOG_DEBUG("Deleted remote bus attachment: %s", name.c_str());
}

BusAttachment* XMPPConnector::CreateBusAttachment(
    const std::string& from
    )
{
    FNLOG;
    BusAttachment* attachment = 0;

    pthread_mutex_lock(&m_remoteAttachmentsMutex);
    map<string, BusAttachment*>::iterator connection_pair(m_buses.find(from));
    if ( m_buses.end() == connection_pair )
    {
        LOG_VERBOSE("Creating bus attachment and bus listener for %s", from.c_str());

        // Create the BusAttachment and related BusListener and register for advertisements and announcements
        attachment = new BusAttachment(from.c_str(), true);

        QStatus status = attachment->Start();
        if (ER_OK != status) {
            LOG_RELEASE("Error starting bus for %s: %s", from.c_str(), QCC_StatusText(status));
            pthread_mutex_unlock(&m_remoteAttachmentsMutex);
            delete attachment;
            return 0;
        }

        status = attachment->Connect();
        if (ER_OK != status) {
            LOG_RELEASE("Error connecting bus for %s: %s", from.c_str(), QCC_StatusText(status));
            pthread_mutex_unlock(&m_remoteAttachmentsMutex);
            delete attachment;
            return 0;
        }

        AllJoynListener* listener = new AllJoynListener(this, m_xmppTransport, attachment);
        attachment->RegisterBusListener(*listener);

        m_buses[from] = attachment;
        m_listeners[from] = listener;
    }
    else
    {
        attachment = connection_pair->second;
    }
    pthread_mutex_unlock(&m_remoteAttachmentsMutex);

    return attachment;
}

BusAttachment* XMPPConnector::GetBusAttachment(
    const std::string& from
    )
{
    FNLOG;
    BusAttachment* attachment = 0;

    pthread_mutex_lock(&m_remoteAttachmentsMutex);
    map<string, BusAttachment*>::iterator connection_pair(m_buses.find(from));
    if ( m_buses.end() == connection_pair )
    {
        LOG_VERBOSE("Asked for a BusAttachment for %s but it doesn't exist!", from.c_str());
    }
    else
    {
        attachment = connection_pair->second;
    }
    pthread_mutex_unlock(&m_remoteAttachmentsMutex);

    return attachment;
}

void XMPPConnector::DeleteBusAttachment(
    const std::string& from
    )
{
    FNLOG;
    AllJoynListener* listener = GetBusListener(from);
    pthread_mutex_lock(&m_remoteAttachmentsMutex);
    map<string, BusAttachment*>::iterator connection_pair(m_buses.find(from));
    if ( m_buses.end() == connection_pair )
    {
        pthread_mutex_unlock(&m_remoteAttachmentsMutex);
        return;
    }
    BusAttachment* attachment = connection_pair->second;
    attachment->UnregisterBusListener(*listener);
    // TODO: Unregister for advertisements and announcements
    DeleteBusListener(from);
    attachment->Stop();
    delete attachment;
    m_buses.erase(from);
    pthread_mutex_unlock(&m_remoteAttachmentsMutex);
}

AllJoynListener* XMPPConnector::GetBusListener(
    const std::string& from
    )
{
    FNLOG;
    AllJoynListener* listener = 0;

    pthread_mutex_lock(&m_remoteAttachmentsMutex);
    map<string, AllJoynListener*>::iterator connection_pair(m_listeners.find(from));
    if ( m_listeners.end() == connection_pair )
    {
        // It should already exist!
        LOG_VERBOSE("Asked for a BusListener for %s but it doesn't exist!", from.c_str());
    }
    else
    {
        listener = connection_pair->second;
    }
    pthread_mutex_unlock(&m_remoteAttachmentsMutex);

    return listener;
}

void XMPPConnector::DeleteBusListener(
    const std::string& from
    )
{
    //pthread_mutex_lock(&m_remoteAttachmentsMutex);
    map<string, AllJoynListener*>::iterator connection_pair(m_listeners.find(from));
    if ( m_listeners.end() == connection_pair )
    {
        pthread_mutex_unlock(&m_remoteAttachmentsMutex);
        return;
    }
    delete connection_pair->second;
    m_listeners.erase(from);
    //pthread_mutex_unlock(&m_remoteAttachmentsMutex);
}
