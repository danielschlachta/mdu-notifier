#include <QtGui>
#include <QLayout>

#include "warndialog.h"

WarnDialog::WarnDialog(QWidget *parent, QString message, long timeout) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{
    label1 = new QLabel(tr("Mobile data is running low.\n"));
    label2 = new QLabel(message + tr("\n"));

    okButton = new QPushButton(tr("&Ok"));
    okButton->setDefault(true);

    dismissButton = new QPushButton(tr("&Dismiss"));

    connect(okButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(dismissButton, SIGNAL(clicked()), this, SLOT(dismissClicked()));

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(okButton);
    bottomLayout->addWidget(dismissButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(label1);
    mainLayout->addWidget(label2);
    mainLayout->addLayout(bottomLayout);
    setLayout(mainLayout);

    setWindowTitle(tr("Mobile data usage"));
    setFixedHeight(sizeHint().height());

    timerId = startTimer(timeout);
}

void WarnDialog::dismissClicked()
{
   close();
   emit dismissed();
}

void WarnDialog::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == timerId)
        close();
}

