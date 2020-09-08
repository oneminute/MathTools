#include "CanvasView.h"

#include <QDebug>
#include <QEvent>
#include <QPainter>
#include <QRandomGenerator>
#include <QtMath>
#include <QWheelEvent>
#include <iostream>

CanvasView::CanvasView(QWidget* parent)
    : QGraphicsView(parent)
    , m_init(false)
    , m_scaleFactor(10)
    , m_factor(50)
    , m_origin(0, 0)
    , m_pressed(false)
{
    qDebug() << "create canvas widget.";

    setScene(new QGraphicsScene(this));
    setTransformationAnchor(AnchorUnderMouse);
    setViewportUpdateMode(FullViewportUpdate);
    setResizeAnchor(AnchorUnderMouse);
    //scale(m_scaleFactor, m_scaleFactor);

    QRectF rect(0, 0, 1000, 1000);
    setSceneRect(rect);
    m_origin = QPointF(rect.width() / 2, rect.height() / 2);

    m_matrix(0, 0) = 2;
    m_matrix(1, 0) = 2;
    m_matrix(0, 1) = 3;
    m_matrix(1, 1) = 1;
}

CanvasView::~CanvasView()
{

}

void CanvasView::generateRandomPoints(int count, bool append)
{
    if (!append)
        m_points.clear();

    QRectF sRect = sceneRect();
    QRandomGenerator* rand = QRandomGenerator::global();

    for (int i = 0; i < count; i++)
    {
        int x = rand->bounded((int)sRect.left(), (int)sRect.right());
        int y = rand->bounded((int)sRect.top(), (int)sRect.bottom());

        QPointF point = QPointF(x, y) - m_origin;
        point /= m_factor;
        m_points.append(point);
    }

    scene()->update();
}

void CanvasView::generateRandomLinePoints(int count, const QPointF& start, const QPointF& end, float radius, bool append)
{
    if (!append)
        m_points.clear();

    QRandomGenerator* rand = QRandomGenerator::global();
    QVector2D dir = QVector2D(end - start);
    QVector2D dirN = dir.normalized();
    QVector2D vertN(dirN.x(), -dirN.y());
    for (int i = 0; i < count; i++)
    {
        QPointF point = start + (dir * rand->generateDouble()).toPointF();
        double r = (rand->bounded(2.0) - 1.0) * radius;
        point = (r * vertN).toPointF() + point;
        m_points.append(point);
    }

    scene()->update();
}

void CanvasView::paintEvent(QPaintEvent * event)
{
    QGraphicsView::paintEvent(event);

    drawGrids();
    drawAxes();
    
    switch (m_toolType)
    {
    case TT_EigenMatrix:
        drawEigenMatrix();
        break;
    case TT_CovMatrix:
        drawCovMatrix();
        break;
    }
}

void CanvasView::mousePressEvent(QMouseEvent * event)
{
    m_pressed = true;
    m_mousePoint = mapToScene(event->pos()) - m_origin;
    scene()->update();
    QGraphicsView::mousePressEvent(event);
}

void CanvasView::mouseReleaseEvent(QMouseEvent * event)
{
    m_pressed = false;
    QGraphicsView::mouseReleaseEvent(event);
}

void CanvasView::mouseMoveEvent(QMouseEvent * event)
{
    if (m_pressed)
    {
        m_mousePoint = mapToScene(event->pos()) - m_origin;
        scene()->update();
    }
    QGraphicsView::mouseMoveEvent(event);
}

void CanvasView::wheelEvent(QWheelEvent * event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        if (event->delta() > 0)
            zoomBy(1.2);
        else
            zoomBy(0.8);
        event->accept();
    }
    else {
        QGraphicsView::wheelEvent(event);
    }
}

void CanvasView::zoomBy(qreal factor)
{
    //qDebug() << "m_scaleFactor:" << m_scaleFactor;
    if ((m_scaleFactor < 0.01 && factor < 1) || (m_scaleFactor > 100 && factor > 1))
        return;

    updateScale(factor);
}

void CanvasView::updateScale(qreal factor)
{
    scale(factor, factor);
    m_scaleFactor = transform().m11();
}

