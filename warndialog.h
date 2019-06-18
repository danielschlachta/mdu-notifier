#ifndef WARNDIALOG_H
#define WARNDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>

class QLabel;
class QPushButton;

class WarnDialog : public QDialog
{
    Q_OBJECT

public:
    WarnDialog(QWidget *parent, QString message, long timeout);

signals:
    void dismissed();

private slots:
    void dismissClicked();

private:
    QLabel *label1;
    QLabel *label2;
    QPushButton *okButton;
    QPushButton *dismissButton;

    int timerId;

    void timerEvent(QTimerEvent *event) override;
};

#endif // WARNDIALOG_H
