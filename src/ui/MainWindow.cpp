#include "MainWindow.h"
#include "ui/ui_MainWindow.h"
#include "CanvasView.h"
#include <QPushButton>
#include <QGenericMatrix>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QMatrix2x2 matrix = ui->graphicsViewCanvas->matrix();
    ui->lineEdit00->setText(QString::number(matrix(0, 0)));
    ui->lineEdit10->setText(QString::number(matrix(1, 0)));
    ui->lineEdit01->setText(QString::number(matrix(0, 1)));
    ui->lineEdit11->setText(QString::number(matrix(1, 1)));

    connect(ui->pushButtonApply, &QPushButton::clicked, this, &MainWindow::onApply);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onApply(bool checked)
{
    QMatrix2x2 matrix;
    matrix(0, 0) = ui->lineEdit00->text().toInt();
    matrix(1, 0) = ui->lineEdit10->text().toInt();
    matrix(0, 1) = ui->lineEdit01->text().toInt();
    matrix(1, 1) = ui->lineEdit11->text().toInt();
    ui->graphicsViewCanvas->setMatrix(matrix);
}
