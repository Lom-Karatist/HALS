#include "OverlayLabel.h"

#include <QPainter>

OverlayLabel::OverlayLabel(QWidget *parent)
    : QLabel(parent),
      m_overlayRect(QRect()),
      m_overlayColor(0xd9, 0x77, 0x06),
      m_overlayOpacity(0.5) {}

void OverlayLabel::setOverlayRect(const QRect &rect) {
    m_overlayRect = rect;
    update();
}

void OverlayLabel::setOverlayColor(const QColor &color) {
    m_overlayColor = color;
    update();
}

void OverlayLabel::setOverlayOpacity(qreal opacity) {
    m_overlayOpacity = opacity;
    update();
}

void OverlayLabel::paintEvent(QPaintEvent *event) {
    QLabel::paintEvent(event);  // рисуем изображение
    if (!m_overlayRect.isNull()) {
        QPainter painter(this);
        QColor fillColor = m_overlayColor;
        fillColor.setAlphaF(m_overlayOpacity);
        painter.setBrush(fillColor);
        painter.setPen(QPen(m_overlayColor, 2));  // контур
        painter.drawRect(m_overlayRect);
    }
}
