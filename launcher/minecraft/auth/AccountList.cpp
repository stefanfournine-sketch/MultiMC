#include "AccountList.h"
#include "AccountData.h"
#include "AccountTask.h"

#include <QIODevice>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDir>

#include <QDebug>

#include <Application.h>
#include <FileSystem.h>
#include <QSaveFile>

#include <Application.h>

enum AccountListVersion {
    MojangOnly = 2,
    MojangMSA = 3
};

AccountList::AccountList(QObject *parent) : QAbstractListModel(parent) {
}

AccountList::~AccountList() noexcept {}

bool AccountList::getAccountByProfileName(const QString& profileName, MinecraftAccountPtr& pointer, int& index) const {
    for (int i = 0; i < count(); i++) {
        auto entry = at(i);
        if(!entry.isAccount)
        {
            continue;
        }
        if (entry.account->profileName() == profileName) {
            pointer = entry.account;
            index = i;
            return true;
        }
    }
    pointer = nullptr;
    index = -1;
    return false;
}

bool AccountList::getAccountById(const QString& internalId, MinecraftAccountPtr& pointer, int& index) const {
    for (int i = 0; i < count(); i++) {
        auto entry = at(i);
        if(!entry.isAccount)
        {
            continue;
        }
        if (entry.account->internalId() == internalId) {
            pointer = entry.account;
            index = i;
            return true;
        }
    }
    pointer = nullptr;
    index = -1;
    return false;
}

const AccountList::Entry& AccountList::at(int i) const
{
    return m_accounts.at(i);
}

QStringList AccountList::profileNames() const {
    QStringList out;
    for(auto & entry: m_accounts) {
        if(!entry.isAccount)
        {
            continue;
        }
        auto profileName =  entry.account->profileName();
        if(profileName.isEmpty())
        {
            continue;
        }
        out.append(profileName);
    }
    return out;
}

QModelIndex AccountList::addAccount(const MinecraftAccountPtr account)
{
    int i = 0;
    for(const auto& entry: m_accounts)
    {
        if(entry.account == account)
        {
            return index(i);
        }
    }

    connect(account.get(), &MinecraftAccount::changed, this, &AccountList::onAccountChanged);

    int row = m_accounts.count();
    beginInsertRows(QModelIndex(), row, row);
    m_accounts.append(Entry{true, account});
    endInsertRows();
    onListChanged();
    return index(row);
}

void AccountList::removeAccount(const QString& internalId)
{
    int row;
    MinecraftAccountPtr account;
    if(!getAccountById(internalId, account, row))
    {
        return;
    }

    if(account == m_defaultAccount)
    {
        m_defaultAccount = nullptr;
        onDefaultAccountChanged();
    }
    account->disconnect(this);

    beginRemoveRows(QModelIndex(), row, row);
    m_accounts.removeAt(row);
    endRemoveRows();
    onListChanged();
}

QModelIndex AccountList::defaultAccountIndex() const
{
    if(!m_defaultAccount)
    {
        return QModelIndex();
    }

    for (int i = 0; i < count(); i++) {
        auto entry = at(i);
        if(!entry.isAccount)
        {
            continue;
        }
        if (entry.account == m_defaultAccount) {
            return index(i);
        }
    }
    return QModelIndex();
}

MinecraftAccountPtr AccountList::defaultAccount() const
{
    return m_defaultAccount;
}

void AccountList::setDefaultAccount(MinecraftAccountPtr newAccount)
{
    if (!newAccount && m_defaultAccount)
    {
        int idx = 0;
        auto previousDefaultAccount = m_defaultAccount;
        m_defaultAccount = nullptr;
        for (auto& entry : m_accounts)
        {
            if(entry.isAccount && entry.account == previousDefaultAccount)
            {
                emit dataChanged(index(idx), index(idx));
            }
            idx ++;
        }
        onDefaultAccountChanged();
    }
    else
    {
        auto currentDefaultAccount = m_defaultAccount;
        int currentDefaultAccountIdx = -1;
        auto newDefaultAccount = m_defaultAccount;
        int newDefaultAccountIdx = -1;
        int idx = 0;
        for (auto& entry : m_accounts)
        {
            if(!entry.isAccount)
            {
                continue;
            }
            if (entry.account == newAccount)
            {
                newDefaultAccount = entry.account;
                newDefaultAccountIdx = idx;
            }
            if(currentDefaultAccount == entry.account)
            {
                currentDefaultAccountIdx = idx;
            }
            idx++;
        }
        if(currentDefaultAccount != newDefaultAccount)
        {
            emit dataChanged(index(currentDefaultAccountIdx), index(currentDefaultAccountIdx));
            emit dataChanged(index(newDefaultAccountIdx), index(newDefaultAccountIdx));
            m_defaultAccount = newDefaultAccount;
            onDefaultAccountChanged();
        }
    }
}

void AccountList::onAccountChanged()
{
    MinecraftAccount *account = qobject_cast<MinecraftAccount *>(sender());
    bool found = false;
    for (int i = 0; i < count(); i++) {
        auto entry = at(i);
        if(!entry.isAccount)
            continue;
        if (entry.account.get() == account) {
            emit dataChanged(index(i),  index(i));
            found = true;
            break;
        }
    }
    if(found)
    {
        emit accountChanged(account);
        onListChanged();
    }
}

void AccountList::onListChanged()
{
    if (m_autosave)
        saveList();

    emit listChanged();
}

void AccountList::onDefaultAccountChanged()
{
    if (m_autosave)
        saveList();

    emit defaultAccountChanged();
}

