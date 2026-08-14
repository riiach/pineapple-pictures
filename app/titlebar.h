// SPDX-FileCopyrightText: 2025 Gary Wang <git@blumia.net>
//
// SPDX-License-Identifier: MIT

#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>
#include <QIcon>

QT_BEGIN_NAMESPACE
class QPaintEvent;
class QMouseEvent;
class QEvent;
class QEnterEvent;
QT_END_NAMESPACE

class OpacityHelper;

class TitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit TitleBar(QWidget *parent = nullptr);

    void setOpacity(qreal opacity, bool animated = true);
    void setCloseButtonVisible(bool visible);
    bool closeButtonOnly() const { return m_closeButtonOnly; }
    void setCloseButtonOnly(bool only);
    int closeButtonWidth() const { return qMax(height(), 46); }
    // "+" button, used to open one or more additional images in new,
    // independent windows.
    void setAddButtonVisible(bool visible);
    int addButtonWidth() const { return qMax(height(), 46); }
    // Lock button: toggles a locked state where only the Lock and Close
    // buttons remain visible on hover, hiding the rest of the UI.
    bool locked() const { return m_locked; }
    void setLocked(bool locked);
    int lockButtonWidth() const { return qMax(height(), 46); }

signals:
    void closeRequested();
    void maximizeToggleRequested();
    void addRequested();
    void lockToggled(bool locked);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    QSize sizeHint() const override;

private:
    QRect closeButtonRect() const;
    QRect addButtonRect() const;
    QRect lockButtonRect() const;
    bool addButtonActuallyVisible() const { return m_addButtonVisible && !m_closeButtonOnly && !m_locked; }
    bool lockButtonActuallyVisible() const { return m_closeButtonVisible; }

    OpacityHelper *m_opacityHelper;
    QIcon m_closeIcon;
    QIcon m_addIcon;
    QIcon m_lockClosedIcon;
    QIcon m_lockOpenIcon;
    bool m_closeButtonVisible = true;
    bool m_addButtonVisible = true;
    bool m_closeButtonOnly = false;
    bool m_locked = false;
    bool m_closeHovered = false;
    bool m_closePressed = false;
    bool m_addHovered = false;
    bool m_addPressed = false;
    bool m_lockHovered = false;
    bool m_lockPressed = false;
    bool m_dragPending = false;
};

#endif // TITLEBAR_H
