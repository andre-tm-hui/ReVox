#ifndef HUD_H
#define HUD_H

#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QScreen>
#include <QVBoxLayout>
#include <sstream>

#include "../util/loggableobject.h"

namespace Ui {
class HUD;
}

class HUD : public QDialog, public LoggableObject {
  Q_OBJECT

 public:
  explicit HUD(QWidget *parent = nullptr);
  ~HUD();

  void SetPosition(int pos);
  int GetPosition() { return position; }
  void AddActiveEffect(QString fx);
  void RemoveActiveEffect(QString fx);

 private:
  void populateList();

  Ui::HUD *ui;
  QLayout *layout;
  std::vector<QPoint> anchors = {};
  std::vector<QSize> sizes = {};
  std::map<QString, QLabel *> activeFX;
  std::vector<QLabel *> fxWidgets = {};

  int position = -1;
  int itemHeight = 0;
  bool vertical = true;
  bool anchorTop = false;
};

#endif  // HUD_H
