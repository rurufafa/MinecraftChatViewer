#ifndef FRAMELESSHELPER_H
#define FRAMELESSHELPER_H

#include <QObject>
#include <QPoint> 
#include <QRect>

class QWidget;
class QEvent;

class FramelessHelper : public QObject {
    Q_OBJECT
public:
    FramelessHelper(QWidget *target, int border = 5);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QWidget *target_;
    bool leftPressed_;
    bool resizing_;
    QPoint dragPos_;
    QRect originalGeom_;
    int borderWidth_;
    enum ResizeRegion {
        None, Left, Right, Top, Bottom, TopLeft, TopRight, BottomLeft, BottomRight
    } resizeRegion_;

    void updateCursorShape(const QPoint &pos);
    ResizeRegion detectRegion(const QPoint &pos);
};

#endif // FRAMELESSHELPER_H
