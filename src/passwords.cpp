/*
 * Copyright 2008 Josef Kufner <jk@myserver.cz>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#include <qheaderview.h>

#include "passwords.h"
#include "browserapplication.h"


/****************************************************************************
 *      Passwords Dialog
 */

PasswordsDialog::PasswordsDialog(QWidget *parent)
        : QDialog(parent)
{
    PasswordsManager *passwordsManager = BrowserApplication::instance()->passwordsManager();

    setupUi(this);
    setWindowFlags(Qt::Sheet);
    
    m_model = passwordsManager->createStorageModel(this);
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    table->verticalHeader()->hide();
    table->setModel(m_proxyModel);
    table->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

    connect(detailsButton, SIGNAL(clicked()), this, SLOT(showDetails()));
    connect(removeButton, SIGNAL(clicked()), table, SLOT(removeSelected()));
    connect(table, SIGNAL(activated(const QModelIndex&)), this, SLOT(showDetails()));
    connect(search, SIGNAL(textChanged(QString)),
            m_proxyModel, SLOT(setFilterFixedString(QString)));
    connect(table->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
                this, SLOT(selectionChanged()));

    selectionChanged();
}

void PasswordsDialog::showDetails(void)
{
    int row = table->currentIndex().row();

    QString uri     = table->model()->index(row, 0).data().toString();
    QString account = table->model()->index(row, 1).data().toString();

    PasswordDetailsDialog d(uri, account, this);
    d.exec();
}

void PasswordsDialog::selectionChanged(void)
{
    bool e = table->selectionModel()->selection().isEmpty();

    removeButton->setEnabled(!e);
    detailsButton->setEnabled(!e);
}

/****************************************************************************
 *      Passwords Details Dialog
 */

PasswordDetailsDialog::PasswordDetailsDialog(QString uri, QString account, QWidget *parent)
    : QDialog(parent)
{
    PasswordsManager *passwordsManager = BrowserApplication::instance()->passwordsManager();

    setupUi(this);

    dataTable->setColumnCount(2);
    dataTable->setHorizontalHeaderLabels(QStringList() << tr("Field") << tr("Value"));
    dataTable->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    dataTable->horizontalHeader()->setStretchLastSection(true);

    uriLabel->setText(uri);
    accountLabel->setText(account);

    PasswordFormData data = passwordsManager->getAccountData(uri, account);
    dataTable->setRowCount(data.count());
    dataTable->setColumnCount(2);
    dataTable->verticalHeader()->hide();

    int row = 0;
    for (PasswordFormData::const_iterator i = data.constBegin(); i != data.constEnd(); i++) {
        QTableWidgetItem *item;

        item = new QTableWidgetItem(i.key());
        dataTable->setItem(row, 0, item);

        item = new QTableWidgetItem(i.value());
        dataTable->setItem(row, 1, item);

        row++;
    }
}


/****************************************************************************
 *      Passwords Storage
 */

PasswordStorageModel *PasswordStorage::createModel(QObject *parent)
{
    return new PasswordStorageModel(this, parent);
}

/****************************************************************************
 *      Password storage model
 */

PasswordStorageModel::PasswordStorageModel(PasswordStorage *ps, QObject *parent)
    : QAbstractTableModel(parent)
{
    Q_ASSERT(ps != 0);

    passwordStorage = ps;

    // fixme: it this really necessary ?
    foreach(QString uri, ps->getUriList()) {
        foreach(QString account, ps->getAccounts(uri)) {
            QVector<QString> d(2);
            d[0] = uri;
            d[1] = account;
            list.append(d);
        }
    }
}

int PasswordStorageModel::rowCount (const QModelIndex & ) const
{
    return list.count();
}

int PasswordStorageModel::columnCount (const QModelIndex & ) const
{
    return 2;
}

QVariant PasswordStorageModel::data (const QModelIndex & index, int role) const
{
    switch(role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            return list.at(index.row()).at(index.column());
        default:
            return QVariant();
    }
}

bool PasswordStorageModel::removeRows (int begin, int count, const QModelIndex & parent)
{
    beginRemoveRows(parent, begin, begin + count - 1);

    for (int row = begin; row < begin + count; row++) {
        if (passwordStorage->removeAccount(list.at(row).at(0), list.at(row).at(1))) {
            list.removeAt(row);
        }
    }

    endRemoveRows();
    return true;
}

