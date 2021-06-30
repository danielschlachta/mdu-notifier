#ifndef ListDialog_H
#define ListDialog_H

#include <QDialog>
#include <QListView>
#include <QPushButton>
#include <QStringListModel>

class QLabel;
class QPushButton;

class ListDialog : public QDialog
{
    Q_OBJECT

public:
    ListDialog(QWidget *parent, QStringList captions, QStringList serials);

signals:
    void selected(QString serial);

private slots:
    void doubleClicked(const QModelIndex &index);
    void dismissClicked();
    void okClicked();

private:
    QListView *listView;
    QPushButton *okButton;
    QPushButton *dismissButton;

    QStringList simSerials;

    void doEmit(const QModelIndex &index);
};

#endif // ListDialog_H
