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
#include <QVector3D>
#include <QMatrix>

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
    connect(ui->comboBoxDistributionType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onComboBoxDistributionTypeChanged);

    ui->graphicsViewCanvas->updateToolType(TT_EigenMatrix);

    ui->comboBoxDistributionType->addItem("Bernoulli", DT_BERNOULLI);
    //ui->comboBoxDistributionType->addItem("Multinoulli", DT_MULTINOULLI);
    ui->comboBoxDistributionType->addItem("Normal", DT_NORMAL);
    ui->comboBoxDistributionType->addItem("Normal2D", DT_NORMAL2D);

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

    qreal sum = 0;
    qreal avg = 0;
    qreal var = 0;
    qreal std = 0;
    QList<QVector2D> samples;
    QList<QVector3D> samples3D;
    if (type == DT_BERNOULLI)
    {
        qreal probability = ui->doubleSpinBoxBernoulliProbability->value();
        int count = ui->spinBoxBernoulliCount->value();
        for (int i = 0; i <= count; i++)
        {
            qreal value = qPow(probability, i) * (qPow(1 - probability, count - i));
            samples.append(QVector2D(i, value));
            sum += value;
        }
        avg = sum / samples.size();
        for (int i = 0; i < samples.size(); i++)
        {
            var += (samples[i].y() - avg) * (samples[i].y() - avg);
        }
        var /= samples.size();
        std = qSqrt(var);
    }
    else if (type == DT_NORMAL)
    {
        qreal u = ui->doubleSpinBoxNormalU->value();
        qreal sigma = ui->doubleSpinBoxNormalSigma->value();
        qreal delta = ui->doubleSpinBoxNormalDelta->value();
        for (qreal i = -10; i <= 10; i += delta)
        {
            qreal value = qSqrt(1 / (2 * M_PI * sigma * sigma)) * qExp(-1.0f / (2 * sigma * sigma) * (i - u) * (i - u));
            samples.append(QVector2D(i, value));
            sum += value;
        }
        avg = sum / samples.size();
        for (int i = 0; i < samples.size(); i++)
        {
            var += (samples[i].y() - avg) * (samples[i].y() - avg);
        }
        var /= samples.size();
        std = qSqrt(var);
    }
    else if (type == DT_NORMAL2D)
    {
        qreal delta = ui->doubleSpinBoxNormalDelta->value();
        qreal sigma = ui->doubleSpinBoxNormalSigma->value();
        QList<Eigen::Vector2f> points2D;
        Eigen::Vector2f avg(Eigen::Vector2f::Zero());
        for (qreal i = -10; i <= 10; i += delta)
        {
            for (qreal j = -10; j <= 10; j += delta)
            {
                Eigen::Vector2f point(i / 10, j / 10);
                points2D.append(point);
                avg += point;
            }
        }
        avg /= points2D.size();
        Eigen::Matrix2f covM(Eigen::Matrix2f::Zero());
        for (int i = 0; i < points2D.size(); i++)
        {
            covM(0, 0) += (points2D[i].x() - avg.x()) * (points2D[i].x() - avg.x());
            covM(0, 1) += (points2D[i].x() - avg.x()) * (points2D[i].y() - avg.y());
            covM(1, 0) += (points2D[i].x() - avg.x()) * (points2D[i].y() - avg.y());
            covM(1, 1) += (points2D[i].y() - avg.y()) * (points2D[i].y() - avg.y());
        }
        //covM /= points2D.size() * sigma;
        covM = Eigen::Matrix2f::Identity();
        covM *= sigma;
        //std::cout << covM << std::endl;
        //covM = covM.cwiseSqrt();
        Eigen::Matrix2f covMInv = covM.inverse();
        std::cout << "avg:" << avg.transpose() << std::endl;
        std::cout << covM << std::endl;
        std::cout << covMInv << std::endl;
        qreal det = covM.determinant();
        qreal det_1 = 1.0 / det;
        qreal pi_2 = 0.5 / M_PI;

        qreal min = 65535, max = 0;
        for (int i = 0; i < points2D.size(); i++)
        {
            Eigen::Vector2f x = points2D[i];
            qreal v = x.transpose() * covM * x;
            qreal value = pi_2 * qSqrt(det_1) * qExp(-0.5 * v);
            if (value < min) min = value;
            if (value > max) max = value;
            QVector3D point(x.x(), x.y(), value);
            samples3D.append(point);
        }
        for (int i = 0; i < samples3D.size(); i++)
        {
            samples3D[i].setZ((samples3D[i].z() - min) / (max - min));
        }
    }

    qDebug() << "avg =" << avg;
    qDebug() << "var =" << var;
    qDebug() << "std =" << std;

    ui->graphicsViewCanvas->setSamples(samples);
    ui->graphicsViewCanvas->setSamples3D(samples3D);
    ui->graphicsViewCanvas->setAvg(avg);
    ui->graphicsViewCanvas->setVar(var);
    ui->graphicsViewCanvas->setStd(std);
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
    else if (type == DT_NORMAL2D)
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
