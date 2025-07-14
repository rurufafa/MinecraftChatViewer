#include <QApplication>
#include <QCursor>
#include <QEvent>
#include <QMouseEvent>
#include <QWidget>

#include "frameless_helper.h"

FramelessHelper::FramelessHelper(QWidget *target, int border)
    : QObject(target), target_(target), leftPressed_(false), resizing_(false),
      borderWidth_(border), resizeRegion_(None)
{
    target_->setMouseTracking(true);
    target_->setAttribute(Qt::WA_TranslucentBackground);
    target_->setAttribute(Qt::WA_NoSystemBackground);
    target_->setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    target_->installEventFilter(this);
    target_->installEventFilter(this);
}

bool FramelessHelper::eventFilter(QObject *watched, QEvent *event) {
    if (watched != target_) {
        return QObject::eventFilter(watched, event);
    }
    
    QEvent::Type type = event->type();
    if (type != QEvent::MouseMove &&
        type != QEvent::Leave &&
        type != QEvent::MouseButtonPress &&
        type != QEvent::MouseButtonRelease) {
        return QObject::eventFilter(watched, event);
    }

    switch (type) {
    case QEvent::MouseMove: {
        QMouseEvent *e = static_cast<QMouseEvent*>(event);
        QPoint globalPos = e->globalPosition().toPoint();
        QPoint localPos = e->pos();

        if (leftPressed_) {
            if (resizing_) {
                QRect newGeom = originalGeom_;

                switch (resizeRegion_) {
                case Left:
                    newGeom.setLeft(globalPos.x());
                    break;
                case Right:
                    newGeom.setRight(globalPos.x());
                    break;
                case Top:
                    newGeom.setTop(globalPos.y());
                    break;
                case Bottom:
                    newGeom.setBottom(globalPos.y());
                    break;
                case TopLeft:
                    newGeom.setTopLeft(globalPos);
                    break;
                case TopRight:
                    newGeom.setTopRight(globalPos);
                    break;
                case BottomLeft:
                    newGeom.setBottomLeft(globalPos);
                    break;
                case BottomRight:
                    newGeom.setBottomRight(globalPos);
                    break;
                case None:
                    break;
                }

                QSize minSize = target_->minimumSize();
                if (newGeom.width() < minSize.width())
                    newGeom.setWidth(minSize.width());
                if (newGeom.height() < minSize.height())
                    newGeom.setHeight(minSize.height());

                target_->setGeometry(newGeom.normalized());
            } else {
                target_->move(target_->frameGeometry().topLeft() + (e->pos() - dragPos_));
            }
        } else {
            updateCursorShape(globalPos);
        }
        return true;
    }
    case QEvent::MouseButtonPress: {
        QMouseEvent *e = static_cast<QMouseEvent*>(event);
        if (e->button() == Qt::LeftButton) {
            leftPressed_ = true;
            resizeRegion_ = detectRegion(e->globalPosition().toPoint());
            if (resizeRegion_ != None) {
                resizing_ = true;
                originalGeom_ = target_->frameGeometry();
            } else {
                dragPos_ = e->pos();
            }
        }
        return true;
    }
    case QEvent::MouseButtonRelease: {
        if (static_cast<QMouseEvent*>(event)->button() == Qt::LeftButton) {
            leftPressed_ = false;
            resizing_ = false;
            resizeRegion_ = None;
        }
        return true;
    }
    case QEvent::Leave: {
        QApplication::restoreOverrideCursor();
        return true;
    }
    }
    return QObject::eventFilter(watched, event);
}

void FramelessHelper::updateCursorShape(const QPoint &globalPos) {
    ResizeRegion region = detectRegion(globalPos);
    Qt::CursorShape shape = Qt::ArrowCursor;

    switch (region) {
    case TopLeft:
    case BottomRight:
        shape = Qt::SizeFDiagCursor;
        break;
    case TopRight:
    case BottomLeft:
        shape = Qt::SizeBDiagCursor;
        break;
    case Top:
    case Bottom:
        shape = Qt::SizeVerCursor;
        break;
    case Left:
    case Right:
        shape = Qt::SizeHorCursor;
        break;
    default:
        shape = Qt::ArrowCursor;
        break;
    }

    QApplication::setOverrideCursor(shape);
}

FramelessHelper::ResizeRegion FramelessHelper::detectRegion(const QPoint &globalPos) {
    QRect rect = target_->frameGeometry();
    int x = globalPos.x();
    int y = globalPos.y();

    bool onLeft   = abs(x - rect.left())   <= borderWidth_;
    bool onRight  = abs(x - rect.right())  <= borderWidth_;
    bool onTop    = abs(y - rect.top())    <= borderWidth_;
    bool onBottom = abs(y - rect.bottom()) <= borderWidth_;

    if (onTop && onLeft) return TopLeft;
    if (onTop && onRight) return TopRight;
    if (onBottom && onLeft) return BottomLeft;
    if (onBottom && onRight) return BottomRight;
    if (onTop) return Top;
    if (onBottom) return Bottom;
    if (onLeft) return Left;
    if (onRight) return Right;

    return None;
}
