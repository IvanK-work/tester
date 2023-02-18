#include "mainwindowtester.h"
#include "./ui_mainwindowtester.h"
#include <QDebug>
#include <QScrollBar>
#include <QCheckBox>
#include <QLabel>
#include <QTimer>
#include <QFileDialog>
#include <QProcess>
#include <QtGlobal>
#include <qglobal.h>
#include <QDir>
#include <QMessageBox>

void MainWindowTester::fill_data_from_settings()
{
    auto groups = settings->childGroups();
    for(const auto &t  : qAsConst(groups)){
        if(t == "general_settings"){
            continue;
        }
        settings->beginGroup(t);
        for(const auto &j : settings->childGroups()){
            auto item = new QListWidgetItem();
            item->setFlags(item->flags() | Qt::ItemIsEditable);
            auto nm = settings->value("name").toString();
            item->setText(nm);
            tasks[item].name = item->text();
            tasks[item].text = settings->value("text").toString();
            settings->beginGroup("cases");
            auto cases = settings->childGroups();
            for(const auto &c : qAsConst(cases)) {
                settings->beginGroup(c);
                test_case tc;
                tc.id = settings->value("id").toInt();
                tc.in = settings->value("in").toString();
                tc.out = settings->value("out").toString();
                tasks[item].test_cases.push_back(tc);
                settings->endGroup();
            }
            settings->endGroup();
        }
        settings->endGroup();
    }

    int row=0;
    auto keys_ = tasks.keys();
    for(auto &i : keys_)
    {
        ui->listCurrentTaskTest->insertItem(row,new QListWidgetItem(i->text()));
        ui->listTests->insertItem(row++,i);
    }
}

MainWindowTester::MainWindowTester(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindowTester)
{
    ui->setupUi(this);
    ui->data_in->setTextBackgroundColor(Qt::lightGray);
    ui->data_out->setTextBackgroundColor(Qt::lightGray);

    settings = new QSettings("tester_settings.ini",QSettings::Format::IniFormat,this);

    ui->lePythonPath->setText(settings->value("general_settings/python").toString());
    ui->leCpath->setText(settings->value("general_settings/c").toString());
    ui->lePascalPath->setText(settings->value("general_settings/pascal").toString());

    fill_data_from_settings();
}

MainWindowTester::~MainWindowTester()
{
    settings->sync();
    delete ui;
}


void MainWindowTester::on_pbAddTask_clicked()
{
    auto it = new QListWidgetItem("Новая задача");
    it->setFlags(it->flags() | Qt::ItemIsEditable);
    ui->listTests->addItem(it);
    it->setSelected(true);

    tasks.insert(it,{});
}


void MainWindowTester::on_listTests_itemChanged(QListWidgetItem *item)
{
    tasks[item].name = item->text();
    ui->listCurrentTaskTest->clear();

    int row=0;
    auto keys_ = tasks.keys();
    for(auto &i : keys_)
    {
        ui->listCurrentTaskTest->insertItem(row++,new QListWidgetItem(i->text()));

    }
    ini_save();
}


void MainWindowTester::on_pbAddCase_clicked()
{

    if(ui->data_in->toPlainText().isEmpty()){
        ui->statusbar->showMessage("Входные данные пустые",3000);
        return;
    }
    if(ui->data_out->toPlainText().isEmpty()){
        ui->statusbar->showMessage("Выходные данные пустые",3000);
        return;
    }

    auto cb = new QLabel("",this);
    auto tin = new QTextBrowser(this);
    auto tout = new QTextBrowser(this);

    auto in = ui->data_in->toPlainText();
    auto out = ui->data_out->toPlainText();
    if(in.back() != '\n'){
        in.append('\n');
    }
    if(out.back() != '\n'){
        out.append('\n');
    }
    tin->setPlainText(in);
    tout->setPlainText(out);

    ui->data_in->clear();
    ui->data_out->clear();

    auto l = (QGridLayout*)ui->currentTestsScrollArea->layout();

    int r = l->count()/l->columnCount();

    cb->setText(QString::number(r));
    l->addWidget(cb,r,0);
    l->addWidget(tin,r,1);
    l->addWidget(tout,r,2);
    QTimer::singleShot(10,this,[&](){    ui->scrollArea->verticalScrollBar()->setValue(ui->scrollArea->verticalScrollBar()->maximum());});

    {
        auto listSel = ui->listTests->selectedItems();
        if(!listSel.empty()){
            auto it = *listSel.begin();
            tasks[it].test_cases.push_back({r,
                                            in,
                                            out});
        }
    }
    ini_save();
}

