#include "AccountTask.h"
#include "MinecraftAccount.h"

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>

#include <QDebug>

AccountTask::AccountTask(AccountData *data, QObject *parent)
    : Task(parent), m_data(data)
{
    changeState(AccountTaskState::STATE_CREATED);
}

QString AccountTask::getStateMessage() const
{
    switch (m_taskState)
    {
    case AccountTaskState::STATE_CREATED:
        return "Waiting...";
    case AccountTaskState::STATE_WORKING:
        return "Working...";
    case AccountTaskState::STATE_SUCCEEDED:
        return "Task succeeded.";
    case AccountTaskState::STATE_FAILED_SOFT:
        return "Encountered an error.";
    case AccountTaskState::STATE_FAILED_HARD:
        return "Task failed.";
    default:
        return "...";
    }
}

bool AccountTask::changeState(AccountTaskState newState, QString reason)
{
    m_taskState = newState;
    setStatus(getStateMessage());
    switch(newState) {
        case AccountTaskState::STATE_CREATED: {
            m_data->errorString.clear();
            return true;
        }
        case AccountTaskState::STATE_WORKING: {
            m_data->accountState = AccountState::Working;
            return true;
        }
        case AccountTaskState::STATE_SUCCEEDED: {
            m_data->accountState = AccountState::Online;
            emitSucceeded();
            return false;
        }
        case AccountTaskState::STATE_FAILED_SOFT: {
            m_data->errorString = reason;
            m_data->accountState = AccountState::Errored;
            emitFailed(reason);
            return false;
        }
        case AccountTaskState::STATE_FAILED_HARD: {
            m_data->errorString = reason;
            m_data->accountState = AccountState::Expired;
            emitFailed(reason);
            return false;
        }
        default: {
            QString error = QString("Unknown account task state: %1").arg(int(newState));
            m_data->accountState = AccountState::Errored;
            emitFailed(error);
            return false;
        }
    }
}
