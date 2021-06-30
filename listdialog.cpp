#include <QtGui>
#include <QLayout>
#include <QStringListModel>

#include "common.h"
#include "listdialog.h"

ListDialog::ListDialog(QWidget *parent, QStringList captions, QStringList serials) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{
    simSerials = serials;

    QStringListModel *model = new QStringListModel(this);
    model->setStringList(captions);

    listView = new QListView();
    listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    listView->setModel(model);

    connect(listView, &QListView::doubleClicked, this, &ListDialog::doubleClicked);

    okButton = new QPushButton(tr("&Ok"));
    okButton->setDefault(true);

    dismissButton = new QPushButton(tr("&Dismiss"));

    connect(okButton, SIGNAL(clicked()), this, SLOT(okClicked()));
    connect(dismissButton, SIGNAL(clicked()), this, SLOT(dismissClicked()));

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(okButton);
    bottomLayout->addWidget(dismissButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(listView);
    mainLayout->addLayout(bottomLayout);
    setLayout(mainLayout);

    setWindowTitle(tr(APPTITLE));
    setFixedHeight(sizeHint().height());
}

void ListDialog::doEmit(const QModelIndex &index)
{
   emit selected(simSerials.value(index.row()));
}

void ListDialog::doubleClicked(const QModelIndex &index)
{
    close();
    doEmit(index);
}

void ListDialog::dismissClicked()
{
   close();
}

void ListDialog::okClicked()
{
    close();
    QModelIndex index = listView->currentIndex();
    doEmit(index);
}