void MainWindowTester::on_pbRemoveCase_clicked()
{
    // remove last case
    auto l = (QGridLayout*)ui->currentTestsScrollArea->layout();
    int max=0;
    for(int i=0; i < l->rowCount(); ++i){
        auto it_cb = l->itemAtPosition(i,0);
        if(it_cb == nullptr)
            continue;
        auto cb = static_cast<QLabel*>(it_cb->widget());
        max = std::max(max,cb->text().toInt());
    }


    for(int i=0; i < l->rowCount(); ++i){
        auto it_cb = l->itemAtPosition(i,0);
        auto it_in = l->itemAtPosition(i,1);
        auto it_out = l->itemAtPosition(i,2);
        if(it_cb == nullptr)
            continue;
        auto cb = static_cast<QLabel*>(it_cb->widget());

        if(cb->text().toInt()== max){

            it_cb->widget()->deleteLater();
            it_in->widget()->deleteLater();
            it_out->widget()->deleteLater();
            l->removeItem(it_cb);
            l->removeItem(it_in);
            l->removeItem(it_out);

        }
    }

    {
        auto listSel = ui->listTests->selectedItems();
        if(!listSel.empty()){
            auto it = *listSel.begin();
            tasks[it].test_cases.pop_back();
        }
    }
    ini_save();
}


void MainWindowTester::on_listTests_itemClicked(QListWidgetItem *item)
{
    static QListWidgetItem* prev_it=nullptr;
    if(prev_it == item)
        return;

    prev_it = item;
    auto &task = tasks[item];
    ui->tbTextTask->setPlainText(task.text);

    auto l = (QGridLayout*)ui->currentTestsScrollArea->layout();
    for(int i=0; i < l->rowCount(); ++i){
        auto it_cb = l->itemAtPosition(i,0);
        auto it_in = l->itemAtPosition(i,1);
        auto it_out = l->itemAtPosition(i,2);
        if(it_cb == nullptr)
            continue;
        it_cb->widget()->deleteLater();
        it_in->widget()->deleteLater();
        it_out->widget()->deleteLater();
        l->removeItem(it_cb);
        l->removeItem(it_in);
        l->removeItem(it_out);
    }

    for(auto &i : task.test_cases){
        int r = l->count()/l->columnCount();

        auto cb = new QLabel("",this);
        auto tin = new QTextBrowser(this);
        auto tout = new QTextBrowser(this);

        cb->setText(QString::number(i.id));
        tin->setPlainText(i.in);
        tout->setPlainText(i.out);
        l->addWidget(cb,r,0);
        l->addWidget(tin,r,1);
        l->addWidget(tout,r,2);
    }

    QTimer::singleShot(10,this,[&](){    ui->scrollArea->verticalScrollBar()->setValue(ui->scrollArea->verticalScrollBar()->maximum());});
}

void MainWindowTester::on_tbTextTask_textChanged()
{
    auto l = ui->listTests->selectedItems();
    if(!l.empty()){
        auto it = *l.begin();
        tasks[it].text = ui->tbTextTask->toPlainText();
        ini_save();
    }
}

void MainWindowTester::on_pbRemoveTask_clicked()
{
    auto l = ui->listTests->selectedItems();
    if(!l.empty()){
        auto it = *l.begin();

        auto tit = ui->listCurrentTaskTest->findItems(it->text(),Qt::MatchExactly);
        if(!tit.empty()){
            auto it2 = *tit.begin();
            auto pp = ui->listCurrentTaskTest->takeItem(ui->listCurrentTaskTest->row(it2));
            delete pp;
        }

        tasks.remove(it);
        auto p=ui->listTests->takeItem(ui->listTests->row(it));
        delete p;
    }
    ini_save();
}

