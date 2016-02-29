#include <QDataStream>
#include <QDebug>

#include "o2baseauth.h"
#include "o2globals.h"
#include "o2settingsstore.h"

#define trace() if (1) qDebug()
// define trace() if (0) qDebug()

static const quint16 DefaultLocalPort = 1965;

O2BaseAuth::O2BaseAuth(QObject *parent): QObject(parent) {
    localPort_ = DefaultLocalPort;
    store_ = new O2SettingsStore(O2_ENCRYPTION_KEY, this);
}

void O2BaseAuth::setStore(O2AbstractStore *store) {
    if (store_) {
        store_->deleteLater();
    }
    if (store) {
        store_ = store;
        store_->setParent(this);
    } else {
        store_ = new O2SettingsStore(O2_ENCRYPTION_KEY, this);
        return;
    }
}

bool O2BaseAuth::linked() {
    QString key = QString(O2_KEY_LINKED).arg(clientId_);
    bool result = !store_->value(key).isEmpty();
    trace() << "O2BaseAuth::linked:" << (result? "Yes": "No");
    return result;
}

void O2BaseAuth::setLinked(bool v) {
    trace() << "O2BaseAuth::setLinked:" << (v? "true": "false");
    bool oldValue = linked();
    QString key = QString(O2_KEY_LINKED).arg(clientId_);
    store_->setValue(key, v? "1": "");
    if (oldValue != v) {
        emit linkedChanged();
    }
}

QString O2BaseAuth::tokenSecret() {
    QString key = QString(O2_KEY_TOKEN_SECRET).arg(clientId_);
    return store_->value(key);
}

void O2BaseAuth::setTokenSecret(const QString &v) {
    QString key = QString(O2_KEY_TOKEN_SECRET).arg(clientId_);
    store_->setValue(key, v);
    emit tokenSecretChanged();
}

QString O2BaseAuth::token() {
    QString key = QString(O2_KEY_TOKEN).arg(clientId_);
    return store_->value(key);
}

void O2BaseAuth::setToken(const QString &v) {
    QString key = QString(O2_KEY_TOKEN).arg(clientId_);
    store_->setValue(key, v);
    emit tokenChanged();
}

QString O2BaseAuth::clientId() {
    return clientId_;
}

void O2BaseAuth::setClientId(const QString &value) {
    clientId_ = value;
    emit clientIdChanged();
}

QString O2BaseAuth::clientSecret() {
    return clientSecret_;
}

void O2BaseAuth::setClientSecret(const QString &value) {
    clientSecret_ = value;
    emit clientSecretChanged();
}

int O2BaseAuth::localPort() {
    return localPort_;
}

void O2BaseAuth::setLocalPort(int value) {
    trace() << "O2BaseAuth::setLocalPort:" << value;
    localPort_ = value;
    emit localPortChanged();
}

QVariantMap O2BaseAuth::extraTokens() {
    QString key = QString(O2_KEY_EXTRA_TOKENS).arg(clientId_);
    QString value = store_->value(key);
    QByteArray bytes = QByteArray::fromBase64(value.toLatin1());
    QDataStream stream(&bytes, QIODevice::ReadOnly);
    stream >> extraTokens_;
    return extraTokens_;
}

void O2BaseAuth::setExtraTokens(QVariantMap extraTokens) {
    extraTokens_ = extraTokens;
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream << extraTokens;
    QString key = QString(O2_KEY_EXTRA_TOKENS).arg(clientId_);
    store_->setValue(key, bytes.toBase64());
    emit extraTokensChanged();
}

QByteArray O2BaseAuth::createQueryParameters(const QList<O2RequestParameter> &parameters) {
    QByteArray ret;
    bool first = true;
    foreach (O2RequestParameter h, parameters) {
        if (first) {
            first = false;
        } else {
            ret.append("&");
        }
        ret.append(QUrl::toPercentEncoding(h.name) + "=" + QUrl::toPercentEncoding(h.value));
    }
    return ret;
}