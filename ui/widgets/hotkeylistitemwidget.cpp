#include "hotkeylistitemwidget.h"

HotkeyListItemWidget::HotkeyListItemWidget(std::shared_ptr<BaseManager> bm,
                                           std::string path, int height,
                                           bool isNew, QWidget *parent)
    : QWidget{parent},
      LoggableObject("Qt-HotkeyListItemWidget"),
      bm(bm),
      path(path) {
  this->setToolTip("Select");
  this->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
  width = parent->width();
  this->resize(width, height);

  label = new QLineEdit(this);
  connect(label, SIGNAL(textChanged(QString)), this,
          SLOT(labelChanged(QString)));
  label->setStyleSheet(
      "QLineEdit {color: rgba(255,255,255,80); border-radius: 3px; "
      "background-color: #303030; margin: 3px;}"
      "QLineEdit:hover {background-color: #404040;}");
  label->resize(label->width(), height);
  label->setPlaceholderText("Your label here");
  label->setTextMargins(10, 0, 10, 0);
  label->installEventFilter(this);
  arrange(label, 0, 8);
  if (!isNew)
    label->setText(QString::fromStdString(bm->GetSetting(path)["label"]));

  bb = new BindableButton(bm, path, isNew, this);
  bb->resize(bb->width(), height);
  arrange(bb, 9, 3);
}

void HotkeyListItemWidget::labelChanged(QString text) {
  if (text.size() > maxTextSize) label->setText(text.first(maxTextSize));
  if (text.size() == 0)
    label->setStyleSheet(
        "QLineEdit {color: #606060; border-radius: 3px; background-color: "
        "#303030; margin: 3px;}"
        "QLineEdit:hover {background-color: #404040;}");
  else
    label->setStyleSheet(
        "QLineEdit {color: #D0D0D0; border-radius: 3px; background-color: "
        "#303030; margin: 3px;}"
        "QLineEdit:hover {background-color: #404040;}");

  bm->UpdateSettings<std::string>(path + "/label", label->text().toStdString());
  log(INFO, "Label changed to \"" + text.toStdString() + "\"");
}

bool HotkeyListItemWidget::eventFilter(QObject *obj, QEvent *event) {
  if (obj == label && event->type() == QEvent::ToolTip) {
    QToolTip::hideText();
    return true;
  }
  return QWidget::eventFilter(obj, event);
}

void HotkeyListItemWidget::arrange(QWidget *widget, int position, int span) {
  float columnWidth = (float)width / columns;
  widget->move((int)(columnWidth * position), 0);
  widget->resize((int)(columnWidth * span), widget->height());
}
