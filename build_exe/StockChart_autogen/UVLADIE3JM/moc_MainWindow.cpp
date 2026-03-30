/****************************************************************************
** Meta object code from reading C++ file 'MainWindow.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/MainWindow.h"
#include <QtGui/qtextcursor.h>
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MainWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN10MainWindowE_t {};
} // unnamed namespace

template <> constexpr inline auto MainWindow::qt_create_metaobjectdata<qt_meta_tag_ZN10MainWindowE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "MainWindow",
        "onStockSelectionChanged",
        "",
        "onDataReady",
        "symbol",
        "QList<StockDataPoint>",
        "data",
        "onError",
        "message",
        "onSymbolTypeReady",
        "SymbolType",
        "type",
        "openSettings",
        "onAddGroupClicked",
        "onTreeContextMenu",
        "QPoint",
        "pos",
        "onToggleTable",
        "onToggleDisplayMode",
        "checked",
        "configurePeriods",
        "onTableColumnClicked",
        "col",
        "onChartRangeChanged",
        "days",
        "showHelp",
        "exportGroups",
        "importGroups",
        "onYScaleChanged",
        "index",
        "onEditStockDetails",
        "QTreeWidgetItem*",
        "item",
        "onSetStar",
        "starIndex"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'onStockSelectionChanged'
        QtMocHelpers::SlotData<void()>(1, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onDataReady'
        QtMocHelpers::SlotData<void(const QString &, const QVector<StockDataPoint> &)>(3, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 4 }, { 0x80000000 | 5, 6 },
        }}),
        // Slot 'onError'
        QtMocHelpers::SlotData<void(const QString &, const QString &)>(7, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 4 }, { QMetaType::QString, 8 },
        }}),
        // Slot 'onSymbolTypeReady'
        QtMocHelpers::SlotData<void(const QString &, SymbolType)>(9, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 4 }, { 0x80000000 | 10, 11 },
        }}),
        // Slot 'openSettings'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onAddGroupClicked'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onTreeContextMenu'
        QtMocHelpers::SlotData<void(const QPoint &)>(14, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 15, 16 },
        }}),
        // Slot 'onToggleTable'
        QtMocHelpers::SlotData<void()>(17, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onToggleDisplayMode'
        QtMocHelpers::SlotData<void(bool)>(18, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 19 },
        }}),
        // Slot 'configurePeriods'
        QtMocHelpers::SlotData<void()>(20, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onTableColumnClicked'
        QtMocHelpers::SlotData<void(int)>(21, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 22 },
        }}),
        // Slot 'onChartRangeChanged'
        QtMocHelpers::SlotData<void(int)>(23, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 24 },
        }}),
        // Slot 'showHelp'
        QtMocHelpers::SlotData<void()>(25, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'exportGroups'
        QtMocHelpers::SlotData<void()>(26, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'importGroups'
        QtMocHelpers::SlotData<void()>(27, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onYScaleChanged'
        QtMocHelpers::SlotData<void(int)>(28, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 29 },
        }}),
        // Slot 'onEditStockDetails'
        QtMocHelpers::SlotData<void(QTreeWidgetItem *)>(30, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 31, 32 },
        }}),
        // Slot 'onSetStar'
        QtMocHelpers::SlotData<void(QTreeWidgetItem *, int)>(33, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 31, 32 }, { QMetaType::Int, 34 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MainWindow, qt_meta_tag_ZN10MainWindowE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10MainWindowE_t>.metaTypes,
    nullptr
} };

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MainWindow *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->onStockSelectionChanged(); break;
        case 1: _t->onDataReady((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QList<StockDataPoint>>>(_a[2]))); break;
        case 2: _t->onError((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 3: _t->onSymbolTypeReady((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<SymbolType>>(_a[2]))); break;
        case 4: _t->openSettings(); break;
        case 5: _t->onAddGroupClicked(); break;
        case 6: _t->onTreeContextMenu((*reinterpret_cast<std::add_pointer_t<QPoint>>(_a[1]))); break;
        case 7: _t->onToggleTable(); break;
        case 8: _t->onToggleDisplayMode((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 9: _t->configurePeriods(); break;
        case 10: _t->onTableColumnClicked((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 11: _t->onChartRangeChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 12: _t->showHelp(); break;
        case 13: _t->exportGroups(); break;
        case 14: _t->importGroups(); break;
        case 15: _t->onYScaleChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 16: _t->onEditStockDetails((*reinterpret_cast<std::add_pointer_t<QTreeWidgetItem*>>(_a[1]))); break;
        case 17: _t->onSetStar((*reinterpret_cast<std::add_pointer_t<QTreeWidgetItem*>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.strings))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 18)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 18;
    }
    return _id;
}
QT_WARNING_POP
