#include <qttransitions.h>

QPropertyAnimation* fade(QWidget *parent, QWidget *obj, bool in, int duration)
{
    QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(parent);
    obj->setGraphicsEffect(eff);
    QPropertyAnimation *a = new QPropertyAnimation(eff, "opacity");
    a->setDuration(duration);
    a->setStartValue(in ? 0 : 1);
    a->setEndValue(in ? 1 : 0);
    a->setEasingCurve(in ? QEasingCurve::InBack : QEasingCurve::OutBack);
    a->start(QPropertyAnimation::DeleteWhenStopped);
    parent->connect(a, &QPropertyAnimation::finished, [=](){obj->setGraphicsEffect(nullptr);});

    return a;
}
