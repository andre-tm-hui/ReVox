#include "hud.h"
#include "ui_hud.h"

HUD::HUD(passthroughData *data, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HUD)
{
    ui->setupUi(this);
    this->data = data;

    this->setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint|Qt::SubWindow|Qt::WindowTransparentForInput);
    this->setAttribute(Qt::WidgetAttribute::WA_TranslucentBackground, true);
    this->setAttribute(Qt::WidgetAttribute::WA_ShowWithoutActivating, true);

    QScreen *screen = QGuiApplication::primaryScreen();
    int width = screen->geometry().width();
    int height = screen->geometry().height();

    itemHeight = height / 35;

    int padding = itemHeight;

    sizes.push_back(QSize(width / 16, 16));
    sizes.push_back(QSize(16, itemHeight));

    anchors.push_back(QPoint(padding, padding));
    anchors.push_back(QPoint(width / 2, padding));
    anchors.push_back(QPoint(width - (width / 16) - padding, padding));
    anchors.push_back(QPoint(padding, height / 2));
    anchors.push_back(QPoint(width - (width / 16) - padding, height / 2));
    anchors.push_back(QPoint(padding, height - padding));
    anchors.push_back(QPoint(width / 2, height - 20 - padding));
    anchors.push_back(QPoint(width - width / 16 - padding, height - padding));

    //AddActiveEffect("Test");
}

HUD::~HUD()
{
    delete ui;
}

void HUD::SetPosition(int pos)
{
    this->position = pos;
    if (pos >= 0 && pos < 8)
    {
        QRect rect(0, 0, 0, 0);
        rect.setTopLeft(anchors[pos]);
        switch (pos)
        {
        case 0: case 2: case 3: case 4: case 5: case 7:
            rect.setSize(sizes[0]);
            break;
        case 1: case 6:
            rect.setSize(sizes[1]);
            break;
        default:
            break;
        }

        this->setGeometry(rect);
        this->populateList();
        this->show();
    }
    else
    {
        this->hide();
    }
}

void HUD::AddActiveEffect(QString fx)
{
    if (activeFX.find(fx) == activeFX.end())
    {
        QLabel *label = new QLabel(this);
        label->show();
        label->setText(fx);
        std::ostringstream ss;
        ss<<"background: rgba(0,0,0,100); border: 1px solid transparent; border-radius: "<<itemHeight / 5<<"px; color: rgba(255,255,255,100); margin: 5%; font-size: "<<(int)(itemHeight / 2.6)<<"px;";
        label->setStyleSheet(QString::fromStdString(ss.str()));
        label->adjustSize();
        int diff = itemHeight - label->size().height();
        label->resize(label->width() + 2 * diff, itemHeight);

        int xPos = 0;

        switch (position)
        {
        case 2:
            xPos = this->width() - label->width();
        case 0: // top right/left
            this->resize(this->width(), fxWidgets.size() == 0 ? itemHeight : this->height() + itemHeight);
            label->move(xPos, this->height() - itemHeight);
            break;
        case 4:
            xPos = this->width() - label->width();
        case 3: // right/left
            this->resize(this->width(), fxWidgets.size() == 0 ? itemHeight : this->height() + itemHeight);
            this->move(this->pos().x(), this->pos().y() - itemHeight / 2);
            label->move(xPos, this->height() - itemHeight);
            break;
        case 1: case 6: // top/bottom
            this->resize(fxWidgets.size() == 0 ? label->width() : this->width() + label->width(), this->height());
            this->move(this->pos().x() - label->width() / 2, this->pos().y());
            label->move(this->width() - label->width(), 0);
            break;
        case 7:
            xPos = this->width() - label->width();
        case 5: // bottom right/left
            this->resize(this->width(), fxWidgets.size() == 0 ? itemHeight : this->height() + itemHeight);
            this->move(this->pos().x(), this->pos().y() - itemHeight);
            for (auto const& [fx, widget] : activeFX)
            {
                widget->move(widget->pos().x(), widget->pos().y() + itemHeight);
            }
            label->move(xPos, 0);
            break;
        default:
            return;
        }

        label->setAlignment(Qt::AlignCenter);
        activeFX[fx] = label;
        fxWidgets.push_back(label);
    }
}

void HUD::RemoveActiveEffect(QString fx)
{
    if (activeFX.find(fx) != activeFX.end())
    {
        QLabel *widget = activeFX[fx];
        int index = -1;

        switch (position)
        {
        case 3: case 4:
            this->move(this->pos().x(), this->pos().y() + itemHeight / 2);
        case 0: case 2:
            for (unsigned i = 0; i < fxWidgets.size(); i++)
            {
                if (index != -1) fxWidgets[i]->move(fxWidgets[i]->pos().x(), fxWidgets[i]->pos().y() - itemHeight);
                if (fxWidgets[i] == widget) index = i;
            }
            this->resize(this->size().width(), this->size().height() - itemHeight);
            break;
        case 1: case 6:
            for (unsigned i = 0; i < fxWidgets.size(); i++)
            {
                if (index != -1) fxWidgets[i]->move(fxWidgets[i]->pos().x() - widget->width(), fxWidgets[i]->pos().y());
                if (fxWidgets[i] == widget) index = i;
            }
            this->resize(this->width() - widget->width(), this->size().height());
            this->move(this->pos().x() + widget->width() / 2, this->pos().y());
            break;
        case 5: case 7:
            for (unsigned i = 0; i < fxWidgets.size(); i++)
            {
                if (fxWidgets[i] == widget) { index = i; break; }
                fxWidgets[i]->move(fxWidgets[i]->pos().x(), fxWidgets[i]->pos().y() - itemHeight);
            }
            this->resize(this->width(), this->height() - itemHeight);
            this->move(this->pos().x(), this->pos().y() + itemHeight);
            break;
        default:
            return;
        }

        activeFX.erase(fx);
        if (index >= 0) fxWidgets.erase(fxWidgets.begin() + index);
        delete widget;
    }
}

void HUD::populateList()
{
    for (unsigned int i = 0; i < fxWidgets.size(); i++)
    {
        QLabel *label = fxWidgets[i];
        int xPos = 0;

        switch (position)
        {
        case 2:
            xPos = this->width() - label->width();
        case 0: // top right/left
            this->resize(this->width(), fxWidgets.size() == 0 ? itemHeight : this->height() + itemHeight);
            label->move(xPos, this->height() - itemHeight);
            break;
        case 4:
            xPos = this->width() - label->width();
        case 3: // right/left
            this->resize(this->width(), fxWidgets.size() == 0 ? itemHeight : this->height() + itemHeight);
            this->move(this->pos().x(), this->pos().y() - itemHeight / 2);
            label->move(xPos, this->height() - itemHeight);
            break;
        case 1: case 6: // top/bottom
            this->resize(fxWidgets.size() == 0 ? label->width() : this->width() + label->width(), this->height());
            this->move(this->pos().x() - label->width() / 2, this->pos().y());
            label->move(this->width() - label->width(), 0);
            break;
        case 7:
            xPos = this->width() - label->width();
        case 5: // bottom right/left
            this->resize(this->width(), fxWidgets.size() == 0 ? itemHeight : this->height() + itemHeight);
            this->move(this->pos().x(), this->pos().y() - itemHeight);
            for (auto const& [fx, widget] : activeFX)
            {
                widget->move(widget->pos().x(), widget->pos().y() + itemHeight);
            }
            label->move(xPos, 0);
            break;
        default:
            return;
        }
    }
}
