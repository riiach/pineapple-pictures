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

signals:
    void closeRequested();
    void maximizeToggleRequested();
    void addRequested();

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
    bool addButtonActuallyVisible() const { return m_addButtonVisible && !m_closeButtonOnly; }

    OpacityHelper *m_opacityHelper;
    QIcon m_closeIcon;
    QIcon m_addIcon;
    bool m_closeButtonVisible = true;
    bool m_addButtonVisible = true;
    bool m_closeButtonOnly = false;
    bool m_closeHovered = false;
    bool m_closePressed = false;
    bool m_addHovered = false;
    bool m_addPressed = false;
    bool m_dragPending = false;
};

#endif // TITLEBAR_H
