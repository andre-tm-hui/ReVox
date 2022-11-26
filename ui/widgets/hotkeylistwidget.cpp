#include "hotkeylistwidget.h"

HotkeyListWidget::HotkeyListWidget(QSize size, std::shared_ptr<BaseManager> bm, std::string path, QWidget *parent)
    : QWidget{parent}, size(size), bm(bm), path(path)
{
    this->resize(size);
    this->setMask(QBitmap(":/rc/hotkeyList/masks/hotkeylist.bmp"));

    bg = new QWidget(this);
    bg->resize(size);
    bg->setStyleSheet("background: #181818;");

    list = new QListWidget(bg);
    list->resize(size.width(), 0);
    list->setStyleSheet("QListWidget {background-color: transparent; border: 0px; outline: 0;}"
                        "QListWidget::item:hover {background-color: #009090;}"
                        "QListWidget::item:disabled {background-color: transparent;}"
                        "QListWidget::item:selected {background-color: #008080;}");
    list->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    list->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    connect(list, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectionChanged()));

    add = new QToolButton(bg);
    add->resize(28, 19);
    add->move(size.width() - 28, size.height() - 19);
    add->setToolTip("Add a new hotkey");
    add->setStyleSheet("QToolButton {background-color: #202020; border: 0px; border-left: 1px solid #181818;}"
                       "QToolButton:hover {background-color: #303030;}");
    QIcon addIcon = QIcon();
    addIcon.addFile(":/rc/hotkeyList/addIdle.png", QSize(13, 13), QIcon::Normal);
    addIcon.addFile(":/rc/hotkeyList/addDisabled.png", QSize(13, 13), QIcon::Disabled);
    addIcon.addFile(":/rc/hotkeyList/addHover.png", QSize(13, 13), QIcon::Active);
    add->setIcon(addIcon);
    add->setIconSize(QSize(13, 13));
    connect(add, SIGNAL(pressed()), this, SLOT(addHotkey()));

    remove = new QToolButton(bg);
    remove->resize(28, 19);
    remove->move(size.width() - 2 * 28, size.height() - 19);
    remove->setToolTip("Remove selected hotkey");
    remove->setStyleSheet("QToolButton {background-color: #202020; border: 0px; border-right: 1px solid #181818; border-top-left-radius: 10px;}"
                          "QToolButton:hover {background-color: #303030;}");
    QIcon removeIcon = QIcon();
    removeIcon.addFile(":/rc/hotkeyList/removeIdle.png", QSize(13, 13), QIcon::Normal);
    removeIcon.addFile(":/rc/hotkeyList/removeDisabled.png", QSize(13, 13), QIcon::Disabled);
    removeIcon.addFile(":/rc/hotkeyList/removeHover.png", QSize(13, 13), QIcon::Active);
    remove->setIcon(removeIcon);
    remove->setIconSize(QSize(13, 13));
    remove->setEnabled(false);
    connect(remove, SIGNAL(pressed()), this, SLOT(removeHotkey()));

    initializeList();
}

void HotkeyListWidget::deselect() {
    list->clearSelection();
    currHotkey = -1;
    remove->setEnabled(false);
}

void HotkeyListWidget::initializeList() {
    json hotkeys = bm->GetSetting(path);
    for (int i = 0; i < hotkeys.size(); i++)
        addHotkey(false);
}

void HotkeyListWidget::selectionChanged() {
    if (bm->GetSetting(path + "/" + std::to_string(list->currentRow()) + "/editable").get<bool>())
        remove->setEnabled(true);
    else
        remove->setEnabled(false);

    if (list->currentRow() != currHotkey)
        emit onSelectionChanged(list->currentRow());
    currHotkey = list->currentRow();
}

void HotkeyListWidget::addHotkey(bool isNew) {
    HotkeyListItemWidget *item = new HotkeyListItemWidget(bm,
                                                          path + "/" + std::to_string(list->count()),
                                                          itemHeight,
                                                          isNew,
                                                          list);
    QListWidgetItem *orig = new QListWidgetItem();
    orig->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
    list->addItem(orig);
    list->setItemWidget(orig, item);
    orig->setSizeHint(QSize(0, itemHeight));

    list->scrollToBottom();
    list->resize(list->width(), __min(itemHeight * list->count() + 2, size.height()));

    emit hotkeyAdded();
}

void HotkeyListWidget::removeHotkey() {
    int currentRow = list->currentRow();
    if (currentRow < 0 || currentRow >= list->count()) return;

    this->blockSignals(true);
    std::string filename = bm->GetRootDir() + "samples/" + std::to_string(currentRow) + ".mp3";
    if (std::filesystem::exists(filename)) {
        std::filesystem::remove(filename);
    }
    bm->RemoveBind(currentRow);
    list->removeItemWidget(list->currentItem());
    delete list->currentItem();

    HotkeyListItemWidget *item;
    for (int i = currentRow; i < list->count(); i++) {
        filename = bm->GetRootDir() + "samples/" + std::to_string(i + 1) + ".mp3";
        if (std::filesystem::exists(filename)) {
            std::filesystem::rename(filename, bm->GetRootDir() + "samples/" + std::to_string(i) + ".mp3");
        }
        item = qobject_cast<HotkeyListItemWidget*>(list->itemWidget(list->item(i)));
        item->setSettingPath(path + "/" + std::to_string(i));
    }

    this->blockSignals(false);
    deselect();
    list->setCurrentRow(-1);
    emit hotkeyRemoved();
}
