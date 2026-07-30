#pragma once
#include <QString>
#include <QByteArray>
#include <QJsonObject>

struct MinecraftProfile {
    QString id;
    QString name;
};

enum class AccountState {
    Unchecked,
    Offline,
    Working,
    Online,
    Errored,
    Expired,
    Gone,
    MustMigrate
};

struct AccountData {
    QJsonObject saveState() const;
    bool resumeStateFromV3(QJsonObject data);

    QString profileId() const;
    QString profileName() const;
    QString lastError() const;

    MinecraftProfile minecraftProfile;

    QString type = "offline";

    // runtime only information (not saved with the account)
    QString internalId;
    QString errorString;
    AccountState accountState = AccountState::Offline;
};
