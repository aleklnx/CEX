#include "launcher.h"
#include "ui_launcher.h"
#include <QMessageBox>
#include <QBuffer>
#include "MemoryViewer.h"
#include "socketmanager.h"
#include "MemoryManager/MemoryManager.h"
#include "OperationsManager.h"
#include <QJsonDocument>

#define PORT 5555

Launcher::Launcher(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Launcher),
    localServer(new SocketManager(this)),
    monitor(new MemoryViewer),
    menMan(MemoryManager::getInstance())
{
    ui->setupUi(this);

    connect(menMan, SIGNAL(change(MemoryManager::listNode)), monitor, SLOT(selector(MemoryManager::listNode)));

    localServer->setMaxPendingConnections(1);

    connect(localServer, &SocketManager::receivingData, [&](QByteArray request){

        ui->textEdit_logConsole->appendPlainText("Client: " + QString::fromStdString(request.toStdString()));

        QByteArray responseBytes = OperationsManager::getInstance()->makeRequest(request);

        QString response = QString::fromStdString(responseBytes.toStdString());

        if(localServer->sendMsg(responseBytes)){
            ui->textEdit_logConsole->appendPlainText("Server: " + response);
        }else{
            ui->textEdit_logConsole->appendPlainText("Server: Error sending: " + response);
        }
    });

    connect(localServer, &SocketManager::newClientConnected, [&](){
       ui->textEdit_logConsole->appendPlainText("Server: IDE connected.");
       localServer->sendMsg("CONNECTION SUCCESFUL");
    });

    connect(localServer, &SocketManager::closeAll, [&](){
        monitor->close();
        close();
    });

    monitor->show();
}

Launcher::~Launcher()
{
    delete ui;
}

void Launcher::on_button_startServer_clicked()
{
    if(localServer->isListening()){
        ui->textEdit_logConsole->appendPlainText("Server: Server is already listening.");
        return;
    }
    if(!localServer->startServer(PORT)){
        ui->textEdit_logConsole->appendPlainText("Server: " + localServer->errorString());
    }else{        
        ui->textEdit_logConsole->appendPlainText("Server: Server started in port " + QString::number(PORT));
    }
}

void Launcher::on_button_quitApp_clicked()
{
    monitor->close();
    this->close();
}

void Launcher::on_button_test_clicked()
{
    OperationsManager * op = OperationsManager::getInstance();

    QJsonDocument doc;

    QJsonObject obj;

    obj.insert("OPERATION", "DECLARATION");
    obj.insert("TYPE_TARGET","INT");
    obj.insert("NAME", "number");

    doc.setObject(obj);

    QByteArray response = op->makeRequest(doc.toJson());

    std::cout << response.toStdString() << "SERVER";

    /*if(localServer->sendMsg("TESTING CONNECTION")){
        ui->textEdit_logConsole->appendPlainText("Server: Test sended");
    }else{
        ui->textEdit_logConsole->appendPlainText("Server: Error, socket not ready");
    }*/
}
