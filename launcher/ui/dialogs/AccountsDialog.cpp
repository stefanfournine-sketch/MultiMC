#include <QAction>
#include <QStatusBar>

#include "AccountsDialog.h"
#include "ui_AccountsDialog.h"

#include "Application.h"
#include "BuildConfig.h"

#include <QMessageBox>
#include <QClipboard>

constexpr auto selectionFlags = QItemSelectionModel::Clear | QItemSelectionModel::Select | QItemSelectionModel::Rows | QItemSelectionModel::Current;

AccountsDialog::AccountsDialog(QWidget *parent, const QString& internalId) : QDialog(parent), ui(new Ui::AccountsDialog)
{
    ui->setupUi(this);
    ui->windowLayout->setWindowFlags(Qt::Widget);
    m_statusBar = ui->windowLayout->statusBar();

    auto icon = APPLICATION->getThemedIcon("accounts");
    if(icon.isNull())
    {
        icon = APPLICATION->getThemedIcon("noaccount");
    }
    m_accounts = APPLICATION->accounts();
    ui->accountListView->setModel(m_accounts.get());
    ui->accountListView->setIconSize(QSize(48, 48));
    setWindowIcon(icon);
    setWindowTitle(tr("Minecraft Accounts"));

    connect(ui->signOutButton, &QPushButton::clicked, [&](bool on) {
        onSignOutButtonClicked(on);
    });

    connect(ui->addOfflineButton, &QPushButton::clicked, this, &AccountsDialog::onAddOfflineButtonClicked);

    QItemSelectionModel *selectionModel = ui->accountListView->selectionModel();
    bool foundAccount = false;
    if(!internalId.isEmpty())
    {
        MinecraftAccountPtr account;
        int row;
        if(m_accounts->getAccountById(internalId, account, row))
        {
            selectionModel->select(m_accounts->index(row), selectionFlags);
            foundAccount = true;
        }
    }
    if(!foundAccount)
    {
        if(m_accounts->count() == 1)
        {
            selectionModel->select(m_accounts->index(0), selectionFlags);
        }
        else
        {
            if(m_accounts->defaultAccount())
            {
                selectionModel->select(m_accounts->defaultAccountIndex(), selectionFlags);
            }
            else
            {
                selectionModel->select(m_accounts->index(1), selectionFlags);
            }
        }
    }
    updateStates();

    connect(selectionModel, &QItemSelectionModel::selectionChanged, [this](const QItemSelection &sel, const QItemSelection &dsel) {
        updateStates();
    });
    connect(m_accounts.get(), &AccountList::accountChanged, this, &AccountsDialog::onAccountChanged);

    restoreGeometry(QByteArray::fromBase64(APPLICATION->settings()->get("AccountsDialogGeometry").toByteArray()));
    ui->splitter->restoreState(QByteArray::fromBase64(APPLICATION->settings()->get("AccountsDialogSplitterState").toByteArray()));
}

AccountsDialog::~AccountsDialog()
{
    delete ui;
}

void AccountsDialog::closeEvent(QCloseEvent* event)
{
    APPLICATION->settings()->set("AccountsDialogSplitterState", ui->splitter->saveState().toBase64());
    APPLICATION->settings()->set("AccountsDialogGeometry", saveGeometry().toBase64());
    QDialog::closeEvent(event);
}

void AccountsDialog::onAccountChanged(MinecraftAccount* account)
{
    if(m_currentAccount.get() == account)
    {
        updateStates();
    }
}

void AccountsDialog::updateStates()
{
    QModelIndexList selection = ui->accountListView->selectionModel()->selectedIndexes();
    bool hasSelection = selection.size() > 0;
    auto prevAccount = m_currentAccount;
    m_currentAccount = nullptr;
    if (hasSelection)
    {
        QModelIndex selected = selection.first();
        m_currentAccount = selected.data(AccountList::PointerRole).value<MinecraftAccountPtr>();
    }

    if(!m_currentAccount)
    {
        ui->accountPageStack->setCurrentWidget(ui->loginPage);
        return;
    }

    ui->accountPageStack->setCurrentWidget(ui->fullAccountPage);
    ui->selectedAccountLabel->setText(m_currentAccount->profileName());
}

void AccountsDialog::onAddOfflineButtonClicked()
{
    QString name = ui->offlineNameEdit->text().trimmed();
    if(name.isEmpty()) {
        return;
    }
    QRegExp valid("[a-zA-Z0-9_]{3,16}");
    if(!valid.exactMatch(name)) {
        QMessageBox::warning(this, tr("Invalid username"), tr("Username must be 3-16 characters and contain only letters, numbers, and underscores."));
        return;
    }
    auto account = MinecraftAccount::createOffline(name);
    QModelIndex idx = m_accounts->addAccount(account);
    if(!m_accounts->defaultAccount()) {
        m_accounts->setData(idx, Qt::Checked, Qt::CheckStateRole);
    }
    ui->offlineNameEdit->clear();
    ui->accountListView->selectionModel()->select(idx,
        QItemSelectionModel::Clear | QItemSelectionModel::Select | QItemSelectionModel::Rows | QItemSelectionModel::Current);
}

void AccountsDialog::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QDialog::changeEvent(event);
}

void AccountsDialog::onSignOutButtonClicked(bool)
{
    if(m_currentAccount)
    {
        m_accounts->removeAccount(m_currentAccount->internalId());
    }
}
