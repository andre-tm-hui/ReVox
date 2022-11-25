#include "customdial.h"

CustomDial::CustomDial(QWidget *parent,
                       double knobRadius,
                       double knobMargin)
    : QDial(parent),
      knobRadius_(knobRadius),
      knobMargin_(knobMargin)
{
    QDial::setRange(0, 100);
}


void CustomDial::setKnobRadius(double radius)
{
    knobRadius_ = radius;
}

double CustomDial::getKnobRadius() const
{
    return knobRadius_;
}

void CustomDial::setKnobMargin(double margin)
{
    knobMargin_ = margin;
}

double CustomDial::getKnobMargin() const
{
    return knobMargin_;
}

void CustomDial::paintEvent(QPaintEvent*)
{
    static const double degree270 = 1.5 * M_PI;
    static const double degree225 = 1.25 * M_PI;

    QPainter painter(this);

    painter.setBackgroundMode(Qt::OpaqueMode);

    painter.setRenderHint(QPainter::Antialiasing);

    painter.setBrush(painter.background());

    QColor pointColor(painter.pen().color());

    painter.setPen(QPen(Qt::NoPen));

    painter.drawEllipse(0, 0, QDial::height(), QDial::height());

    painter.setBrush(QBrush(pointColor));

    double ratio = (double)(QDial::value() - QDial::minimum()) / (QDial::maximum() - QDial::minimum());

    double angle = ratio * degree270 - degree225;

    double r = QDial::height() / 2.0;

    double y = sin(angle) * (r - knobRadius_ - knobMargin_) + r;

    double x = cos(angle) * (r - knobRadius_ - knobMargin_) + r;

    painter.drawEllipse(QPointF(x,y), knobRadius_, knobRadius_);
}

void CustomDial::lerpToValue(int value, int ms)
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "sliderPosition");
    animation->setDuration(ms);
    animation->setStartValue(this->value());
    animation->setEndValue(value);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    animation->start(QPropertyAnimation::DeleteWhenStopped);
}
