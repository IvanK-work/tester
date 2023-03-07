#ifndef MAINWINDOWTESTER_H
#define MAINWINDOWTESTER_H

#include <QMainWindow>
#include <QMap>
#include <QSettings>

class QListWidgetItem;
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindowTester; }
QT_END_NAMESPACE

struct test_case{
    int id;
    QString in;
    QString out;
};

struct task_info{
    QString name;
    QString text;
    QVector<test_case> test_cases;
};

class MainWindowTester : public QMainWindow
{
    Q_OBJECT

public:
    MainWindowTester(QWidget *parent = nullptr);
    ~MainWindowTester();

    void fill_data_from_settings();

    void hide(bool yes);

private slots:
    void on_pbAddTask_clicked();

    void on_listTests_itemChanged(QListWidgetItem *item);

    void on_pbAddCase_clicked();

    void on_pbRemoveCase_clicked();

    void on_listTests_itemClicked(QListWidgetItem *item);

    void on_tbTextTask_textChanged();


    void on_pbRemoveTask_clicked();

    void on_pbChoosePathPy_clicked();

    void on_pbChoosePathPascal_clicked();

    void on_pbChoosePathC_clicked();

    void on_lePythonPath_textChanged(const QString &file);

    void on_lePascalPath_textChanged(const QString &arg1);

    void on_leCpath_textChanged(const QString &arg1);

    void on_pbCheckTask_clicked();

    void on_pbClearConsole_clicked();

    void on_pbClearCode_clicked();

    void on_pbExport_clicked();

    void on_pbImport_clicked();

    void on_pb_about_clicked();

    void on_tbSourceCodeTask_textChanged();

    void on_rbCpp_toggled(bool checked);

    void on_rbPascal_toggled(bool checked);

    void on_rbPy_toggled(bool checked);

private:
    Ui::MainWindowTester *ui;
    QMap<QListWidgetItem*,task_info> tasks;
    QSettings *settings;
    QString python,c,pascal;
    void ini_save();
    QString extension;

};
#endif // MAINWINDOWTESTER_H
