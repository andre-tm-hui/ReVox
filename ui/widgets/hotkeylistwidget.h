#ifndef HOTKEYLISTWIDGET_H
#define HOTKEYLISTWIDGET_H

#include <QAbstractItemView>
#include <QListWidget>
#include <QToolButton>
#include <QWidget>

#include "../../interface/basemanager.h"
#include "hotkeylistitemwidget.h"

class HotkeyListWidget : public QWidget, public LoggableObject {
  Q_OBJECT
 public:
  explicit HotkeyListWidget(QSize size, std::shared_ptr<BaseManager> bm,
                            std::string path, QWidget *parent = nullptr);
  void deselect();

 signals:
  void onSelectionChanged(int idx);
  void hotkeyAdded();
  void hotkeyRemoved();

 private slots:
  void selectionChanged();
  void addHotkey(bool isNew = true);
  void removeHotkey();

 private:
  void initializeList();

  QWidget *bg;
  QListWidget *list;
  QToolButton *add, *remove;

  QSize size;
  std::shared_ptr<BaseManager> bm;
  std::string path;

  const int itemHeight = 36;
  int currHotkey = -1;
};

#endif  // HOTKEYLISTWIDGET_H
