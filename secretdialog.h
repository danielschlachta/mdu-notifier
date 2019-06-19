#ifndef SECRETDIALOG_H
#define SECRETDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class QLabel;
class QPushButton;

class SecretDialog : public QDialog
{
    Q_OBJECT

public:
    SecretDialog(QWidget *parent, QString initialValue);

signals:
    void secretChanged(QString secret);

private slots:
    void okClicked();

private:
    QLabel *label;
    QLineEdit *lineEdit;
    QPushButton *okButton;
    QPushButton *cancelButton;
};

#endif // SECRETDIALOG_H
