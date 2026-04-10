#include "StockDataProvider.h"

StockDataProvider::StockDataProvider(QObject *parent) : QObject(parent) {}

bool StockDataProvider::hasCredentials() const
{
    for (const auto &field : credentialFields()) {
        if (m_credentials.value(field.first).trimmed().isEmpty())
            return false;
    }
    return true;
}

int StockDataProvider::rangeToDays(const QString &range)
{
    if (range == "1mo") return 30;
    if (range == "6mo") return 180;
    if (range == "1y")  return 365;
    return 90; // default: 3mo
}
