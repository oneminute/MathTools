#include "MainWindow.h"
#include "ui/ui_MainWindow.h"
#include "CanvasView.h"

#include <QActionGroup>
#include <QFileDialog>
#include <QGenericMatrix>
#include <QImage>
#include <QPushButton>
#include <QSpinBox>
#include <QtMath>

#include <opencv2/opencv.hpp>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_toolsGroup = new QActionGroup(this);
    m_toolsGroup->addAction(ui->actionEigenMatrixTool);
    m_toolsGroup->addAction(ui->actionCovMatrixTool);
    m_toolsGroup->addAction(ui->actionPCATool);
    m_toolsGroup->addAction(ui->actionProbabilityTool);

    ui->toolButtonGenerate->setDefaultAction(ui->actionGenerate);
    ui->toolButtonOpenImage->setDefaultAction(ui->actionOpenImage);
    ui->toolButtonShowDistribution->setDefaultAction(ui->actionShowDistribution);

    QMatrix2x2 matrix = ui->graphicsViewCanvas->matrix();
    ui->lineEdit00->setText(QString::number(matrix(0, 0)));
    ui->lineEdit10->setText(QString::number(matrix(1, 0)));
    ui->lineEdit01->setText(QString::number(matrix(0, 1)));
    ui->lineEdit11->setText(QString::number(matrix(1, 1)));

    connect(ui->pushButtonApply, &QPushButton::clicked, this, &MainWindow::onApply);
    connect(m_toolsGroup, &QActionGroup::triggered, this, &MainWindow::onToolsGroupTriggered);
    connect(ui->actionGenerate, &QAction::triggered, this, &MainWindow::onActionGenerate);
    connect(ui->actionOpenImage, &QAction::triggered, this, &MainWindow::onActionOpenImage);
    connect(ui->actionShowDistribution, &QAction::triggered, this, &MainWindow::showDistribution);

    ui->graphicsViewCanvas->updateToolType(TT_EigenMatrix);

    ui->comboBoxDistributionType->addItem("Bernoulli", DT_BERNOULLI);
    ui->comboBoxDistributionType->addItem("Multinoulli", DT_MULTINOULLI);
    ui->comboBoxDistributionType->addItem("Normal", DT_NORMAL);

    showDistribution();
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
        ui->graphicsViewCanvas->updateToolType(TT_CovMatrix);
    }
    else if (action == ui->actionPCATool)
    {
        ui->graphicsViewCanvas->updateToolType(TT_PCA);
    }
    else if (action == ui->actionProbabilityTool)
    {
        ui->graphicsViewCanvas->updateToolType(TT_Probability);
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

void MainWindow::onActionOpenImage(bool checked)
{
    QString filename = QFileDialog::getOpenFileName(this,
        tr("Open Image"), tr("."), tr("Image (*.png *.bmp *.jpeg);;"));

    if (!filename.isNull() && !filename.isEmpty())
    {
        QImage image(filename);
        ui->graphicsViewCanvas->setImageRaw(image);

        int total = image.width() * image.height();
        Eigen::MatrixXf X;
        X.resize(3, total);
        for (int i = 0; i < image.height(); i++)
        {
            for (int j = 0; j < image.width(); j++)
            {
                QRgb rgb = image.pixel(j, i);
                Eigen::Vector3f x;
                x.x() = qRed(rgb);
                x.y() = qGreen(rgb);
                x.z() = qBlue(rgb);
                X.col(i * image.width() + j) = x;
            }
        }

        Eigen::Matrix3f M = X * X.transpose();
        Eigen::EigenSolver<Eigen::Matrix3f> eigensolver(M);
        Eigen::Matrix3f em = eigensolver.eigenvectors().real();
        Eigen::Vector3f ev = eigensolver.eigenvalues().real();
        Eigen::Vector3f e1 = em.col(0) * ev.x();
        Eigen::Vector3f d = e1.normalized();

        std::cout << "M" << std::endl;
        std::cout << M << std::endl;
        std::cout << "eigen vector 1:" << e1.transpose() << std::endl;
        std::cout << "d:" << d.transpose() << std::endl;
        std::cout << "dT * d:" << (d.transpose() * d) << std::endl;

        Eigen::Vector3f white(255, 255, 255);
        float max = white.transpose() * d;
        float min = 0;

        QImage encodered(image.width(), image.height(), QImage::Format::Format_Grayscale8);
        cv::Mat mat(image.height(), image.width(), CV_32F);
        for (int i = 0; i < image.height(); i++)
        {
            for (int j = 0; j < image.width(); j++)
            {
                QRgb rgb = image.pixel(j, i);
                Eigen::Vector3f x;
                x.x() = qRed(rgb);
                x.y() = qGreen(rgb);
                x.z() = qBlue(rgb);

                float output = d.transpose() * x;
                mat.ptr<float>(i)[j] = output;
                int gray = output * 255 / max;
                encodered.setPixelColor(j, i, QColor::fromRgb(gray, gray, gray));
            }
        }

        ui->graphicsViewCanvas->setEncodered(encodered);

        //Eigen::Matrix3f dM = d * d.transpose();
        QImage decodered(image.width(), image.height(), QImage::Format::Format_RGB888);
        for (int i = 0; i < image.height(); i++)
        {
            for (int j = 0; j < image.width(); j++)
            {
                float input = mat.ptr<float>(i)[j];
                Eigen::Vector3f output = d * input;

                output.x() = qBound(0.f, output.x(), 255.f);
                output.y() = qBound(0.f, output.y(), 255.f);
                output.z() = qBound(0.f, output.z(), 255.f);

                decodered.setPixelColor(j, i, QColor::fromRgb(output.x(), output.y(), output.z()));
            }
        }

        ui->graphicsViewCanvas->setDecodered(decodered);
        ui->graphicsViewCanvas->scene()->update();
    }
}

void MainWindow::showDistribution(bool ckecked)
{
    DistributionType type = static_cast<DistributionType>(ui->comboBoxDistributionType->currentData(Qt::UserRole).toInt());

    ui->graphicsViewCanvas->updateDistributionType(type);

    ui->graphicsViewCanvas->setBernoulliProbability(ui->doubleSpinBoxBernoulliProbability->value());
    ui->graphicsViewCanvas->setBernoulliCount(ui->spinBoxBernoulliCount->value());
    ui->graphicsViewCanvas->setNormalU(ui->doubleSpinBoxNormalU->value());
    ui->graphicsViewCanvas->setNormalSigma(ui->doubleSpinBoxNormalSigma->value());

    ui->graphicsViewCanvas->scene()->update();
}

void MainWindow::onComboBoxDistributionTypeChanged(int index)
{
    DistributionType type = static_cast<DistributionType>(ui->comboBoxDistributionType->currentData(Qt::UserRole).toInt());

    ui->groupBoxBernoulli->setVisible(false);
    ui->groupBoxNormal->setVisible(false);
    if (type == DT_BERNOULLI)
    {
        ui->groupBoxBernoulli->setVisible(true);
    }
    else if (type == DT_NORMAL)
    {
        ui->groupBoxNormal->setVisible(true);
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