void CanvasView::drawGrids()
{
    QPainter painter(viewport());
    QRectF sRect = sceneRect();

    painter.setPen(QPen(Qt::black, 1, Qt::DashLine, Qt::RoundCap));
    for (int i = m_origin.y(); i < sRect.bottom(); i += m_factor)
    {
        painter.drawLine(mapFromScene(QPointF(sRect.left(), i)), mapFromScene(QPointF(sRect.right(), i)));
    }
    for (int i = m_origin.y(); i > sRect.top(); i -= m_factor)
    {
        painter.drawLine(mapFromScene(QPointF(sRect.left(), i)), mapFromScene(QPointF(sRect.right(), i)));
    }
    for (int i = m_origin.x(); i < sRect.right(); i += m_factor)
    {
        painter.drawLine(mapFromScene(QPointF(i, sRect.top())), mapFromScene(QPointF(i, sRect.bottom())));
    }
    for (int i = m_origin.x(); i > sRect.left(); i -= m_factor)
    {
        painter.drawLine(mapFromScene(QPointF(i, sRect.top())), mapFromScene(QPointF(i, sRect.bottom())));
    }

}

void CanvasView::drawAxes()
{   
    QPainter painter(viewport());
    QRectF sRect = sceneRect();

    QRectF rect = mapFromScene(sRect).boundingRect();
    QPointF origin = mapFromScene(m_origin);
    painter.setPen(QPen(Qt::black, 2, Qt::SolidLine, Qt::PenCapStyle::RoundCap));
    painter.drawRect(rect);

    painter.drawLine(QPointF(rect.left(), origin.y()), QPointF(rect.right(), origin.y()));
    painter.drawLine(QPointF(origin.x(), rect.top()), QPointF(origin.x(), rect.bottom()));
}

void CanvasView::drawEigenMatrix()
{
    QPainter painter(viewport());
    painter.setPen(QPen(Qt::red, 2, Qt::SolidLine, Qt::PenCapStyle::RoundCap));
    QRectF sRect = sceneRect();
    QRectF rect = mapFromScene(sRect).boundingRect();
    QPointF origin = mapFromScene(m_origin);
    
    QMatrix m;
    m.translate(origin.x(), origin.y());
    m.scale(transform().m11(), -transform().m11());
    painter.setMatrix(m);

    m_mousePoint.setY(-m_mousePoint.y());
    int rx = static_cast<int>(2 * rect.width() / sceneRect().width());
    int ry = static_cast<int>(2 * rect.height() / sceneRect().height());
    painter.setPen(QPen(Qt::darkRed, 2, Qt::SolidLine));
    painter.drawEllipse(m_mousePoint, rx, ry);
    painter.drawLine(QPointF(0, 0), m_mousePoint);
    qDebug() << m_mousePoint / m_factor;

    Eigen::Vector2f point(m_mousePoint.x() / m_factor, m_mousePoint.y() / m_factor);
    std::cout << "point:" << point.transpose() << std::endl;

    Eigen::Matrix2f m2f;
    m2f = Eigen::Matrix2f::Map(m_matrix.data());
    std::cout << m2f << std::endl;

    Eigen::Vector2f result = m2f * point;
    std::cout << "result:" << point.transpose() << std::endl;
    painter.setPen(QPen(Qt::darkGreen, 1, Qt::SolidLine));
    painter.drawEllipse(QPointF(result.x() * m_factor, result.y() * m_factor), rx, ry);
    painter.drawLine(QPointF(0, 0), QPointF(result.x() * m_factor, result.y() * m_factor));

    Eigen::EigenSolver<Eigen::Matrix2f> eigensolver(m2f);
    Eigen::Matrix2f em = eigensolver.eigenvectors().real();
    Eigen::Vector2f ev = eigensolver.eigenvalues().real();
    Eigen::Vector2f e1 = em.col(0) * ev.x();
    Eigen::Vector2f e2 = em.col(1) * ev.y();
    std::cout << "eigen vector 1:" << e1.transpose() << std::endl;
    std::cout << "eigen vector 2:" << e2.transpose() << std::endl;
    std::cout << "eigen values:" << ev.transpose() << std::endl;
    //std::cout << "eigen ratio:" << std::abs(ev.x() / ev.y()) << std::endl;
    Eigen::Vector2f longVec = m2f.col(0) + m2f.col(1);
    Eigen::Vector2f shortVec = m2f.col(0) - m2f.col(1);
    //std::cout << "vector ratio:" << std::abs(longVec.norm() / shortVec.norm()) << std::endl;
    painter.setPen(QPen(Qt::cyan, 3, Qt::SolidLine));
    painter.drawLine(QPointF(0, 0), QPointF(e1.x(), e1.y()) * m_factor);
    painter.setPen(QPen(Qt::green, 3, Qt::SolidLine));
    painter.drawLine(QPointF(0, 0), QPointF(e2.x(), e2.y()) * m_factor);

    QPointF v1 = QPointF(m_matrix(0, 0), m_matrix(1, 0)) * m_factor;
    QPointF v2 = QPointF(m_matrix(0, 1), m_matrix(1, 1)) * m_factor;
    for (int i = -10; i < 10; i++)
    {
        painter.setPen(QPen(Qt::red, 1, Qt::SolidLine));
        painter.drawLine(v1 * -10 + v2 * i, v1 * 10 + v2 * i);
        painter.setPen(QPen(Qt::blue, 1, Qt::SolidLine));
        painter.drawLine(v2 * -10 + v1 * i, v2 * 10 + v1 * i);
    }
}

