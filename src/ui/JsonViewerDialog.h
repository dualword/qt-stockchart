#pragma once
#include <QDialog>
#include <QByteArray>
#include <QString>

// Modal dialog that displays a raw JSON (or HTML) response in a readable, formatted view.
// Attempts to pretty-print as JSON; falls back to plain UTF-8 text.
class JsonViewerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit JsonViewerDialog(const QString &title, const QByteArray &data,
                              QWidget *parent = nullptr);
};
