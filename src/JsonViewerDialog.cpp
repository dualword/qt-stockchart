#include "JsonViewerDialog.h"
#include <QVBoxLayout>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QFont>
#include <QApplication>
#include <QClipboard>
#include <QJsonDocument>
#include <QLabel>

JsonViewerDialog::JsonViewerDialog(const QString &title, const QByteArray &data,
                                   QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(title);
    resize(720, 540);

    auto *layout = new QVBoxLayout(this);

    // Size info
    auto *sizeLabel = new QLabel(
        QString("Response size: %1 bytes").arg(data.size()), this);
    QFont sf = sizeLabel->font();
    sf.setPointSize(sf.pointSize() - 1);
    sizeLabel->setFont(sf);
    layout->addWidget(sizeLabel);

    auto *textEdit = new QTextEdit(this);
    textEdit->setReadOnly(true);
    QFont mono;
    mono.setFamilies({"Menlo", "Courier New", "Monospace"});
    mono.setPointSize(11);
    textEdit->setFont(mono);

    // Try to pretty-print as JSON; fall back to raw UTF-8 text (e.g. HTML from YahooPage)
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isNull())
        textEdit->setPlainText(doc.toJson(QJsonDocument::Indented));
    else
        textEdit->setPlainText(QString::fromUtf8(data));

    layout->addWidget(textEdit);

    auto *btns = new QDialogButtonBox(QDialogButtonBox::Close, this);
    auto *copyBtn = btns->addButton("Copy", QDialogButtonBox::ActionRole);
    connect(copyBtn, &QPushButton::clicked, this, [textEdit]() {
        QApplication::clipboard()->setText(textEdit->toPlainText());
    });
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::accept);
    layout->addWidget(btns);
}
