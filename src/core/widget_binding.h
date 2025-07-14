#ifndef WIDGETBINDING_H
#define WIDGETBINDING_H

#include <QWidget>
#include <QVariant>
#include <QStringList>
#include <QListWidget>
#include <QTableWidget>

class ValueBinding {
public:
    static QVariant getWidgetValue(QWidget* widget);
    static void setWidgetValue(QWidget* widget, const QVariant& value);
    static void applyWidgetSettingsToConfig(const QList<QWidget*>& widgets);
    static void applyConfigToWidgets(const QList<QWidget*>& widgets);
    static void applyDefaultsToWidgets(const QList<QWidget*>& widgets);
};

class ListBinding {
public:
    static void saveListWidgetToConfig(QListWidget* widget, const QString& key);
    static void loadListWidgetFromConfig(QListWidget* widget, const QString& key);
};

class TableBinding {
public:
    static void saveTableWidgetToConfig(QTableWidget* widget, const QString& key);
    static void loadTableWidgetFromConfig(QTableWidget* widget, const QString& key);
};

#endif // WIDGETBINDING_H