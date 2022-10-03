#ifndef HUD_H
#define HUD_H

#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScreen>
#include <callbacks.h>
#include <thread>
#include <sstream>

namespace Ui {
class HUD;
}

class HUD : public QDialog
{
    Q_OBJECT

public:
    explicit HUD(passthroughData *data, QWidget *parent = nullptr);
    ~HUD();

    void SetPosition(int pos);
    int GetPosition() { return position; }
    void AddActiveEffect(QString fx);
    void RemoveActiveEffect(QString fx);

    std::atomic<bool> terminate = false;

private:
    void populateList();
    static void update(HUD* hud, passthroughData *data);

    Ui::HUD *ui;
    passthroughData *data;
    QLayout* layout;
    std::vector<QPoint> anchors = {};
    std::vector<QSize> sizes = {};
    std::map<QString, QLabel*> activeFX;
    std::vector<QLabel*> fxWidgets = {};

    int position = -1;
    int itemHeight = 0;
    bool vertical = true;
    bool anchorTop = false;

    std::thread t;
};

#endif // HUD_H