void MainWindowTester::ini_save()
{
    settings->clear();
    for(  const auto &i : qAsConst(tasks)){
        if(i.name.isEmpty())
            continue;
        settings->beginGroup(i.name);
        settings->setValue("text",i.text);
        settings->setValue("name",i.name);
        {
            settings->beginGroup("cases");
            for(const auto &j : qAsConst(i.test_cases))
            {
                settings->beginGroup("case" + QString::number(j.id));

                settings->setValue("id",j.id);
                settings->setValue("in",j.in);
                settings->setValue("out",j.out);

                settings->endGroup();
            }
            settings->endGroup();
        }

        settings->endGroup();
    }
    settings->sync();
}


void MainWindowTester::on_pbChoosePathPy_clicked()
{
    QString file= QFileDialog::getOpenFileName(this,"Укажите файл интерпретатора Python");
    ui->lePythonPath->setText(file);

    QProcess *s = new QProcess(this);
    s->start(file,{"--version"});
    connect(s,static_cast<void (QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished),this,[this,s](){
        ui->lblPythonVer->setText(s->readAllStandardOutput());
    });
    settings->setValue("general_settings/python",file);
}


void MainWindowTester::on_pbChoosePathPascal_clicked()
{
    QString file= QFileDialog::getOpenFileName(this,"Укажите файл компилятора Pascal");
    ui->lePascalPath->setText(file);
}


void MainWindowTester::on_pbChoosePathC_clicked()
{
    QString file= QFileDialog::getOpenFileName(this,"Укажите путь к компилятору Cи");
    ui->leCpath->setText(file);
}


void MainWindowTester::on_lePythonPath_textChanged(const QString &file)
{
    python = file;
    QProcess *s = new QProcess(this);
    s->start(file,{"--version"});
    connect(s,static_cast<void (QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished),this,[this,s](){
        ui->lblPythonVer->setText(s->readAllStandardOutput());
        ui->lblPythonVer->setStyleSheet("QLabel { color : green; }");
    });
    connect(s,&QProcess::errorOccurred,this,[this,s](QProcess::ProcessError error){
        ui->lblPythonVer->setText(s->errorString());
        ui->lblPythonVer->setStyleSheet("QLabel { color : red; }");
        python.clear();
    });
    settings->setValue("general_settings/python",file);
}


void MainWindowTester::on_lePascalPath_textChanged(const QString &file)
{
    pascal=file;
    QProcess *s = new QProcess(this);
    s->start(file,{"--version"});
    connect(s,static_cast<void (QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished),this,[this,s](){
        ui->lblPascalVer->setText(s->readAllStandardOutput() + s->readAllStandardError());
    });
    connect(s,&QProcess::errorOccurred,this,[this,s](QProcess::ProcessError error){
        ui->lblPascalVer->setText(s->errorString());
        pascal.clear();
    });
    settings->setValue("general_settings/pascal",file);
}


void MainWindowTester::on_leCpath_textChanged(const QString &file)
{
    c = file;
    QProcess *s = new QProcess(this);
    s->start(file,{"--version"});
    connect(s,static_cast<void (QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished),this,[this,s](){
        ui->lblCver->setText(s->readAllStandardOutput()  + s->readAllStandardError());
        ui->lblCver->setStyleSheet("QLabel { color : green; }");
    });
    connect(s,&QProcess::errorOccurred,this,[this,s](QProcess::ProcessError error){
        ui->lblCver->setText(s->errorString());
        ui->lblCver->setStyleSheet("QLabel { color : red; }");
        c.clear();
    });
    settings->setValue("general_settings/c",file);
}


