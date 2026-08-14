// SPDX-FileCopyrightText: 2025 Gary Wang <git@blumia.net>
//
// SPDX-License-Identifier: MIT

#include "titlebar.h"

#include "opacityhelper.h"

#include <QPainter>
#include <QStyle>
#include <QMouseEvent>
#include <QEvent>
#include <QEnterEvent>
#include <QWindow>
#include <QCursor>

TitleBar::TitleBar(QWidget *parent)
    : QWidget(parent)
    , m_opacityHelper(new OpacityHelper(this))
    , m_closeIcon(QStringLiteral(":/icons/window-close.svg"))
    , m_addIcon(QStringLiteral(":/icons/list-add.svg"))
    , m_lockClosedIcon(QStringLiteral(":/icons/lock-closed.svg"))
    , m_lockOpenIcon(QStringLiteral(":/icons/lock-open.svg"))
{
    setFixedHeight(32);
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover, true);

    if (QWidget *win = window())
        win->installEventFilter(this);
}

void TitleBar::setOpacity(qreal opacity, bool animated)
{
    m_opacityHelper->setOpacity(opacity, animated);
}

void TitleBar::setCloseButtonVisible(bool visible)
{
    if (m_closeButtonVisible == visible)
        return;
    m_closeButtonVisible = visible;
    if (!visible) {
        m_closeHovered = false;
        m_closePressed = false;
    }
    update();
}

void TitleBar::setCloseButtonOnly(bool only)
{
    if (m_closeButtonOnly == only)
        return;
    m_closeButtonOnly = only;
    update();
}

void TitleBar::setAddButtonVisible(bool visible)
{
    if (m_addButtonVisible == visible)
        return;
    m_addButtonVisible = visible;
    if (!visible) {
        m_addHovered = false;
        m_addPressed = false;
    }
    update();
}

void TitleBar::setLocked(bool locked)
{
    if (m_locked == locked)
        return;
    m_locked = locked;
    // Leaving the locked/add-button-hidden state might change which
    // buttons are actually visible, so drop any stale hover state.
    m_addHovered = false;
    m_addPressed = false;
    update();
    emit lockToggled(m_locked);
}

QRect TitleBar::closeButtonRect() const
{
    const int btnWidth = closeButtonWidth();
    
    return QRect(isRightToLeft() ? 0 : width() - btnWidth, 0, btnWidth, height());
}

QRect TitleBar::addButtonRect() const
{
    // Placed right next to the close button, on its "inner" side, so the
    // two buttons always sit together regardless of layout direction.
    const int btnWidth = addButtonWidth();
    const QRect closeRect = closeButtonRect();

    return QRect(isRightToLeft() ? closeRect.right() + 1 : closeRect.left() - btnWidth,
                 0, btnWidth, height());
}

QRect TitleBar::lockButtonRect() const
{
    // Chained to whichever button currently sits closest to it (the "+"
    // button when visible, otherwise the close button directly), so there
    // is never a gap left behind when the "+" button gets hidden (e.g.
    // while locked).
    const int btnWidth = lockButtonWidth();
    const QRect anchor = addButtonActuallyVisible() ? addButtonRect() : closeButtonRect();

    return QRect(isRightToLeft() ? anchor.right() + 1 : anchor.left() - btnWidth,
                 0, btnWidth, height());
}

void TitleBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    // Subtle translucent backdrop so the title bar region is distinguishable
    // (similar to the bottom button group). Skipped in close-button-only
    // mode, and skipped while locked so only the Lock/Close buttons stand
    // out against the picture on hover.
    if (!m_closeButtonOnly && !m_locked) {
        painter.fillRect(rect(), QColor(0, 0, 0, 120));
    }

    const QRect closeRect = closeButtonRect();
    const bool addVisible = addButtonActuallyVisible();
    const QRect addRect = addButtonRect();
    const bool lockVisible = lockButtonActuallyVisible();
    const QRect lockRect = lockButtonRect();

    // Title text (leave room for the close button and, when visible, the
    // "+"/Lock buttons next to it). Hidden entirely while locked.
    QRect labelRect = rect();
    if (isRightToLeft()) {
        labelRect.adjust(0, 0, -8, 0);
        if (lockVisible)
            labelRect.setLeft(lockRect.right() + 2);
        else if (addVisible)
            labelRect.setLeft(addRect.right() + 2);
        else if (m_closeButtonVisible)
            labelRect.setLeft(closeRect.right() + 2);
    } else {
        labelRect.adjust(8, 0, 0, 0);
        if (lockVisible)
            labelRect.setRight(lockRect.left() - 2);
        else if (addVisible)
            labelRect.setRight(addRect.left() - 2);
        else if (m_closeButtonVisible)
            labelRect.setRight(closeRect.left() - 2);
    }

    const QString title = window() ? window()->windowTitle() : QString();
    if (!m_closeButtonOnly && !m_locked && !title.isEmpty()) {
        const QString elided = painter.fontMetrics().elidedText(title, Qt::ElideRight, labelRect.width());
        const int flags = Qt::AlignLeading | Qt::AlignVCenter | Qt::TextSingleLine;
        painter.setPen(Qt::black);
        painter.drawText(labelRect.adjusted(1, 1, 1, 1), flags, elided);
        painter.setPen(Qt::white);
        painter.drawText(labelRect, flags, elided);
    }

    if (m_closeButtonVisible) {
        if (m_closeHovered) {
            painter.fillRect(closeRect,
                             m_closePressed ? QColor(0xC5, 0x0F, 0x1F)
                                            : QColor(0xE8, 0x11, 0x23));
        }
        const int sz = height() / 3 * 2;
        const QRect iconRect = QStyle::alignedRect(layoutDirection(), Qt::AlignCenter,
                                                   QSize(sz, sz), closeRect);
        m_closeIcon.paint(&painter, iconRect);
    }

    if (addVisible) {
        if (m_addHovered) {
            painter.fillRect(addRect,
                             m_addPressed ? QColor(255, 255, 255, 60)
                                          : QColor(255, 255, 255, 35));
        }
        const int sz = height() / 3 * 2;
        const QRect iconRect = QStyle::alignedRect(layoutDirection(), Qt::AlignCenter,
                                                   QSize(sz, sz), addRect);
        m_addIcon.paint(&painter, iconRect);
    }

    if (lockVisible) {
        if (m_lockHovered) {
            painter.fillRect(lockRect,
                             m_lockPressed ? QColor(255, 255, 255, 60)
                                           : QColor(255, 255, 255, 35));
        }
        const int sz = height() / 3 * 2;
        const QRect iconRect = QStyle::alignedRect(layoutDirection(), Qt::AlignCenter,
                                                   QSize(sz, sz), lockRect);
        (m_locked ? m_lockClosedIcon : m_lockOpenIcon).paint(&painter, iconRect);
    }
}

void TitleBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    if (m_closeButtonVisible && closeButtonRect().contains(event->pos())) {
        m_closePressed = true;
        m_dragPending = false;
        update();
        event->accept();
        return;
    }

    if (addButtonActuallyVisible() && addButtonRect().contains(event->pos())) {
        m_addPressed = true;
        m_dragPending = false;
        update();
        event->accept();
        return;
    }

    if (lockButtonActuallyVisible() && lockButtonRect().contains(event->pos())) {
        m_lockPressed = true;
        m_dragPending = false;
        update();
        event->accept();
        return;
    }

    m_dragPending = true;
    event->accept();
}

