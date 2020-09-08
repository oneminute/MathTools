#ifndef CANVASVIEW_H
#define CANVASVIEW_H

#include <QGraphicsView>
#include <QGenericMatrix> 
#include <Eigen/Core>
#include <Eigen/Dense>

#include "common.h"

class CanvasView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit CanvasView(QWidget* parent = nullptr);
    virtual ~CanvasView();

    void setMatrix(const QMatrix2x2& matrix) { m_matrix = matrix; }
    QMatrix2x2 matrix() const { return m_matrix; }

    void generateRandomPoints(int count, bool append = false);
    void generateRandomLinePoints(int count, const QPointF& start, const QPointF& end, float radius = 0.2f, bool append = false);
    //void generateRandomCirclePoints(int count);

protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;

    void zoomBy(qreal factor);
    void updateScale(qreal factor);

public slots:
    void updateToolType(ToolType toolType);


private:
    void drawGrids();
    void drawAxes();
    void drawEigenMatrix();
    void drawCovMatrix();
    
private:
    bool m_init;
    qreal m_scaleFactor;
    qreal m_factor;

    QPointF m_mousePoint;
    QPointF m_origin;
    bool m_pressed;

    QMatrix2x2 m_matrix;

    ToolType m_toolType;

    QList<QPointF> m_points;
};

#endif // CANVASVIEW_H
