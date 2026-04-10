#pragma once
#include <QDialog>
#include <QButtonGroup>
#include <QTabWidget>
#include <QLineEdit>
#include <QMap>
#include <QList>
#include "StockDataProvider.h"

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(const QList<StockDataProvider*> &providers,
                            const QString &activeProviderId,
                            QWidget *parent = nullptr);

    QString selectedProviderId() const;
    // Returns credentials for every provider keyed by provider id
    QMap<QString, QMap<QString,QString>> allCredentials() const;

private:
    QButtonGroup *m_providerGroup;
    QTabWidget   *m_tabs;
    // provider id -> (field key -> line edit)
    QMap<QString, QMap<QString, QLineEdit*>> m_fields;
    QList<StockDataProvider*> m_providers;
};
