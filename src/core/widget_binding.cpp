#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include <QKeySequenceEdit>
#include <QLabel>

#include "config_manager.h"
#include "widget_binding.h"

QVariant ValueBinding::getWidgetValue(QWidget* widget)
{
    if (auto* chk = qobject_cast<QCheckBox*>(widget)) return chk->isChecked();
    if (auto* sb = qobject_cast<QSpinBox*>(widget)) return sb->value();
    if (auto* dsb = qobject_cast<QDoubleSpinBox*>(widget)) return dsb->value();
    if (auto* cbx = qobject_cast<QComboBox*>(widget)) return cbx->currentText();
    if (auto* le = qobject_cast<QLineEdit*>(widget)) return le->text();
    if (auto* kse = qobject_cast<QKeySequenceEdit*>(widget)) return kse->keySequence().toString();
    if (auto* lbl = qobject_cast<QLabel*>(widget)) return lbl->text();
    return {};
}

void ValueBinding::setWidgetValue(QWidget* widget, const QVariant& value)
{
    if (auto* chk = qobject_cast<QCheckBox*>(widget)) chk->setChecked(value.toBool());
    else if (auto* sb = qobject_cast<QSpinBox*>(widget)) sb->setValue(value.toInt());
    else if (auto* dsb = qobject_cast<QDoubleSpinBox*>(widget)) dsb->setValue(value.toDouble());
    else if (auto* cbx = qobject_cast<QComboBox*>(widget)) {
        int index = cbx->findText(value.toString());
        if (index >= 0) cbx->setCurrentIndex(index);
    }
    else if (auto* le = qobject_cast<QLineEdit*>(widget)) le->setText(value.toString());
    else if (auto* kse = qobject_cast<QKeySequenceEdit*>(widget)) kse->setKeySequence(QKeySequence(value.toString()));
    else if (auto* lbl = qobject_cast<QLabel*>(widget)) lbl->setText(value.toString());
}

void ValueBinding::applyWidgetSettingsToConfig(const QList<QWidget*>& widgets)
{
    ConfigManager& config = ConfigManager::instance();

    for (QWidget* widget : widgets) {
        QString name = widget->objectName();
        if (!name.startsWith("input")) continue;

        QString key = name.mid(5);
        QVariant value = getWidgetValue(widget);
        if (value.isValid()) {
            config.set(key, value);
        }
    }
}

void ValueBinding::applyConfigToWidgets(const QList<QWidget*>& widgets)
{
    ConfigManager& config = ConfigManager::instance();

    for (QWidget* widget : widgets) {
        QString name = widget->objectName();
        if (!name.startsWith("input")) continue;

        QString key = name.mid(5);
        QVariant value = config.get(key);
        if (value.isValid()) {
            setWidgetValue(widget, value);
        }
    }
}

void ValueBinding::applyDefaultsToWidgets(const QList<QWidget*>& widgets)
{
    ConfigManager& config = ConfigManager::instance();
    const QJsonObject& defaults = config.defaultConfig();

    for (QWidget* widget : widgets) {
        QString name = widget->objectName();
        if (!name.startsWith("input")) continue;

        QString key = name.mid(5);
        if (!defaults.contains(key)) continue;

        QVariant value = defaults.value(key).toVariant();
        setWidgetValue(widget, value);
    }
}

void ListBinding::saveListWidgetToConfig(QListWidget* widget, const QString& key)
{
    ConfigManager& config = ConfigManager::instance();

    QStringList items;
    for (int i = 0; i < widget->count(); ++i) {
        items << widget->item(i)->text();
    }

    config.set(key, items);
}

void ListBinding::loadListWidgetFromConfig(QListWidget* widget, const QString& key)
{
    ConfigManager& config = ConfigManager::instance();

    widget->clear();

    QVariant value = config.get(key);
    if (!value.isValid()) return;

    QStringList items = value.toStringList();
    for (const QString& item : items) {
        widget->addItem(item);
    }
}

void TableBinding::saveTableWidgetToConfig(QTableWidget* widget, const QString& key)
{
    ConfigManager& config = ConfigManager::instance();

    QVariantMap map;
    for (int row = 0; row < widget->rowCount(); ++row) {
        QTableWidgetItem* keyItem = widget->item(row, 0);
        QTableWidgetItem* valueItem = widget->item(row, 1);
        if (!keyItem || !valueItem) continue;

        QString mapKey = keyItem->text();
        QString mapValue = valueItem->text();
        if (!mapKey.isEmpty()) {
            map.insert(mapKey, mapValue);
        }
    }

    config.set(key, map);
}

void TableBinding::loadTableWidgetFromConfig(QTableWidget* widget, const QString& key)
{
    ConfigManager& config = ConfigManager::instance();

    widget->setRowCount(0); 

    QVariant value = config.get(key);
    if (!value.isValid()) return;

    QVariantMap map = value.toMap();
    int row = 0;
    for (auto it = map.begin(); it != map.end(); ++it, ++row) {
        widget->insertRow(row);
        widget->setItem(row, 0, new QTableWidgetItem(it.key()));
        widget->setItem(row, 1, new QTableWidgetItem(it.value().toString()));
    }
}
