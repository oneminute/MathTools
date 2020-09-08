#include "MainWindow.h"
#include "ui/ui_MainWindow.h"
#include "CanvasView.h"

#include <QActionGroup>
#include <QGenericMatrix>
#include <QPushButton>
#include <QSpinBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_toolsGroup = new QActionGroup(this);
    m_toolsGroup->addAction(ui->actionEigenMatrixTool);
    m_toolsGroup->addAction(ui->actionCovMatrixTool);

    ui->toolButtonGenerate->setDefaultAction(ui->actionGenerate);

    QMatrix2x2 matrix = ui->graphicsViewCanvas->matrix();
    ui->lineEdit00->setText(QString::number(matrix(0, 0)));
    ui->lineEdit10->setText(QString::number(matrix(1, 0)));
    ui->lineEdit01->setText(QString::number(matrix(0, 1)));
    ui->lineEdit11->setText(QString::number(matrix(1, 1)));

    connect(ui->pushButtonApply, &QPushButton::clicked, this, &MainWindow::onApply);
    connect(m_toolsGroup, &QActionGroup::triggered, this, &MainWindow::onToolsGroupTriggered);
    connect(ui->actionGenerate, &QAction::triggered, this, &MainWindow::onActionGenerate);

    ui->graphicsViewCanvas->updateToolType(TT_EigenMatrix);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onToolsGroupTriggered(QAction * action)
{
    qDebug() << action;
    if (action == ui->actionEigenMatrixTool)
    {
        ui->graphicsViewCanvas->updateToolType(TT_EigenMatrix);
    }
    else if (action == ui->actionCovMatrixTool)
    {
        //ui->graphicsViewCanvas->setPointsCount(ui->spinBoxCount->value());
        ui->graphicsViewCanvas->updateToolType(TT_CovMatrix);
    }
}

void MainWindow::onActionGenerate(bool checked)
{
    if (ui->radioButtonAllRandom->isChecked())
    {
        ui->graphicsViewCanvas->generateRandomPoints(ui->spinBoxCount->value(), ui->checkBoxAppend->isChecked());
    }
    else if (ui->radioButtonLineRandom->isChecked())
    {
        QPointF start(ui->doubleSpinBoxStartX->value(), ui->doubleSpinBoxStartY->value());
        QPointF end(ui->doubleSpinBoxEndX->value(), ui->doubleSpinBoxEndY->value());
        float radius = ui->doubleSpinBoxRandRadius->value();
        ui->graphicsViewCanvas->generateRandomLinePoints(ui->spinBoxCount->value(),
            start, end, radius, ui->checkBoxAppend->isChecked());
    }
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
