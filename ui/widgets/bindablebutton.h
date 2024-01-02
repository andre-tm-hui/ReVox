#ifndef BINDABLEBUTTON_H
#define BINDABLEBUTTON_H

#include <QPushButton>
#include <thread>

#include "../../interface/basemanager.h"
#include "../../util/friendlyDeviceName.h"
#include "../../backend/vkcodenames.h"

class BindableButton : public QPushButton, public LoggableObject {
  Q_OBJECT
 public:
  BindableButton(std::shared_ptr<BaseManager> bm, std::string path,
                 bool isNew = false, QWidget *parent = nullptr);

  void setSettingPath(std::string path);

  void manualRebind() { startRebinding(); }

 signals:
  void newHotkeySet();

 private slots:
  void startRebinding();
  void cleanUp();

 private:
  static void waitForRebind(BindableButton *hkb,
                            std::shared_ptr<BaseManager> bm);
  void updateSelf();

  bool isBasic;
  std::string path = "";
  std::shared_ptr<BaseManager> bm;
};

#endif  // BINDABLEBUTTON_H
