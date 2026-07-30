#include "MinecraftAccount.h"

#include <QUuid>
#include <QCryptographicHash>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegExp>
#include <QStringList>
#include <QJsonDocument>

#include <QDebug>

#include <Application.h>

MinecraftAccount::MinecraftAccount(QObject* parent) : QObject(parent) {
    data.internalId = QUuid::createUuid().toString().remove(QRegExp("[{}-]"));
}

MinecraftAccountPtr MinecraftAccount::loadFromJsonV3(const QJsonObject& json) {
    MinecraftAccountPtr account(new MinecraftAccount());
    if(account->data.resumeStateFromV3(json)) {
        return account;
    }
    return nullptr;
}

// Generate UUID v3 matching Java's UUID.nameUUIDFromBytes("OfflinePlayer:" + username)
static QString offlineUUID(const QString& username)
{
    QByteArray data = QString("OfflinePlayer:" + username).toUtf8();
    QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Md5);
    hash[6] = (hash[6] & 0x0f) | 0x30;
    hash[8] = (hash[8] & 0x3f) | 0x80;
    return QString::fromLatin1(hash.toHex().insert(8, '-').insert(13, '-').insert(18, '-').insert(23, '-'));
}

MinecraftAccountPtr MinecraftAccount::createOffline(const QString& username)
{
    MinecraftAccountPtr account(new MinecraftAccount());
    account->data.type = "offline";
    account->data.minecraftProfile.name = username;
    account->data.minecraftProfile.id = offlineUUID(username);
    account->data.accountState = AccountState::Offline;
    return account;
}

QJsonObject MinecraftAccount::saveToJson() const
{
    return data.saveState();
}

AccountState MinecraftAccount::accountState() const {
    return data.accountState;
}

QString MinecraftAccount::accountStateText() const
{
    switch(data.accountState)
    {
        case AccountState::Unchecked: {
            return tr("Unchecked", "Account status");
        }
        case AccountState::Offline: {
            return tr("Offline", "Account status");
        }
        case AccountState::Online: {
            return tr("Online", "Account status");
        }
        case AccountState::Working: {
            return tr("Working", "Account status");
        }
        case AccountState::Errored: {
            return tr("Errored", "Account status");
        }
        case AccountState::Expired: {
            return tr("Expired", "Account status");
        }
        case AccountState::Gone: {
            return tr("Gone", "Account status");
        }
        case AccountState::MustMigrate: {
            return tr("Must Migrate", "Account status");
        }
        default: {
            return tr("Unknown", "Account status");
        }
    }
}

bool MinecraftAccount::isActive() const {
    return false;
}

void MinecraftAccount::fillSession(AuthSessionPtr session)
{
    session->status = AuthSession::PlayableOffline;
    session->access_token = QString();
    session->player_name = data.profileName();
    session->uuid = data.profileId();
    session->user_type = typeString();
    session->session = "-";
}

void MinecraftAccount::decrementUses()
{
    Usable::decrementUses();
    if(!isInUse())
    {
        emit changed();
    }
}

void MinecraftAccount::incrementUses()
{
    bool wasInUse = isInUse();
    Usable::incrementUses();
    if(!wasInUse)
    {
        emit changed();
    }
}
