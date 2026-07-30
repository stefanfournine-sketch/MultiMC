#pragma once

#include <QString>
#include <memory>
#include "QObjectPtr.h"

class MinecraftAccount;

struct AuthSession
{
    QString serializeUserProperties();

    enum Status
    {
        Undetermined,
        PlayableOffline
    } status = PlayableOffline;

    // client token
    QString client_token;
    // combined session ID
    QString session;
    // volatile auth token
    QString access_token;
    // profile name
    QString player_name;
    // profile ID
    QString uuid;
    // 'legacy' or 'mojang', depending on account type
    QString user_type;
    // Did the auth server reply?
    bool auth_server_online = false;

    //Is this a demo session?
    bool demo = false;
};

typedef std::shared_ptr<AuthSession> AuthSessionPtr;
