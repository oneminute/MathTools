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

QMatrix CanvasView::transformMatrix() const
{
    QPointF origin = mapFromScene(m_origin);
    QTransform t(transform());
    QMatrix m(m_factor, 0, 0, -m_factor, origin.x(), origin.y());
    m = t.toAffine() * m;
    return m;
}

void CanvasView::paintEvent(QPaintEvent * event)
{
    QGraphicsView::paintEvent(event);

    switch (m_toolType)
    {
    case TT_EigenMatrix:
        drawEigenMatrix();
        break;
    case TT_CovMatrix:
        drawCovMatrix();
        break;
    case TT_PCA:
        drawPCA();
        break;
    }
}

void CanvasView::mousePressEvent(QMouseEvent * event)
{
    m_pressed = true;
    m_mousePoint = (mapToScene(event->pos()) - m_origin) / m_factor;
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
        m_mousePoint = (mapToScene(event->pos()) - m_origin) / m_factor;
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
    drawGrids();
    drawAxes();

    QPainter painter(viewport());
    painter.setPen(QPen(Qt::red, 2, Qt::SolidLine, Qt::PenCapStyle::RoundCap));
    QRectF sRect = sceneRect();
    QRectF rect = mapFromScene(sRect).boundingRect();
    QPointF origin = mapFromScene(m_origin);
    qreal lineFactor = 1.0 / m_factor;
    
    QMatrix m = transformMatrix();
    painter.setMatrix(m);
    QMatrix mInv = m.inverted();

    m_mousePoint.setY(-m_mousePoint.y());
    painter.setPen(QPen(Qt::darkRed, 2 * lineFactor, Qt::SolidLine));
    painter.drawEllipse(m_mousePoint, 2 * lineFactor, 2 * lineFactor);
    painter.drawLine(QPointF(0, 0), m_mousePoint);
    qDebug() << m_mousePoint;

    Eigen::Vector2f point(m_mousePoint.x(), m_mousePoint.y());
    std::cout << "point:" << point.transpose() << std::endl;

    Eigen::Matrix2f m2f;
    m2f = Eigen::Matrix2f::Map(m_matrix.data());
    std::cout << m2f << std::endl;

    Eigen::Vector2f result = m2f * point;
    std::cout << "result:" << point.transpose() << std::endl;
    painter.setPen(QPen(Qt::darkGreen, 1 * lineFactor, Qt::SolidLine));
    painter.drawEllipse(QPointF(result.x(), result.y()), 2 * lineFactor, 2 * lineFactor);
    painter.drawLine(QPointF(0, 0), QPointF(result.x(), result.y()));

    Eigen::EigenSolver<Eigen::Matrix2f> eigensolver(m2f);
    Eigen::Matrix2f em = eigensolver.eigenvectors().real();
    Eigen::Vector2f ev = eigensolver.eigenvalues().real();
    Eigen::Vector2f e1 = em.col(0) * ev.x();
    Eigen::Vector2f e2 = em.col(1) * ev.y();
    std::cout << "eigen vector 1:" << e1.transpose() << std::endl;
    std::cout << "eigen vector 2:" << e2.transpose() << std::endl;
    std::cout << "eigen values:" << ev.transpose() << std::endl;
    painter.setPen(QPen(Qt::red, 3 * lineFactor, Qt::SolidLine));
    painter.drawLine(QPointF(0, 0), QPointF(e1.x(), e1.y()));
    painter.setPen(QPen(Qt::blue, 3 * lineFactor, Qt::SolidLine));
    painter.drawLine(QPointF(0, 0), QPointF(e2.x(), e2.y()));

    //rect = mInv.mapRect(sRect);
    rect = QRectF(-10, -10, 20, 20);
    QPointF v1 = QPointF(m_matrix(0, 0), m_matrix(1, 0));
    QPointF v2 = QPointF(m_matrix(0, 1), m_matrix(1, 1));
    QLineF leftLine(rect.topLeft(), rect.bottomLeft());
    QLineF rightLine(rect.topRight(), rect.bottomRight());
    QLineF topLine(rect.topLeft(), rect.topRight());
    QLineF bottomLine(rect.bottomLeft(), rect.bottomRight());
    rect = QRectF(-10.01, -10.01, 20.02, 20.02);
    QList<QLineF> lines;
    lines << leftLine << rightLine << topLine << bottomLine;
    int count = 0;
    float diagonal = std::sqrt(rect.width() * rect.width() + rect.height() * rect.height());
    while (true)
    {
        QLineF line1(v1 * -diagonal + v2 * count, v1 * diagonal + v2 * count);
        QLineF line2(v1 * -diagonal + v2 * -count, v1 * diagonal + v2 * -count);
        QLineF line3(v1 * count + v2 * -diagonal, v1 * count + v2 * diagonal);
        QLineF line4(v1 * -count + v2 * -diagonal, v1 * -count + v2 * diagonal);

        QList<QPointF> intersects1;
        QList<QPointF> intersects2;
        QList<QPointF> intersects3;
        QList<QPointF> intersects4;
        for (QLineF l : lines)
        {
            QPointF p1(0, 0);
            QPointF p2(0, 0);
            QPointF p3(0, 0);
            QPointF p4(0, 0);
            line1.intersect(l, &p1);
            line2.intersect(l, &p2);
            line3.intersect(l, &p3);
            line4.intersect(l, &p4);
            if (!p1.isNull() && rect.contains(p1))
            {
                intersects1 << p1;
            }
            if (!p2.isNull() && rect.contains(p2))
            {
                intersects2 << p2;
            }
            if (!p3.isNull() && rect.contains(p3))
            {
                intersects3 << p3;
            }
            if (!p4.isNull() && rect.contains(p4))
            {
                intersects4 << p4;
            }
        }

        if (intersects1.isEmpty() && intersects2.isEmpty() &&
            intersects3.isEmpty() && intersects4.isEmpty())
            break;

        if (intersects1.size() >= 2)
        {
            painter.setPen(QPen(Qt::cyan, 2 * lineFactor, Qt::SolidLine));
            painter.drawLine(intersects1[0], intersects1[1]);
        }

        if (intersects2.size() >= 2)
        {
            painter.setPen(QPen(Qt::cyan, 2 * lineFactor, Qt::SolidLine));
            painter.drawLine(intersects2[0], intersects2[1]);
        }

        if (intersects3.size() >= 2)
        {
            painter.setPen(QPen(Qt::lightGray, 2 * lineFactor, Qt::SolidLine));
            painter.drawLine(intersects3[0], intersects3[1]);
        }

        if (intersects4.size() >= 2)
        {
            painter.setPen(QPen(Qt::lightGray, 2 * lineFactor, Qt::SolidLine));
            painter.drawLine(intersects4[0], intersects4[1]);
        }
        count++;
    }
    /*for (int i = -10; i < 10; i++)
    {
        painter.setPen(QPen(Qt::cyan, 1 * lineFactor, Qt::SolidLine));
        painter.drawLine(v1 * -10 + v2 * i, v1 * 10 + v2 * i);
        painter.setPen(QPen(Qt::lightGray, 1 * lineFactor, Qt::SolidLine));
        painter.drawLine(v2 * -10 + v1 * i, v2 * 10 + v1 * i);
    }*/
}

