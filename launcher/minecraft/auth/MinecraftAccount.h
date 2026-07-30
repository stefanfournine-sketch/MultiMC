#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include <QJsonObject>
#include <QPair>
#include <QMap>
#include <QPixmap>

#include <memory>

#include "AuthSession.h"
#include "Usable.h"
#include "AccountData.h"
#include "QObjectPtr.h"

class Task;
class AccountTask;
class MinecraftAccount;

typedef shared_qobject_ptr<MinecraftAccount> MinecraftAccountPtr;
Q_DECLARE_METATYPE(MinecraftAccountPtr)

class MinecraftAccount :
    public QObject,
    public Usable
{
    Q_OBJECT
public:
    explicit MinecraftAccount(const MinecraftAccount &other, QObject *parent) = delete;
    explicit MinecraftAccount(QObject *parent = 0);

    static MinecraftAccountPtr createOffline(const QString& username);
    static MinecraftAccountPtr loadFromJsonV3(const QJsonObject &json);
    QJsonObject saveToJson() const;

public:
    QString internalId() const {
        return data.internalId;
    }

    QString profileId() const {
        return data.profileId();
    }

    QString profileName() const {
        return data.profileName();
    }

    bool isActive() const;

    bool ownsMinecraft() const {
        return true;
    }

    bool hasProfile() const {
        return data.profileId().size() != 0;
    }

    QString typeString() const {
        return "offline";
    }

    AccountState accountState() const;
    QString accountStateText() const;

    AccountData * accountData() {
        return &data;
    }

    void fillSession(AuthSessionPtr session);

    QString lastError() const {
        return data.lastError();
    }

signals:
    void changed();
    void activityChanged(bool active);

protected:
    AccountData data;

    void incrementUses() override;
    void decrementUses() override;
};
