#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "pubheader.h"
#include "database.h"

#ifdef WIN32
#include "windows.h"
#include <QtWinExtras/QtWin>
#endif

enum Col {
    IdCol = 0,
    IconCol,
    NameCol,
    EnNameCol,
    DescribeCol,
    ExePathCol,
    CommentCol,
    ColNum
};

static const int MaxNonameNum = 100;
static const QColor ValidExepathColor = QColor(255, 255, 255);
static const QColor InvalidExepathColor = QColor(170, 0, 0, 160);

const QMap<int, QString> ColNO2ColNameMap {
    std::make_pair(IdCol, "id"),
    std::make_pair(IconCol, "icon"),
    std::make_pair(NameCol, "name"),
    std::make_pair(EnNameCol, "enname"),
    std::make_pair(DescribeCol, "describe"),
    std::make_pair(ExePathCol, "exepath"),
    std::make_pair(CommentCol, "comment")
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    exepathRow(-1)
{
    // ui //////////////////////////////////////////////////////////////////
    ui->setupUi(this);
    setWindowTitle("Game Explorer");
    // 表头
    QStringList headerLabels;
    headerLabels << "id" << "图标" << "名字" << "英文名" << "简介" << "执行文件" << "备注";
    ui->tableView->setModel(model = new QStandardItemModel(this));
    model->setHorizontalHeaderLabels(headerLabels);
    ui->tableView->hideColumn(IdCol);
//    ui->tableView->hideColumn(IconCol);
    ui->tableView->setContextMenuPolicy(Qt::ContextMenuPolicy::ActionsContextMenu);
    ui->tableView->addAction(newAction = new QAction("新增"));
    ui->tableView->addAction(delAction = new QAction("删除"));
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->setSortingEnabled(true);
    ui->tableView->setColumnWidth(ExePathCol, 800);
    ui->tableView->setEditTriggers(QTableView::EditTrigger::NoEditTriggers);
    showMaximized();
    // 进度条
    QProgressBar* progress = new QProgressBar(this);
    progress->setWindowFlag(Qt::WindowType::Window);
    progress->setWindowFlag(Qt::WindowType::WindowDoesNotAcceptFocus);
    progress->setWindowFlag(Qt::WindowType::FramelessWindowHint);
    progress->setWindowModality(Qt::WindowModality::WindowModal);
    progress->setValue(0);
    if (Database::instance()->rowCount() > 0)
        progress->show();
    // 文件选择窗口
    fileDialog = new QFileDialog(this);
    fileDialog->setModal(true);
    fileDialog->setDirectory("/");


//    for (int i=0;i<900;++i)
//        slotOnNewAction();

    // 连接信号槽 //////////////////////////////////////////////////////////////////
    // 新建
    connect(newAction, &QAction::triggered, this, &MainWindow::slotOnNewAction);
    // 删除
    connect(delAction, &QAction::triggered, this, &MainWindow::slotOnDelAction);
    // 更改
    connect(model, &QStandardItemModel::itemChanged, this, &MainWindow::slotOnItemChanged);
    // 从数据库加载数据
    connect(Database::instance(), &Database::dataLoaded, this, [=](const Data& d) {
        static const double rowCount = Database::instance()->rowCount();
        static double n = 0;
        n++;
        progress->setValue(std::ceil(n/rowCount));
        updateRow(QStringList{d.id, "", d.name, d.enname, d.describe, d.exepath, d.comment});
        if (n == rowCount)
            progress->close();
    });
    // 表格双击
    connect(ui->tableView, &QTableView::doubleClicked, this, [=](const QModelIndex &index) {
        switch (index.column()) {
        case IconCol: {
            QString exepath = model->item(index.row(), ExePathCol)->text();
            QProcess process(this);
            process.startDetached(exepath);
            break;
        }
        case NameCol:
        case EnNameCol:
        case DescribeCol:
        case CommentCol: {
            ui->tableView->edit(index);
            break;
        }
        case ExePathCol: {
            exepathRow = index.row();
            fileDialog->show();
            break;
        }
        default:
            break;
        }
    });
    // 选择可执行文件路径
    connect(fileDialog, &QFileDialog::filesSelected, this, [=](const QStringList &selected) {
        if (selected.size() < 1)
            return;
        model->item(exepathRow, ExePathCol)->setText(selected.front());
    });

    Database::instance()->loadAllData();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateRow(QStringList content)
{
    if (content.size() != ColNum) {
        QMessageBox::information(this, "", "实际输入数据的列数和表格的列数不一致。");
        return;
    }
    QList<QStandardItem*> items;
    items.append(new QStandardItem(content[IdCol]));
    QStandardItem* iconitem = new QStandardItem();
//    iconitem->setIcon(iconFromExe(content[ExePathCol]));
    iconitem->setText("开始游戏");
    if (!QFile(content[ExePathCol]).exists())
        iconitem->setBackground(QBrush(InvalidExepathColor));
    else
        iconitem->setBackground(QBrush(ValidExepathColor));
    items.append(iconitem);
    QStandardItem* nameitem = new QStandardItem();
    nameitem->setIcon(iconFromExe(content[ExePathCol]));
    nameitem->setText(content[NameCol]);
    items.append(nameitem);
    items.append(new QStandardItem(content[EnNameCol]));
    items.append(new QStandardItem(content[DescribeCol]));
    items.append(new QStandardItem(content[ExePathCol]));
    items.append(new QStandardItem(content[CommentCol]));
    model->appendRow(items);
}

void MainWindow::deleteRow(QString name)
{
    QList<QStandardItem*> items = model->findItems(name, Qt::MatchExactly, NameCol);
    for (QStandardItem* item : items) {
        model->removeRow(item->index().row());
    }
}

QString MainWindow::createNewName()
{
    QString name = "新游戏";
    for(int i = 1; i < MaxNonameNum; ++i) {
        if (!model->findItems(name + QString::number(i), Qt::MatchExactly, NameCol).empty())
            continue;
        return name + QString::number(i);
    }
    return QString();
}


QIcon MainWindow::iconFromExe(QString exe)
{
#ifdef WIN32
    HICON hicon;
    if (ExtractIconExA(exe.toStdString().c_str(), 0, &hicon, NULL, 1) > 0) {
        if (hicon)
            return QIcon(QtWin::fromHICON(hicon));
    }
#endif
    return QIcon();
}

void MainWindow::slotOnNewAction()
{
    QString newName = createNewName();
    if (newName.isEmpty()) {
        QMessageBox::information(this, "", "未命名的游戏过多，请修改未命名的游戏后重试。");
        return;
    }
    Data d;
    d.id = QUuid::createUuid().toString();
    d.name = newName;
    if (!Database::instance()->update(d)) {
        QMessageBox::information(this, "", "数据库新增数据失败");
        return;
    }
    QStringList contents;
    contents << d.id << "" << d.name << "" << "" << "" << "";
    updateRow(contents);
}

void MainWindow::slotOnItemChanged(QStandardItem *item)
{
    if (item->column() == IconCol)
        return;
    QString id = model->item(item->row(), IdCol)->text();
    if (item->index().column() == ExePathCol) {
        QIcon icon = iconFromExe(item->text());
        model->item(item->index().row(), NameCol)->setIcon(icon);
        model->item(item->index().row(), IconCol)->setIcon(icon);
        if (!QFile(item->text()).exists())
            model->item(item->index().row(), IconCol)->setBackground(QBrush(InvalidExepathColor));
        else
            model->item(item->index().row(), IconCol)->setBackground(QBrush(ValidExepathColor));
    }
    if (!Database::instance()->update(id, ColNO2ColNameMap[item->column()], item->text())) {
        QMessageBox::information(this, "", "数据库更新失败");
    }
}

void MainWindow::slotOnDelAction()
{
    QModelIndex curindex = ui->tableView->currentIndex();
    if (!curindex.isValid()) {
        QMessageBox::information(this, "", "未选择游戏");
        return;
    }
    if (QMessageBox::information(this, "", "确认删除该记录？") != QMessageBox::StandardButton::Ok)
        return;

    int curRow = curindex.row();
    QString id = model->item(curRow, IdCol)->text();
    if (!Database::instance()->remove(id)) {
        QMessageBox::information(this, "", "数据库删除数据失败");
        return;
    }
    model->removeRow(curRow);
}