void MainWindowTester::on_pbCheckTask_clicked()
{
    ui->tbConsole->clear();
    QString extension;
    auto source_code = ui->tbSourceCodeTask->toPlainText();
    if(source_code.isEmpty()){
        ui->statusbar->showMessage("Исходный код задачи пуст",3000);
        return;
    }
    if(source_code.contains("#include")){
        extension=".cpp";
    }
    else if(source_code.contains("begin") &&source_code.contains("end") && source_code.contains("program") && source_code.contains("var")){
        extension=".pp";
    }else{
        extension=".py";
    }

    QFile file_task(QDir::tempPath()+QDir::separator()+"temptask"+extension);

    if(file_task.open(QIODevice::WriteOnly) == false){
        ui->statusbar->showMessage("Не могу создать временный файл в " + file_task.fileName() + " ошибка: " + file_task.errorString(),10000);
    }

    file_task.write(ui->tbSourceCodeTask->toPlainText().toUtf8());
    file_task.waitForBytesWritten(10000);
    file_task.close();

    QString compiler;

    {
        if(extension == ".py"){
            compiler = python;
            if(compiler.isEmpty()){
                ui->statusbar->showMessage("Не указан интерпретатор Python");
                QMessageBox::StandardButton reply;
                reply = QMessageBox::information(this, "Ошибка",
                                              "Не указан путь до интерпретатора Python",
                                              QMessageBox::Ok);
                ui->tabWidget->setCurrentIndex(0);
                return;
            }
        }else if(extension == ".cpp") {
            compiler = c;
            if(compiler.isEmpty()){
                ui->statusbar->showMessage("Не указан компилятор С/С++");
                QMessageBox::StandardButton reply;
                reply = QMessageBox::information(this, "Ошибка",
                                              "Не указан путь до компилятора С/С++",
                                              QMessageBox::Ok);
                ui->tabWidget->setCurrentIndex(0);
                return;
            }
             QProcess *s = new QProcess(this);
             bool testpas=true;
             connect(s,static_cast<void (QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished),this,[this,s,&testpas](int exitCode, QProcess::ExitStatus exitStatus){
                auto r = s->readAllStandardError();
                if(!r.isEmpty()){
                    ui->tbConsole->append("Ошибка компиляции: " + r);
                    testpas=false;
                }
             });
             s->start(c,{file_task.fileName(),"-o",QDir::tempPath()+QDir::separator()+"a.out"});
             if(!s->waitForFinished(10000)){
                 ui->tbConsole->append("Ошибка компиляции: " + s->errorString());
                 return;
             }
             if(!testpas)
                 return;
             compiler=QDir::tempPath()+QDir::separator()+"a.out";
        }else if(extension == ".pp") {
            compiler = pascal;
        }else{
            ui->statusbar->showMessage("Не определен язык программирования",5000);
            return;
        }
    }

    task_info cases;

    {
        auto t = ui->listCurrentTaskTest->selectedItems();
        if(t.empty()){
            ui->statusbar->showMessage("Не выбрана задача по которой нужно проверять ", 5000);
            return;
        }
        auto task = *t.begin();
        for(auto &i : tasks){
            if(i.name == task->text()){
                cases =i ;
            }
        }
        if(cases.name.isEmpty()){
            ui->statusbar->showMessage("Неизвестная задача ", 5000);
            return;
        }
    }

bool testpass=true;

    for(auto &i : cases.test_cases){
        ui->tbConsole->append("\n\n  <font color=\"Orange\">Начинаю проверять тест №" + QString::number(i.id)+ "</font>");
        QProcess *s = new QProcess(this);


        connect(s,&QProcess::started,this,[this,i,s](){
            ui->tbConsole->append("Программа запустилась, подаю на вход данные: \n" + i.in.toUtf8());
            s->write(i.in.toUtf8()+'\n');
        });


        connect(s, &QProcess::readyRead,this,[this,i,s,&testpass](){
            ui->tbConsole->append("Поступил ответ от программы ");
            auto result = s->readAllStandardOutput();
            if(result.isEmpty()){
                auto er = s->readAllStandardError();
                ui->tbConsole->append("<font color=\"Red\">Тест №" + QString::number(i.id)
                                      + " Ошибка runtime error " + er+ "</font>");
                return;
            }
            if(result == i.out || result == i.out + '\n'){
                ui->tbConsole->append("<font color=\"Green\">Тест №" + QString::number(i.id) + " пройден\n</font>" );
            }else{
                testpass=false;
                ui->tbConsole->append("<font color=\"Red\">Тест №" + QString::number(i.id) + " Ошибка  получен результат " + result
                                      + " ожидаемый результат " + i.out+ "</font>");
            }
        });



        connect(s,static_cast<void (QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished),this,[this,s,i,&testpass]
                (int exitCode, QProcess::ExitStatus exitStatus){
            ui->tbConsole->append("Программа завершена\n\n");
            auto er = s->readAllStandardError();
            if(!er.isEmpty()){
             testpass=false;
                ui->tbConsole->append("<font color=\"Red\">Тест №" + QString::number(i.id)
                                      + " Ошибка runtime error " + er+ "</font>");
            }

        });

        connect(s,&QProcess::errorOccurred,this,[this,s,i,&testpass](QProcess::ProcessError error){
            testpass=false;
            ui->tbConsole->append("<font color=\"Red\">Тест № " + QString::number(i.id) + " Ошибка runtime error " + s->errorString()+ "</font>");
        });

        s->start(compiler,{file_task.fileName()});
        QCoreApplication::processEvents(QEventLoop::AllEvents,4000);
        if(!s->waitForFinished(4000)){
            s->kill();
            ui->tbConsole->append("<font color=\"Red\">Тест №" + QString::number(i.id) + " Ошибка, слишком долго выполняется </font>");
        }else{
            bool delay=true;
            QTimer::singleShot(500,this,[&delay](){
                delay=false;
            });
            while(delay)
             QCoreApplication::processEvents(QEventLoop::AllEvents,5000);

        }
        s->deleteLater();
    }

    if(testpass){
        ui->tbConsole->append("<font color=\"Green\">Все тесты успешно пройдены</font>");
    }else{
        ui->tbConsole->append("<font color=\"Red\">Программа не прошла проверку</font>");
    }

    QTimer::singleShot(10,this,[&](){    ui->tbConsole->verticalScrollBar()->setValue(ui->tbConsole->verticalScrollBar()->maximum());});
}


