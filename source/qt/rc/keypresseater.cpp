#include "keypresseater.h"

KeyPressEater::KeyPressEater(QObject *parent)
    : QObject{parent}
{

}

bool KeyPressEater::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        return true;
    } else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}