void CanvasView::drawCovMatrix()
{
    drawGrids();
    drawAxes();

    QPainter painter(viewport());
    QRectF sRect = sceneRect();
    QRectF rect = mapFromScene(sRect).boundingRect();
    QPointF origin = mapFromScene(m_origin);
    qreal lineFactor = 1.0 / m_factor;

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
    std::cout << "eigen vector 1:" << e1.transpose() << std::endl;
    std::cout << "eigen vector 2:" << e2.transpose() << std::endl;
    std::cout << "eigen values:" << ev.transpose() << std::endl;
    painter.setPen(QPen(Qt::red, 3 * lineFactor, Qt::SolidLine));
    painter.drawLine(lineCenter, lineCenter + QPointF(e1.x(), e1.y()));
    painter.setPen(QPen(Qt::blue, 3 * lineFactor, Qt::SolidLine));
    painter.drawLine(lineCenter, lineCenter + QPointF(e2.x(), e2.y()));
}

void CanvasView::drawPCA()
{
    QPainter painter(viewport());
    QTransform t(transform());
    t.translate(0, 0);

    painter.setTransform(transform());
    painter.drawImage(QPoint(), m_imageRaw);
    painter.drawImage(QPoint() + QPoint(m_imageRaw.width() + 5, 0), m_encodered);
    painter.drawImage(QPoint() + QPoint(0, m_imageRaw.height() + 5), m_decodered);
}

void CanvasView::updateToolType(ToolType toolType)
{
    m_toolType = toolType;
    scene()->update();
}
