#include "AccountData.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QUuid>

bool AccountData::resumeStateFromV3(QJsonObject data) {
    auto typeV = data.value("type");
    if(!typeV.isString()) {
        qWarning() << "Failed to parse account data: type is missing.";
        return false;
    }
    auto typeS = typeV.toString();
    if(typeS == "offline") {
        type = "offline";
        auto nameV = data.value("profileName");
        auto idV = data.value("profileId");
        if(!nameV.isString()) {
            qWarning() << "Offline account: profileName is missing.";
            return false;
        }
        minecraftProfile.name = nameV.toString();
        minecraftProfile.id = idV.isString() ? idV.toString() : QString();
        accountState = AccountState::Offline;
        return true;
    }
    qWarning() << "Failed to parse account data: type is not recognized.";
    return false;
}

QJsonObject AccountData::saveState() const {
    QJsonObject output;
    output["type"] = "offline";
    output["profileName"] = minecraftProfile.name;
    output["profileId"] = minecraftProfile.id;
    return output;
}

QString AccountData::profileId() const {
    return minecraftProfile.id;
}

QString AccountData::profileName() const {
    return minecraftProfile.name;
}

QString AccountData::lastError() const {
    return errorString;
}
