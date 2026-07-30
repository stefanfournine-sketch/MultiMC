#pragma once

#include <tasks/Task.h>
#include <QString>
#include <QJsonObject>

#include "MinecraftAccount.h"

enum class AccountTaskState
{
    STATE_CREATED,
    STATE_WORKING,
    STATE_SUCCEEDED,
    STATE_FAILED_SOFT,
    STATE_FAILED_HARD,
};

class AccountTask : public Task
{
    Q_OBJECT
public:
    explicit AccountTask(AccountData * data, QObject *parent = 0);
    virtual ~AccountTask() {};

    AccountTaskState m_taskState = AccountTaskState::STATE_CREATED;

    AccountTaskState taskState() {
        return m_taskState;
    }

protected:
    virtual QString getStateMessage() const;

protected slots:
    bool changeState(AccountTaskState newState, QString reason = QString());

protected:
    AccountData *m_data = nullptr;
};
