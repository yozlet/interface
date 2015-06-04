//
//  LoginDialog.cpp
//  interface/src/ui
//
//  Created by Bradley Austin Davis on 2015/04/14
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "LoginDialog.h"

#include <NetworkingConstants.h>

#include "AccountManager.h"
#include "DependencyManager.h"
#include "Menu.h"

HIFI_QML_DEF(LoginDialog)

LoginDialog::LoginDialog(QQuickItem *parent) : OffscreenQmlDialog(parent),
    _dialogFormat("rectangular"),
    _rootUrl(NetworkingConstants::METAVERSE_SERVER_URL.toString())
{
    connect(&AccountManager::getInstance(), &AccountManager::loginComplete,
        this, &LoginDialog::handleLoginCompleted);
    connect(&AccountManager::getInstance(), &AccountManager::loginFailed,
        this, &LoginDialog::handleLoginFailed);

    connect(DependencyManager::get<DialogsManager>().data(), &DialogsManager::loginDialogFormatChanged,
        this, &LoginDialog::updateDialogFormat);
}

void LoginDialog::toggleAction() {
    AccountManager& accountManager = AccountManager::getInstance();
    QAction* loginAction = Menu::getInstance()->getActionForOption(MenuOption::Login);
    Q_CHECK_PTR(loginAction);
    disconnect(loginAction, 0, 0, 0);

    if (accountManager.isLoggedIn()) {
        // change the menu item to logout
        loginAction->setText("Logout " + accountManager.getAccountInfo().getUsername());
        connect(loginAction, &QAction::triggered, &accountManager, &AccountManager::logout);
    } else {
        // change the menu item to login
        loginAction->setText("Login");
        connect(loginAction, &QAction::triggered, [] {
            LoginDialog::show();
        });
    }
}

void LoginDialog::handleLoginCompleted(const QUrl&) {
    hide();
}

void LoginDialog::handleLoginFailed() {
    setStatusText("<font color = \"#267077\">Invalid username or password.< / font>");
}

void LoginDialog::setDialogFormat(const QString& dialogFormat) {
    if (dialogFormat != _dialogFormat) {
        _dialogFormat = dialogFormat;
        emit dialogFormatChanged();
    }
}

QString LoginDialog::dialogFormat() const {
    return _dialogFormat;
}

void LoginDialog::updateDialogFormat() {
    if (Menu::getInstance()->isOptionChecked(MenuOption::LoginDialogCircular)) {
        setDialogFormat("circular");
    } else {
        setDialogFormat("rectangular");
    }
}

void LoginDialog::setStatusText(const QString& statusText) {
    if (statusText != _statusText) {
        _statusText = statusText;
        emit statusTextChanged();
    }
}

QString LoginDialog::statusText() const {
    return _statusText;
}

QString LoginDialog::rootUrl() const {
    return _rootUrl;
}

void LoginDialog::login(const QString& username, const QString& password) {
    qDebug() << "Attempting to login " << username;
    setStatusText("Authenticating...");
    AccountManager::getInstance().requestAccessToken(username, password);
}

void LoginDialog::openUrl(const QString& url) {
    qDebug() << url;
}
