#ifndef HOTKEYLISTITEMWIDGET_H
#define HOTKEYLISTITEMWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QEvent>
#include <QToolTip>
#include "bindablebutton.h"
#include "../../interface/basemanager.h"

class HotkeyListItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HotkeyListItemWidget(std::shared_ptr<BaseManager> bm, std::string path, int height, bool isNew = false, QWidget *parent = nullptr);

    void setSettingPath(std::string path) { this->path = path; bb->setSettingPath(path); }

signals:

private slots:
    void labelChanged(QString text);

private:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void arrange(QWidget *widget, int position, int span);

    const int columns = 12, maxTextSize = 50;
    int width;

    std::shared_ptr<BaseManager> bm;
    std::string path;

    QLineEdit *label;
    QWidget *icon;
    BindableButton *bb;
};

#endif // HOTKEYLISTITEMWIDGET_H
