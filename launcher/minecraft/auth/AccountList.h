#pragma once

#include "MinecraftAccount.h"

#include <QObject>
#include <QVariant>
#include <QAbstractListModel>
#include <QSharedPointer>

class AccountList : public QAbstractListModel
{
    Q_OBJECT
public:
    enum ModelRoles
    {
        PointerRole = Qt::UserRole,
        ProfileNameRole,
        AccountStatusRole,
    };

    struct Entry
    {
        bool isAccount;
        MinecraftAccountPtr account;
    };

    explicit AccountList(QObject *parent = 0);
    virtual ~AccountList() noexcept;

    const AccountList::Entry& at(int i) const;
    int count() const;

    QVariant data(const QModelIndex &index, int role) const override;
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    QModelIndex addAccount(const MinecraftAccountPtr account);
    void removeAccount(const QString& accountId);

    bool getAccountByProfileName(const QString& profileName, MinecraftAccountPtr& pointer, int& index) const;
    bool getAccountById(const QString& internalId, MinecraftAccountPtr& pointer, int& index) const;

    QStringList profileNames() const;

    void setListFilePath(QString path, bool autosave = false);

    bool loadList();
    bool loadV3(QJsonObject &root);
    bool saveList();

    MinecraftAccountPtr defaultAccount() const;
    QModelIndex defaultAccountIndex() const;
    void setDefaultAccount(MinecraftAccountPtr profileId);
    bool anyAccountIsValid();

signals:
    void listChanged();
    void accountChanged(MinecraftAccount *account);
    void defaultAccountChanged();

public slots:
    void onAccountChanged();

protected:
    void onListChanged();
    void onDefaultAccountChanged();

    QList<Entry> m_accounts;
    MinecraftAccountPtr m_defaultAccount;
    QString m_listFilePath;
    bool m_autosave = false;
};
