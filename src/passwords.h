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

#ifndef PASSWORDS_H
#define PASSWORDS_H

#include <qdialog.h>
#include <qsortfilterproxymodel.h>

#include "ui_passwords.h"
#include "ui_passworddetails.h"

// passwords dialog (not a "login" dialog)
class PasswordsDialog : public QDialog, public Ui_PasswordsDialog
{
    Q_OBJECT

public:
    PasswordsDialog(QWidget *parent = 0);

private:
    QAbstractTableModel *m_model;
    QSortFilterProxyModel *m_proxyModel;

private slots:
    void showDetails(void);
    void selectionChanged(void);
};

// password details dialog
class PasswordDetailsDialog : public QDialog, public Ui_PasswordDetailsDialog
{
    Q_OBJECT

public:
    PasswordDetailsDialog(QString uri, QString account, QWidget *parent = 0);

};

// submited form data to remember - pairs of "input name" => "value"
// there are two pairs for for http auth: "username" and "password"
typedef QMap<QString, QString> PasswordFormData;

class PasswordStorageModel;

// interface common to all password storages
class PasswordStorage
{
public:

    virtual QStringList getUriList(void) const = 0;
    virtual QStringList getAccounts(const QString uri) const = 0;
    virtual PasswordFormData getAccountData(const QString uri, const QString account) const = 0;

    virtual bool storeAccountData(const QString uri, const QString account, const PasswordFormData data) = 0;
    virtual bool removeAccount(const QString uri, const QString account) = 0;

    virtual PasswordStorageModel *createModel(QObject *parent);
};

// model for Passwords dialog
class PasswordStorageModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    PasswordStorageModel(PasswordStorage *ps, QObject *parent = 0);

    virtual int rowCount (const QModelIndex & parent = QModelIndex()) const;
    int columnCount (const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data (const QModelIndex & index, int role = Qt::DisplayRole) const;
    bool removeRows (int begin, int count, const QModelIndex & parent = QModelIndex());
    QVariant headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

private:
    PasswordStorage *passwordStorage;
    QList< QVector<QString> > list;
};

// dummy password storage -- for debuging only
// Yes, you are looking for this.
class PasswordStorageDummy : public PasswordStorage
{
    QStringList getUriList(void) const;
    QStringList getAccounts(const QString uri) const;
    PasswordFormData getAccountData(const QString uri, const QString account) const;

    bool storeAccountData(const QString uri, const QString account, const PasswordFormData data);
    bool removeAccount(const QString uri, const QString account);
};

// password manager initializes password storage and handle interaction with all other parts of arora
class PasswordsManager : public QObject
{
    Q_OBJECT

public:
    PasswordsManager(QObject *parent = 0);
    ~PasswordsManager();

public slots:
    QStringList getAccounts(const QString uri) const;
    PasswordFormData getAccountData(const QString uri, const QString account) const;
    bool storeAccountData(const QString uri, const QString account, const PasswordFormData data);
    bool storeAccountPassword(const QString uri, const QString account, const QString password);
    bool removeAccount(const QString uri, const QString account);
    PasswordStorageModel *createStorageModel(QObject *parent);

private:
    PasswordStorage *storage;

};

// model for http auth dialog
class PasswordsManagerAccountsModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    PasswordsManagerAccountsModel(PasswordsManager *pm, QString uri);
    int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

private:
    PasswordsManager *passwordsManager;
    QString uri;
    QStringList accounts;
};

#endif

