#include "CongfigGui.h"
#include <QtEvents>

CongfigGui::CongfigGui(QWidget *parent, Qt::WindowFlags flags) : QWidget(parent, flags)
{
    ui = new Ui::ConfigGuiClass();
    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    this->setMouseTracking(true);

    ui->title->installEventFilter(this);
}

CongfigGui::~CongfigGui() = default;

bool CongfigGui::eventFilter(QObject *object, QEvent *event)
{
    static QPoint mousePoint;
    static bool   mousePressed = false;
    const auto   *pEvent       = dynamic_cast<QMouseEvent *>(event);
    if (pEvent)
    {
        if (pEvent->type() == QEvent::MouseButtonPress)
        {
            if (pEvent->button() == Qt::LeftButton)
            {
                mousePressed = true;
                mousePoint   = pEvent->globalPos() - this->pos();
                return true;
            }
        }
        else if (pEvent->type() == QEvent::MouseButtonRelease)
        {
            mousePressed = false;
            return true;
        }
        else if (pEvent->type() == QEvent::MouseMove)
        {
            if (mousePressed && (pEvent->buttons() && Qt::LeftButton))
            {
                this->move(pEvent->globalPos() - mousePoint);
                return true;
            }
        }
    }

    return QWidget::eventFilter(object, event);
}
