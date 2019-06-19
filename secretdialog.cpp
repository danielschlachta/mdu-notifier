#include <QtGui>
#include <QLayout>

#include "common.h"
#include "secretdialog.h"

SecretDialog::SecretDialog(QWidget *parent, QString initialValue) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{
    label = new QLabel(tr("Secret"));
    lineEdit = new QLineEdit(initialValue);

    okButton = new QPushButton(tr("&Ok"));
    okButton->setDefault(true);

    cancelButton = new QPushButton(tr("&Cancel"));

    connect(okButton, SIGNAL(clicked()), this, SLOT(okClicked()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(label);
    topLayout->addWidget(lineEdit);

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addStretch();
    bottomLayout->addWidget(okButton);
    bottomLayout->addWidget(cancelButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(bottomLayout);
    setLayout(mainLayout);

    setWindowTitle(tr(APPTITLE));
    setFixedHeight(sizeHint().height());
}

void SecretDialog::okClicked()
{
   close();
   emit secretChanged(lineEdit->text());
}


