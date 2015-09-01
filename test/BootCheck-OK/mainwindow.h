#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QTimer>
#include<QDateTime>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_m_pushButtonStart_clicked();

    void on_m_pushButtonEnd_clicked();
    void USBResult();
    void NetworkResult();
    void OnAutoResetSystemTimer();
    void OnUSBTestTimer();
    void OnNetTestTimer();
    void OnShowTimer();
    void setNetworkCheck();
    void setUSBCheck();
    int getSystemBootTimes();
    int IsRunningCheck();

    void on_m_pushButtonReset_clicked();

private:
    Ui::MainWindow *ui;
    QTimer * m_atuoResetTimer;
    QTimer * m_testNetTimer;
    QTimer * m_testUSBTimer;
    QTimer * m_testShowTimer;
    bool  m_bStartTest;
    QProcess* m_myProcessNet;
    QProcess* m_myProcessUSB;
    int m_nBootTimes;


};

#endif // MAINWINDOW_H