void TitleBar::mouseMoveEvent(QMouseEvent *event)
{
    if (m_closeButtonVisible) {
        const bool hovered = closeButtonRect().contains(event->pos());
        if (hovered != m_closeHovered) {
            m_closeHovered = hovered;
            update();
        }
    }

    if (addButtonActuallyVisible()) {
        const bool hovered = addButtonRect().contains(event->pos());
        if (hovered != m_addHovered) {
            m_addHovered = hovered;
            update();
        }
    }

    if (lockButtonActuallyVisible()) {
        const bool hovered = lockButtonRect().contains(event->pos());
        if (hovered != m_lockHovered) {
            m_lockHovered = hovered;
            update();
        }
    }

#if defined(Q_OS_WIN)
    const bool shouldAcceptDrag = !window()->isMaximized() && !window()->isFullScreen();
#else
    const bool shouldAcceptDrag = !window()->isFullScreen();
#endif

    if (event->buttons() & Qt::LeftButton && m_dragPending && shouldAcceptDrag) {

        if (QWindow *wh = window()->windowHandle()) {
            if (wh->startSystemMove()) {
                m_dragPending = false;
            }
        }
        event->accept();
    }

    QWidget::mouseMoveEvent(event);
}

void TitleBar::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        const bool wasClosePressed = m_closePressed;
        const bool wasAddPressed = m_addPressed;
        const bool wasLockPressed = m_lockPressed;
        m_closePressed = false;
        m_addPressed = false;
        m_lockPressed = false;
        m_dragPending = false;
        update();
        if (wasClosePressed && m_closeButtonVisible
            && closeButtonRect().contains(event->pos())) {
            emit closeRequested();
            event->accept();
            return;
            }
        if (wasAddPressed && addButtonActuallyVisible()
            && addButtonRect().contains(event->pos())) {
            emit addRequested();
            event->accept();
            return;
            }
        if (wasLockPressed && lockButtonActuallyVisible()
            && lockButtonRect().contains(event->pos())) {
            setLocked(!m_locked);
            event->accept();
            return;
            }
    }

    QWidget::mouseReleaseEvent(event);
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    const bool onCloseButton = m_closeButtonVisible && closeButtonRect().contains(event->pos());
    const bool onAddButton = addButtonActuallyVisible() && addButtonRect().contains(event->pos());
    const bool onLockButton = lockButtonActuallyVisible() && lockButtonRect().contains(event->pos());
    if (event->button() == Qt::LeftButton && !onCloseButton && !onAddButton && !onLockButton) {
        emit maximizeToggleRequested();
        event->accept();
        return;
    }

    QWidget::mouseDoubleClickEvent(event);
}

void TitleBar::enterEvent(QEnterEvent *event)
{
    Q_UNUSED(event);
    if (m_closeButtonVisible) {
        const bool hovered = closeButtonRect().contains(mapFromGlobal(QCursor::pos()));
        if (hovered != m_closeHovered) {
            m_closeHovered = hovered;
            update();
        }
    }

    if (addButtonActuallyVisible()) {
        const bool hovered = addButtonRect().contains(mapFromGlobal(QCursor::pos()));
        if (hovered != m_addHovered) {
            m_addHovered = hovered;
            update();
        }
    }

    if (lockButtonActuallyVisible()) {
        const bool hovered = lockButtonRect().contains(mapFromGlobal(QCursor::pos()));
        if (hovered != m_lockHovered) {
            m_lockHovered = hovered;
            update();
        }
    }

    QWidget::enterEvent(event);
}

void TitleBar::leaveEvent(QEvent *event)
{
    if (m_closeHovered) {
        m_closeHovered = false;
        update();
    }
    if (m_addHovered) {
        m_addHovered = false;
        update();
    }
    if (m_lockHovered) {
        m_lockHovered = false;
        update();
    }

    QWidget::leaveEvent(event);
}

bool TitleBar::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == window()) {
        switch (event->type()) {
        case QEvent::WindowTitleChange:
        case QEvent::WindowStateChange:
        case QEvent::ActivationChange:
            update();
            break;
        default:
            break;
        }
    }

    return QWidget::eventFilter(watched, event);
}

QSize TitleBar::sizeHint() const
{
    return QSize(0, 32);
}
