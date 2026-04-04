#include "AdBlockDialog.h"
#include "RequestInterceptor.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPushButton>
#include <QTimer>
#include <QHeaderView>

AdBlockDialog::AdBlockDialog(RequestInterceptor *interceptor, QWidget *parent)
    : QDialog(parent, Qt::Tool)
    , m_interceptor(interceptor)
{
    setWindowTitle("Domain Filter / Ad Blocker");
    setMinimumSize(600, 420);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(6);

    // ── Column headers ────────────────────────────────────────────────────────
    auto *headerRow = new QHBoxLayout();
    auto *leftHeader = new QLabel("<b>Active Domains</b>  (this page load)");
    auto *rightHeader = new QLabel("<b>Blocked Domains</b>");
    headerRow->addWidget(leftHeader, 1);
    headerRow->addSpacing(52); // aligns with centre buttons
    headerRow->addWidget(rightHeader, 1);
    mainLayout->addLayout(headerRow);

    // ── Lists + buttons ───────────────────────────────────────────────────────
    auto *listsRow = new QHBoxLayout();
    listsRow->setSpacing(6);

    // Left: active domains
    m_activeList = new QListWidget(this);
    m_activeList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_activeList->setSortingEnabled(true);
    listsRow->addWidget(m_activeList, 1);

    // Centre: + / - buttons
    auto *btnCol = new QVBoxLayout();
    btnCol->setSpacing(6);
    btnCol->addStretch();

    m_addBtn = new QPushButton("+", this);
    m_addBtn->setFixedWidth(36);
    m_addBtn->setToolTip("Block selected active domains");
    btnCol->addWidget(m_addBtn);

    m_removeBtn = new QPushButton("\xe2\x88\x92", this); // − (minus sign)
    m_removeBtn->setFixedWidth(36);
    m_removeBtn->setToolTip("Unblock selected domains");
    btnCol->addWidget(m_removeBtn);

    btnCol->addStretch();
    listsRow->addLayout(btnCol);

    // Right: blacklist (domain + hit count)
    m_blackList = new QTreeWidget(this);
    m_blackList->setColumnCount(2);
    m_blackList->setHeaderLabels({ "Domain", "Hits" });
    m_blackList->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_blackList->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    m_blackList->header()->resizeSection(1, 50);
    m_blackList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_blackList->setRootIsDecorated(false);
    m_blackList->setSortingEnabled(true);
    m_blackList->sortByColumn(0, Qt::AscendingOrder);
    listsRow->addWidget(m_blackList, 1);

    mainLayout->addLayout(listsRow, 1);

    // ── Hint label ───────────────────────────────────────────────────────────
    auto *hint = new QLabel(
        "Select domains in the left list and press <b>+</b> to block them.  "
        "Select rows in the right list and press <b>−</b> to unblock them.", this);
    hint->setWordWrap(true);
    mainLayout->addWidget(hint);

    // ── Connections ──────────────────────────────────────────────────────────
    connect(m_addBtn,    &QPushButton::clicked, this, &AdBlockDialog::onAdd);
    connect(m_removeBtn, &QPushButton::clicked, this, &AdBlockDialog::onRemove);

    m_refreshTimer = new QTimer(this);
    connect(m_refreshTimer, &QTimer::timeout, this, &AdBlockDialog::refreshLists);
    m_refreshTimer->start(500);

    refreshLists();
}

// ── Slots ──────────────────────────────────────────────────────────────────────

void AdBlockDialog::onAdd()
{
    const auto selected = m_activeList->selectedItems();
    if (selected.isEmpty()) return;

    QStringList domains;
    domains.reserve(selected.size());
    for (const auto *item : selected)
        domains << item->text();

    m_interceptor->addDomains(domains);
    refreshBlacklist();
}

void AdBlockDialog::onRemove()
{
    const auto selected = m_blackList->selectedItems();
    if (selected.isEmpty()) return;

    QStringList domains;
    domains.reserve(selected.size());
    for (const auto *item : selected)
        domains << item->text(0);

    m_interceptor->removeDomains(domains);
    refreshBlacklist();
}

void AdBlockDialog::refreshLists()
{
    refreshActiveList();
    refreshBlacklist();
}

// ── Private helpers ────────────────────────────────────────────────────────────

void AdBlockDialog::refreshActiveList()
{
    const QStringList current = m_interceptor->accessedDomains();

    // If interceptor tracking was cleared (new page load), reset the list.
    if (current.isEmpty()) {
        m_activeList->clear();
        return;
    }

    // Incrementally add new domains to avoid disrupting selection.
    QSet<QString> existing;
    existing.reserve(m_activeList->count());
    for (int i = 0; i < m_activeList->count(); ++i)
        existing.insert(m_activeList->item(i)->text());

    for (const QString &d : current)
        if (!existing.contains(d))
            m_activeList->addItem(d);
}

void AdBlockDialog::refreshBlacklist()
{
    const QSet<QString>    blacklist = m_interceptor->blacklist();
    const QMap<QString,int> hits     = m_interceptor->blockedHits();

    // Build map of existing items for fast lookup.
    QMap<QString, QTreeWidgetItem *> existing;
    for (int i = 0; i < m_blackList->topLevelItemCount(); ++i) {
        QTreeWidgetItem *it = m_blackList->topLevelItem(i);
        existing[it->text(0)] = it;
    }

    // Add new / update existing.
    for (const QString &domain : blacklist) {
        const int h = hits.value(domain, 0);
        const QString hStr = h > 0 ? QString::number(h) : QString();

        if (existing.contains(domain)) {
            existing[domain]->setText(1, hStr);
            existing.remove(domain);
        } else {
            auto *item = new QTreeWidgetItem({ domain, hStr });
            item->setTextAlignment(1, Qt::AlignRight | Qt::AlignVCenter);
            m_blackList->addTopLevelItem(item);
        }
    }

    // Remove entries that were deleted from the blacklist.
    for (QTreeWidgetItem *stale : std::as_const(existing))
        delete stale;
}
