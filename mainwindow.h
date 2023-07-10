#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QStandardItemModel;
class QAction;
class QStandardItem;
class QFileDialog;

namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void updateRow(QStringList content);
    void deleteRow(QString name);
    QString createNewName();
    QIcon iconFromExe(QString exe);

private slots:
    void slotOnNewAction();
    void slotOnItemChanged(QStandardItem* item);
    void slotOnDelAction();

private:
    Ui::MainWindow *ui;
    QStandardItemModel *model;
    QAction *newAction;
    QAction *delAction;
    QAction* openPathAction;
    QFileDialog* fileDialog;
    int exepathRow;
};
#endif // MAINWINDOW_H
