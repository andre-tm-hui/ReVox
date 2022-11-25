#include "qttransitions.h"

QPropertyAnimation* fade(QWidget *parent, QWidget *obj, bool in, int duration)
{
    QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(parent);
    obj->setGraphicsEffect(eff);
    QPropertyAnimation *a = new QPropertyAnimation(eff, "opacity");
    a->setDuration(duration);
    a->setStartValue(obj->isHidden() ? 0 : 1);
    obj->show();
    a->setEndValue(in ? 1 : 0);
    a->setEasingCurve(in ? QEasingCurve::InBack : QEasingCurve::OutBack);
    a->start(QPropertyAnimation::DeleteWhenStopped);
    parent->connect(a, &QPropertyAnimation::finished, [=](){
        obj->setGraphicsEffect(nullptr);
        if (!in) obj->hide();
    });

    return a;
}


QPropertyAnimation* slideToValue(QWidget *obj, int target, int duration) {
    //obj->blockSignals(true);
    QPropertyAnimation *a = new QPropertyAnimation(obj, "value");
    //obj->connect(a, &QPropertyAnimation::finished, [=](){obj->blockSignals(false);});
    a->setDuration(duration);
    a->setEndValue(target);
    a->setEasingCurve(QEasingCurve::OutCubic);
    a->start(QPropertyAnimation::DeleteWhenStopped);
    return a;
}
