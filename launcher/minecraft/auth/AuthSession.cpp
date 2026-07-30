#include "AuthSession.h"
#include <QJsonObject>
#include <QJsonDocument>

QString AuthSession::serializeUserProperties()
{
    QJsonObject userAttrs;
    QJsonDocument value(userAttrs);
    return value.toJson(QJsonDocument::Compact);
}
