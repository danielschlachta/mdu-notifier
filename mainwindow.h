#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon *trayIcon;

    QMenu *trayIconMenu;
    QAction *settingsAction;
    QAction *quitAction;

    void updateTrayIcon(int percent, bool active);
    void closeEvent(QCloseEvent *event) override;

private slots:
    void iconMessageClicked();
};

#endif // MAINWINDOW_H
