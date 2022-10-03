#ifndef QTTRANSITIONS_H
#define QTTRANSITIONS_H

#include <QWidget>
#include <QGroupBox>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

QPropertyAnimation* fade(QWidget *parent, QWidget *obj, bool in, int duration);

#endif // QTTRANSITIONS_H
