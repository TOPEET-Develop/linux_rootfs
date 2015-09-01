#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <termios.h>





#define TEST_RUN_FILE   "run.txt"
#define TEST_USB_FILE   "USB.txt"
#define TEST_USB_FILE_ERROR   "USB-Error.txt"
#define TEST_NET_FILE   "NET.txt"
#define TEST_NET_FILE_ERROR   "NET-Error.txt"
#define TEST_BOOT_TIMES  "TIMES.txt"

#define CMD_NET_RESULT  "eth0"
#define CMD_USB_RESULT "Device 005"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_bStartTest=false;
    m_nBootTimes=0;
    m_myProcessNet = NULL;
    m_myProcessUSB = NULL;
    m_atuoResetTimer = NULL;
    m_testUSBTimer =NULL;
    m_testNetTimer =NULL;
    m_testShowTimer = NULL;

    //tf or sd  ,not used
    ui->m_checkTF->setEnabled(false);
    ui->m_nTF->setEnabled(false);



    if(IsRunningCheck())
    {
        ui->m_pushButtonStart->setEnabled(false);
        ui->m_pushButtonReset->setEnabled(false);

        int time=ui->m_nInterval->text().toInt();
        printf("reset system interval is %d sec\n",time);

        m_bStartTest =true;

        m_atuoResetTimer=NULL;
        m_atuoResetTimer=new QTimer(this);
        connect(m_atuoResetTimer,SIGNAL(timeout()),this,SLOT(OnAutoResetSystemTimer()));
        m_atuoResetTimer->start(time*1000);


        m_testNetTimer=NULL;
        m_testNetTimer=new QTimer(this);
        connect(m_testNetTimer,SIGNAL(timeout()),this,SLOT(OnNetTestTimer()));
        m_testNetTimer->start(500);

        m_testUSBTimer=NULL;
        m_testUSBTimer=new QTimer(this);
        connect(m_testUSBTimer,SIGNAL(timeout()),this,SLOT(OnUSBTestTimer()));
        m_testUSBTimer->start(1000);


        m_testShowTimer=NULL;
        m_testShowTimer=new QTimer(this);
        connect(m_testShowTimer,SIGNAL(timeout()),this,SLOT(OnShowTimer()));
        m_testShowTimer->start(5000);


        int bootTimes=getSystemBootTimes();

        if(bootTimes>=0)
        {
         //  QString times(bootTimes);

           char times[255];
           sprintf(times,"%d",bootTimes);
           ui->m_totalTimers->setText(times);
        }

        printf("start board item check......\n");

     //   setNetworkCheck();

      //  usleep(1000);

      //  setUSBCheck();

    }else //no start
    {

        ui->m_pushButtonEnd->setEnabled(false);
        ui->m_pushButtonReset->setEnabled(true);
        ui->m_pushButtonStart->setEnabled(true);


    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_m_pushButtonStart_clicked()
{

    m_bStartTest=true;


    QString cmd_run= "echo \"1\" > ";
    cmd_run += TEST_RUN_FILE;
    system(cmd_run.toUtf8());


    QString cmd_boot= "echo \"0\" > ";
    cmd_boot += TEST_BOOT_TIMES;
    system(cmd_boot.toUtf8());


    if(ui->m_checkEnternet->isChecked())
    {
        QString cmd_net= "echo \"0\" > ";
        cmd_net += TEST_NET_FILE;
        system(cmd_net.toUtf8());

      //  QString cmd_net_error= "echo \"0\" > ";
      //  cmd_net_error += TEST_NET_FILE_ERROR;
      //  system(cmd_net_error.toUtf8());

    }

    if(ui->m_checkUSB->isChecked())
    {
        QString cmd_usb= "echo \"0\" > ";
        cmd_usb += TEST_USB_FILE;
        system(cmd_usb.toUtf8());

     //   QString cmd_usb_error= "echo \"0\" > ";
     //   cmd_usb_error += TEST_USB_FILE_ERROR;
     //   system(cmd_usb_error.toUtf8());

    }

    system("reboot");





}

void MainWindow::on_m_pushButtonEnd_clicked()
{

    m_bStartTest=false;

    QString cmd_run= "echo \"0\" > ";
    cmd_run += TEST_RUN_FILE;
    system(cmd_run.toUtf8());


   m_atuoResetTimer->stop();


   ui->m_pushButtonStart->setEnabled(true);
   ui->m_pushButtonReset->setEnabled(true);


}
void MainWindow::OnAutoResetSystemTimer()
{
    if(!m_bStartTest)
        return;

    printf("reboot system ......\n");
    system("reboot");

}
void MainWindow::USBResult()
{

    QString abc = m_myProcessUSB->readAllStandardOutput();
    QString efg = m_myProcessUSB->readAllStandardError();


    if((abc.indexOf(CMD_USB_RESULT,0)!=-1))
    {

         printf("USB devices is working ...\n");

        char usbContent[255];
        int file= open(TEST_USB_FILE,O_RDWR | O_CREAT);
        if(file==-1)
        {
            perror("usb file  open error!");
        }

        if (read(file, usbContent, 255) < 0){
            perror("usb file  read error");

        }

        ::close(file);

        int usbCheckSuccessTimes=atoi(usbContent);

        usbCheckSuccessTimes++;



        char n_str[255];
        sprintf(n_str, "%d\n", usbCheckSuccessTimes);

        int fd;
        fd = open(TEST_USB_FILE, O_RDWR | O_CREAT);

        if (-1 == fd)
        {
             perror("usb file  open error!");
            return ; //perror(), etc etc
        }

        write(fd, n_str, strlen(n_str)); // pls check return value and do err checking
        ::close(fd);


     //   ui->m_nUSB->setText(n_str);

    }else
    {
        printf("USB devices is not working ...\n");

    //    ui->m_nUSB->setText("Not");

        int file_error= open(TEST_USB_FILE_ERROR,O_RDWR | O_APPEND |O_CREAT );

        if(file_error==-1)
        {
            perror("usb_error file open error!");
        }

        char usbCheckErr[255];
        sprintf(usbCheckErr,"#%d\n",m_nBootTimes);

        write(file_error,usbCheckErr,strlen(usbCheckErr));
        ::close(file_error);

    }


  //  m_myProcessUSB->close();

}

int MainWindow::IsRunningCheck()
{


    char content[2];
    memset(content,0,sizeof(content));

    int bfile=access(TEST_RUN_FILE,F_OK|R_OK|W_OK);
    if(bfile!=-1) //file exist and permission
    {
        int file= open(TEST_RUN_FILE,O_RDONLY);
        if(file==-1)
        {
            perror("file open error!");
        }

        if (read(file, content, 32) < 0)
        {
            perror("read error");

        }
        ::close(file);


        int nStart=atoi(content);

        return nStart;

    }

    return 0;

}



int MainWindow::getSystemBootTimes()
{


    //boot times +
    char bootBuf[10];
    FILE* bootfile = fopen(TEST_BOOT_TIMES,"rw");
    if(bootfile==NULL)
    {
        perror("file open error!");
    }

    if (fread(bootBuf,1,10,bootfile) < 0)
    {
        perror("read error");

    }

    fclose(bootfile);

    int bootTimes=atoi(bootBuf);
    bootTimes++;

    printf("boot times is %d\n",bootTimes);

    m_nBootTimes=bootTimes;

    char bootTimesBuff[10];
    memset(bootTimesBuff,0,sizeof(bootTimesBuff));

  //  sprintf(bootTimesBuff,"echo  -n %d > TIMES.txt",bootTimes);
  //  printf("write boot times %s to file\n",bootTimesBuff);
  //  system(bootTimesBuff);
    //fwrite(bootTimesBuff,1,strlen(bootTimesBuff),bootfile);
    //fflush(bootfile);
 //   fclose(bootfile);


    char n_str[16];
    sprintf(n_str, "%d", bootTimes);

    int fd;
    fd = open(TEST_BOOT_TIMES, O_RDWR | O_CREAT);

    if (-1 == fd)
        return -1; //perror(), etc etc

    write(fd, n_str, strlen(n_str)); // pls check return value and do err checking
    ::close(fd);


    printf("write boot times to TIMES.txt\n");

    return bootTimes;


}



void MainWindow::setNetworkCheck()
{

    int bfile=access(TEST_NET_FILE,F_OK|R_OK|W_OK);
    if(bfile!=-1) //file exist and permission
    {
        ui->m_checkEnternet->setChecked(true);

        QString cmd ;
        cmd +="ifconfig";
        if(m_myProcessNet)
        {
            delete m_myProcessNet;
            m_myProcessNet =NULL;
        }

        m_myProcessNet = new QProcess(this);
        connect(m_myProcessNet, SIGNAL(readyReadStandardOutput()),this, SLOT(NetworkResult()));
        connect(m_myProcessNet, SIGNAL(readyReadStandardError()),this, SLOT(NetworkResult()));
        m_myProcessNet->start(cmd);
        printf("start netcheck......\n");
    }

}

void MainWindow::NetworkResult()
{
    //net check

    QString abc = m_myProcessNet->readAllStandardOutput();
    QString efg = m_myProcessNet->readAllStandardError();

    if((abc.indexOf(CMD_NET_RESULT,0)!=-1))
    {
        printf("Enthernet is working ...\n");

        char netContent[255];
        int file= open(TEST_NET_FILE,O_RDWR);
        if(file==-1)
        {
            perror("network file open error!");
        }

        if (read(file, netContent, 255) < 0){
            perror("network file  read error");

        }

        ::close(file);

        int netTimes=atoi(netContent);

        netTimes++;


        char n_str[255];
        sprintf(n_str, "%d\n", netTimes);

        int fd;
        fd = open(TEST_NET_FILE, O_RDWR | O_CREAT);

        if (-1 == fd)
        {
            perror("network file open error!");
            return ; //perror(), etc etc
        }

        write(fd, n_str, strlen(n_str)); // pls check return value and do err checking
        ::close(fd);

   //     ui->m_nEnternet->setText(n_str);


    }else
    {
         printf("Enthernet is not working ...\n");
    //     ui->m_nEnternet->setText("Not");
        int file_error= open(TEST_NET_FILE_ERROR,O_RDWR | O_APPEND |O_CREAT);

        if(file_error==-1)
        {
            perror("network_error file  open error!");
        }

        char buf[255];
        memset(buf,0,sizeof(buf));
        sprintf(buf,"#%d\n",m_nBootTimes);

        write(file_error,buf,sizeof(buf));
        ::close(file_error);

    }

   // m_myProcessNet->terminate();
//    m_myProcessNet->close();
 //   m_myProcessNet->waitForFinished();

}

void MainWindow::setUSBCheck()
{
    //usb check
    int bfile=access(TEST_USB_FILE,F_OK|R_OK|W_OK);
    if(bfile!=-1) //file exist and permission
    {
        ui->m_checkUSB->setChecked(true);

        QString cmd ;
        cmd +="lsusb\r\n";
        if(m_myProcessUSB)
        {
            delete m_myProcessUSB;
            m_myProcessUSB =NULL;
        }

        m_myProcessUSB = new QProcess(this);
        connect(m_myProcessUSB, SIGNAL(readyReadStandardOutput()),this, SLOT(USBResult()));
        connect(m_myProcessUSB, SIGNAL(readyReadStandardError()),this, SLOT(USBResult()));
        m_myProcessUSB->start(cmd);

        printf("start usb check.....\n");
    }

}


void MainWindow::on_m_pushButtonReset_clicked()
{

     m_nBootTimes=0;
     ui->m_checkUSB->setChecked(false);
     ui->m_checkEnternet->setChecked(false);

     ui->m_nEnternet->setText("0");
     ui->m_nUSB->setText("0");
     ui->m_totalTimers->setText("0");


    ::system("rm ./*.txt");

}

void MainWindow::OnNetTestTimer()
{

   //stop timer
    m_testNetTimer->stop();


    //start  netcheck only one time
    setNetworkCheck();


}


void MainWindow::OnUSBTestTimer()
{

    //stop timer
     m_testUSBTimer->stop();


     //start usb check
      setUSBCheck();



}


//show thread

void MainWindow::OnShowTimer()
{

    int bfile=access(TEST_NET_FILE,F_OK|R_OK|W_OK);
    if(bfile!=-1) //file exist and permission
    {

        char netContent[255];
        FILE * file= fopen(TEST_NET_FILE,"r");
        if(file==NULL)
        {
            perror("network file open error!");
        }

      //  if (read(file, netContent, 255) < 0){
        //    perror("network file  read error");

        //}


        fgets(netContent,255,file);

        fclose(file);

        int netTimes=atoi(netContent);
        char n_str[255];
        sprintf(n_str, "%d", netTimes);

        printf("Network %s  times work well\n",n_str);


        ui->m_nEnternet->setText(n_str);

    }


    bfile=access(TEST_USB_FILE,F_OK|R_OK|W_OK);
    if(bfile!=-1) //file exist and permission
    {

        char usbContent[255];
        FILE * file= fopen(TEST_USB_FILE,"r");
        if(file==NULL)
        {
            perror("usb file  open error!");
        }

       // if (read(file, usbContent, 255) < 0){
         //   perror("usb file  read error");

        //}

        fgets(usbContent,255,file);


        fclose(file);

        int usbCheckSuccessTimes=atoi(usbContent);

        char n_str[255];
        sprintf(n_str, "%d", usbCheckSuccessTimes);
        printf("USB  %s  times work well\n",n_str);

        ui->m_nUSB->setText(n_str);

    }


    m_testShowTimer->stop();

}











