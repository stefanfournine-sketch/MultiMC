#pragma once

#include <QDialog>
#include "minecraft/auth/AccountList.h"

#include <QTimer>
#include <QUrl>
#include <QString>
#include <QDialog>
#include <QIcon>

class QAbstractButton;
class QMenu;
class QEvent;
class QAction;

namespace Ui
{
class AccountsDialog;
}

class AccountsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AccountsDialog(QWidget *parent = 0, const QString& internalId = QString());
    virtual ~AccountsDialog();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onAddOfflineButtonClicked();
    void onAccountChanged(MinecraftAccount * account);

private:
    void updateStates();

private slots:
    void onSignOutButtonClicked(bool);

private:
    void changeEvent(QEvent * event) override;

private:
    Ui::AccountsDialog *ui;
    class QStatusBar* m_statusBar = nullptr;
    shared_qobject_ptr<AccountList> m_accounts;

    MinecraftAccountPtr m_currentAccount;
};