int AccountList::count() const
{
    return m_accounts.count();
}

QVariant AccountList::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() > count())
        return QVariant();

    auto entry = at(index.row());
    if(!entry.isAccount)
    {
        switch (role)
        {
            case Qt::DisplayRole:
                return tr("Add New Account");
            case PointerRole:
                return QVariant::fromValue(MinecraftAccountPtr());
        }
        return QVariant();
    }
    auto account = entry.account;

    switch (role)
    {
        case Qt::DisplayRole:
        {
            return account->profileName() + "\n" + account->accountStateText();
        }
        case ProfileNameRole:
            return account->profileName();
        case AccountStatusRole:
            return account->accountStateText();
        case PointerRole:
            return QVariant::fromValue(account);
        case Qt::CheckStateRole:
            return account == m_defaultAccount ? Qt::Checked : Qt::Unchecked;
        default:
            return QVariant();
    }
}

int AccountList::rowCount(const QModelIndex &) const
{
    return count();
}

Qt::ItemFlags AccountList::flags(const QModelIndex &index) const
{
    if (index.row() < 0 || index.row() >= rowCount(index) || !index.isValid())
    {
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool AccountList::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if (idx.row() < 0 || idx.row() >= rowCount(idx) || !idx.isValid())
    {
        return false;
    }
    auto entry = at(idx.row());
    if(!entry.isAccount)
    {
        return false;
    }

    if(role == Qt::CheckStateRole)
    {
        if(value == Qt::Checked)
        {
            setDefaultAccount(entry.account);
        }
        else if(value == Qt::Unchecked)
        {
            setDefaultAccount(nullptr);
        }
    }

    emit dataChanged(idx, idx);
    return true;
}

bool AccountList::loadList()
{
    if (m_listFilePath.isEmpty())
    {
        qCritical() << "Can't load account list. No file path given and no default set.";
        return false;
    }

    m_accounts.append(Entry{false, nullptr});

    QFile file(m_listFilePath);

    if (!file.open(QIODevice::ReadOnly))
    {
        qCritical() << QString("Failed to read the account list file (%1).").arg(m_listFilePath).toUtf8();
        return false;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError)
    {
        qCritical() << QString("Failed to parse account list file: %1 at offset %2")
                            .arg(parseError.errorString(), QString::number(parseError.offset))
                            .toUtf8();
        return false;
    }

    if (!jsonDoc.isObject())
    {
        qCritical() << "Invalid account list JSON: Root should be an array.";
        return false;
    }

    QJsonObject root = jsonDoc.object();

    auto listVersion = root.value("formatVersion").toVariant().toInt();
    switch(listVersion) {
        case AccountListVersion::MojangMSA: {
            return loadV3(root);
        }
        break;
        default: {
            QString newName = "accounts-old.json";
            qWarning() << "Unknown format version when loading account list. Existing one will be renamed to" << newName;
            file.rename(newName);
            return false;
        }
    }
}

bool AccountList::loadV3(QJsonObject& root) {
    beginResetModel();
    QJsonArray accounts = root.value("accounts").toArray();
    for (QJsonValue accountVal : accounts)
    {
        QJsonObject accountObj = accountVal.toObject();
        MinecraftAccountPtr account = MinecraftAccount::loadFromJsonV3(accountObj);
        if (account.get() != nullptr)
        {
            connect(account.get(), &MinecraftAccount::changed, this, &AccountList::onAccountChanged);
            m_accounts.append(Entry{true, account});
            if(accountObj.value("active").toBool(false)) {
                m_defaultAccount = account;
            }
        }
        else
        {
            qWarning() << "Failed to load an account.";
        }
    }
    endResetModel();
    return true;
}

bool AccountList::saveList()
{
    if (m_listFilePath.isEmpty())
    {
        qCritical() << "Can't save account list. No file path given and no default set.";
        return false;
    }

    if(!FS::ensureFilePathExists(m_listFilePath))
        return false;

    QFileInfo finfo(m_listFilePath);
    if(finfo.isDir())
    {
        QDir badDir(m_listFilePath);
        badDir.removeRecursively();
    }

    qDebug() << "Writing account list to" << m_listFilePath;

    QJsonObject root;
    root.insert("formatVersion", AccountListVersion::MojangMSA);

    QJsonArray accounts;
    for (auto& entry : m_accounts)
    {
        if(!entry.isAccount)
        {
            continue;
        }
        QJsonObject accountObj = entry.account->saveToJson();
        if(m_defaultAccount == entry.account) {
            accountObj["active"] = true;
        }
        accounts.append(accountObj);
    }

    root.insert("accounts", accounts);

    QJsonDocument doc(root);

    QSaveFile file(m_listFilePath);

    if (!file.open(QIODevice::WriteOnly))
    {
        qCritical() << QString("Failed to read the account list file (%1).").arg(m_listFilePath).toUtf8();
        return false;
    }

    file.write(doc.toJson());
    file.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ReadUser|QFile::WriteUser);
    if(file.commit()) {
        qDebug() << "Saved account list to" << m_listFilePath;
        return true;
    }
    else {
        qDebug() << "Failed to save accounts to" << m_listFilePath;
        return false;
    }
}

void AccountList::setListFilePath(QString path, bool autosave)
{
    m_listFilePath = path;
    m_autosave = autosave;
}

bool AccountList::anyAccountIsValid()
{
    for(auto& entry: m_accounts)
    {
        if(entry.account) {
            return true;
        }
    }
    return false;
}
