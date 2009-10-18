/**
 * Copyright (c) 2009, Josef Kufner  <jk@myserver.cz>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Benjamin Meyer nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "incdecbutton.h"

#include <qpainter.h>
#include <QMouseEvent>

IncDecButton::IncDecButton(QWidget *parent)
    : QAbstractButton(parent)
{
    setCursor(Qt::ArrowCursor);
    setFocusPolicy(Qt::NoFocus);
    setToolTip(tr("Increment/decrement number"));
    setMinimumSize(22, 22);
    setVisible(true);	//TODO

    connect(this, SIGNAL(clicked()),
            this, SLOT(click()));
}

void IncDecButton::click()
{
    if (clickDirection == ClickUpper) {
        emit increment();
    } else {
        emit decrement();
    }
}

void IncDecButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    static const QPointF upper_triangle_points[] = {
        QPointF(0.5, 0.2),
        QPointF(0.7, 0.4),
        QPointF(0.3, 0.4),
    };

    static const QPointF lower_triangle_points[] = {
        QPointF(0.5, 0.8),
        QPointF(0.7, 0.6),
        QPointF(0.3, 0.6),
    };

    painter.setRenderHint(QPainter::Antialiasing, true);

    QPalette p = palette();
    QColor cn = p.color(QPalette::Mid);
    QColor cd = isDown() ? p.color(QPalette::Text) : cn;

    QTransform trans;
    trans.scale(width(), height());
    painter.setWorldTransform(trans);

    painter.setBrush(clickDirection == ClickUpper ? cd : cn);
    painter.setPen(clickDirection == ClickUpper ? cd : cn);
    painter.drawPolygon(upper_triangle_points, 3);

    painter.setBrush(clickDirection == ClickLower ? cd : cn);
    painter.setPen(clickDirection == ClickLower ? cd : cn);
    painter.drawPolygon(lower_triangle_points, 3);
}


void IncDecButton::mousePressEvent (QMouseEvent *e)
{
    clickDirection = e->y() > height() / 2 ? ClickLower : ClickUpper;
    QAbstractButton::mousePressEvent(e);
}