void CanvasView::drawCovMatrix()
{
    QPainter painter(viewport());
    QRectF sRect = sceneRect();
    QRectF rect = mapFromScene(sRect).boundingRect();
    QPointF origin = mapFromScene(m_origin);
    qreal lineFactor = rect.width() / sceneRect().width() / m_factor;

    QTransform t(transform());
    QMatrix m(m_factor, 0, 0, -m_factor, origin.x(), origin.y());
    m = t.toAffine() * m;
    painter.setMatrix(m);

    if (m_points.isEmpty())
        return;

    Eigen::Vector2f center(Eigen::Vector2f::Zero());
    for (QPointF pt : m_points)
    {
        //painter.setPen(QPen(QColor::fromRgb(QRandomGenerator::global()->generate()), 2, Qt::SolidLine, Qt::PenCapStyle::RoundCap));
        painter.setPen(QPen(Qt::black, 2 * lineFactor, Qt::NoPen, Qt::PenCapStyle::RoundCap));
        painter.setBrush(Qt::darkYellow);

        center.x() += pt.x();
        center.y() += pt.y();

        painter.drawEllipse(pt, 4 * lineFactor, 4 * lineFactor);
    }
    center /= m_points.size();
    std::cout << "center:" << center.transpose() << std::endl;

    Eigen::Matrix2f matrix(Eigen::Matrix2f::Zero());
    for (QPointF pt : m_points)
    {
        matrix.row(0).x() += (pt.x() - center.x()) * (pt.x() - center.x());
        matrix.row(0).y() += (pt.x() - center.x()) * (pt.y() - center.y());
        matrix.row(1).x() += (pt.x() - center.x()) * (pt.y() - center.y());
        matrix.row(1).y() += (pt.y() - center.y()) * (pt.y() - center.y());
    }
    matrix /= m_points.size();
    std::cout << "matrix:" << std::endl;
    std::cout << matrix << std::endl;

    QPointF lineCenter = QPointF(center.x(), center.y());

    painter.setPen(QPen(Qt::green, lineFactor));
    painter.setBrush(Qt::green);
    painter.drawEllipse(lineCenter, 6 * lineFactor, 6 * lineFactor);

    Eigen::EigenSolver<Eigen::Matrix2f> eigensolver(matrix);
    Eigen::Matrix2f em = eigensolver.eigenvectors().real();
    Eigen::Vector2f ev = eigensolver.eigenvalues().real();
    Eigen::Vector2f e1 = em.col(0) * ev.x();
    Eigen::Vector2f e2 = em.col(1) * ev.y();
    std::cout << "e1 * e2 = " << e1.dot(e2) << std::endl;
    std::cout << "eigen vector 1:" << e1.transpose() << std::endl;
    std::cout << "eigen vector 2:" << e2.transpose() << std::endl;
    std::cout << "eigen values:" << ev.transpose() << std::endl;
    painter.setPen(QPen(Qt::cyan, 3 * lineFactor, Qt::SolidLine));
    painter.drawLine(lineCenter, lineCenter + QPointF(e1.x(), e1.y()));
    painter.setPen(QPen(Qt::green, 3 * lineFactor, Qt::SolidLine));
    painter.drawLine(lineCenter, lineCenter + QPointF(e2.x(), e2.y()));
}

void CanvasView::updateToolType(ToolType toolType)
{
    m_toolType = toolType;
    scene()->update();
}
