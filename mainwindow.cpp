#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    table = new Table(this);
    ui->graphicsView->setScene(table);
    QLabel *gameLabel = new QLabel(tr("Game status"));
    ui->statusBar->addWidget(gameLabel);
    connect(table, SIGNAL(setGameStatusText(QString)), gameLabel,
            SLOT(setText(QString)));
    resizeEvent(nullptr); // it's safe because event value isn't actually used
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QRectF bounds = table->sceneRect();
    ui->graphicsView->fitInView(bounds, Qt::KeepAspectRatio);
}

void MainWindow::on_actionNew_Game_triggered() { table->newGame(); }

void MainWindow::on_actionStats_triggered()
{
    QMessageBox::information(this, tr("Stats isn't yet implemented..."),
                             tr("But it is planned in future versions."));
}

void MainWindow::on_actionSettings_triggered() {}

void MainWindow::on_actionQuit_triggered() { QApplication::quit(); }

void MainWindow::on_actionHelp_triggered()
{
    QMessageBox::information(this, tr("Help system is actually absent"),
                             tr("You will be redirected to Wikipedia page"));
    QDesktopServices::openUrl(QUrl("https://en.wikipedia.org/wiki/Durak"));
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, "OpenFool",
                       "Free and open source Fool game implementation");
}