void MainWindowTester::on_pbClearConsole_clicked()
{
    ui->tbConsole->clear();
}


void MainWindowTester::on_pbClearCode_clicked()
{
    ui->tbSourceCodeTask->clear();
}

void copySettings( QSettings &dst, QSettings &src )
{
    QStringList keys = src.allKeys();
    for( QStringList::iterator i = keys.begin(); i != keys.end(); i++ )
    {
        if(*i == "general_settings")
            continue;
        dst.setValue( *i, src.value( *i ) );
    }
}
void MainWindowTester::on_pbExport_clicked()
{
      QString file= QFileDialog::getSaveFileName(this,"Укажите куда сохранить файл");

      auto new_set = new QSettings(file,QSettings::Format::IniFormat,this);
      new_set->clear();
      copySettings(*new_set, *settings);

      new_set->sync();
      new_set->deleteLater();
}


void MainWindowTester::on_pbImport_clicked()
{
      QString file= QFileDialog::getOpenFileName(this,"Укажите файл компилятора Pascal");
      auto new_set = new QSettings(file,QSettings::Format::IniFormat,this);

      copySettings(*settings, *new_set);
      ui->listCurrentTaskTest->clear();
      ui->listTests->clear();
      tasks.clear();
      fill_data_from_settings();
}


void MainWindowTester::on_pb_about_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::information(this, "О программе",
                                  "Программа для проверки задач по программированию\n Версия: " + QString(tester_VERSION)
                                     + "\n Email разработчика: ivan.ku.work@gmail.com",
                                  QMessageBox::Ok);
}