QVariant PasswordStorageModel::headerData (int section, Qt::Orientation orientation, int role) const
{
    switch(role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            if (orientation == Qt::Horizontal) {
                switch (section) {
                    case 0: return QString(tr("URI"));
                    case 1: return QString(tr("Account"));
                    default: return 0;
                }
            }
    }

    return QVariant();
}


/****************************************************************************
 *      Dummy Passwords Storage
 */

QStringList PasswordStorageDummy::getUriList(void) const
{
    qDebug("getUriList");
    QStringList list;
    list.append(QLatin1String("http://localhost/"));
    list.append(QLatin1String("http://phpmyadmin/"));
    return list;
}

QStringList PasswordStorageDummy::getAccounts(const QString uri) const
{
    qDebug(qPrintable(QString(QLatin1String("getAccounts: uri = \"%1\"")).arg(uri)));
    QStringList sl;
    sl.append(QLatin1String("root"));
    sl.append(uri);
    sl.append(QLatin1String("hello world"));
    sl.append(QLatin1String("nobody"));
    return sl;
}

PasswordFormData PasswordStorageDummy::getAccountData(const QString uri, const QString account) const
{
    qDebug(qPrintable(QString(QLatin1String("getAccountData: uri = \"%1\", account = \"%2\"")).arg(uri).arg(account)));
    PasswordFormData d;
    d[QLatin1String("username")] = account;
    d[QLatin1String("password")] = account;
    d[QLatin1String("submit")] = QLatin1String("Login");
    return d;
}

bool PasswordStorageDummy::storeAccountData(const QString uri, const QString account, const PasswordFormData data)
{
    (void) data;
    qDebug(qPrintable(QString(QLatin1String("storeAccountData: uri = \"%1\", account = \"%2\"")).arg(uri).arg(account)));
    return true;
}

bool PasswordStorageDummy::removeAccount(const QString uri, const QString account)
{
    qDebug(qPrintable(QString(QLatin1String("removeAccount: uri = \"%1\", account = \"%2\"")).arg(uri).arg(account)));
    return true;
}


/****************************************************************************
 *      Password Manager
 */

PasswordsManager::PasswordsManager(QObject *parent)
    : QObject(parent)
{
    storage = new PasswordStorageDummy();
}

PasswordsManager::~PasswordsManager()
{
    if (storage)
        delete storage;
}

QStringList PasswordsManager::getAccounts(const QString uri) const
{
    if (storage) {
        return storage->getAccounts(uri);
    } else {
        return QStringList();
    }
}

PasswordFormData PasswordsManager::getAccountData(const QString uri, const QString account) const
{
    if (storage) {
        return storage->getAccountData(uri, account);
    } else {
        return PasswordFormData();
    }
}

// usable for any html forms
bool PasswordsManager::storeAccountData(const QString uri, const QString account, const PasswordFormData data)
{
    if (storage) {
        return storage->storeAccountData(uri, account, data);
    } else {
        return false;
    }
}

// usable for http auth
bool PasswordsManager::storeAccountPassword(const QString uri, const QString account, const QString password)
{
    PasswordFormData data;
    data[QLatin1String("username")] = account;
    data[QLatin1String("password")] = password; 

    return storeAccountData(uri, account, data);
}

bool PasswordsManager::removeAccount(const QString uri, const QString account)
{
    if (storage) {
        return storage->removeAccount(uri, account);
    } else {
        return false;
    }
}

PasswordStorageModel *PasswordsManager::createStorageModel(QObject *parent)
{
    if (storage) {
        return storage->createModel(parent);
    } else {
        return false;
    }
}


/****************************************************************************
 *      PasswordsManagerAccountsModel 
 */

PasswordsManagerAccountsModel::PasswordsManagerAccountsModel(PasswordsManager *pm, QString uri)
{
    Q_ASSERT(pm != 0);
    passwordsManager = pm;
    this->uri = uri;
    accounts = pm->getAccounts(uri);
}

int PasswordsManagerAccountsModel::rowCount(const QModelIndex &) const
{
    return accounts.count();
}

int PasswordsManagerAccountsModel::columnCount(const QModelIndex &) const
{
    return 2;
}

QVariant PasswordsManagerAccountsModel::data(const QModelIndex & index, int role) const
{
    switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch(index.column()) {
        case 0:
            return accounts.at(index.row());
        case 1:
            return passwordsManager->getAccountData(uri, accounts.at(index.row())).value(QLatin1String("password"));
        default:
            return 0;
        }
        break;
    default:
        return QVariant();
    }
}

QVariant PasswordsManagerAccountsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QAbstractTableModel::headerData(section, orientation, role);
}

