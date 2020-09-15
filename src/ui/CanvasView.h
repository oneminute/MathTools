#ifndef CANVASVIEW_H
#define CANVASVIEW_H

#include <QGraphicsView>
#include <QGenericMatrix> 
#include <QVector2D>
#include <QVector3D>
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

    QMatrix fromSceneMatrix() const;
    QMatrix toSceneMatrix() const;
    qreal lineFactor() const;
    qreal lineWidth(qreal width = 1.0f) const;

    void setImageRaw(const QImage& image) { m_imageRaw = image; }
    void setEncodered(const QImage& image) { m_encodered = image; }
    void setDecodered(const QImage& image) { m_decodered = image; }

    void setSamples(const QList<QVector2D>& samples) { m_samples = samples; }
    void setAvg(qreal avg) { m_avg = avg; }
    void setVar(qreal var) { m_var = var; }
    void setStd(qreal std) { m_std = std; }
    void setSamples3D(const QList<QVector3D>& samples) { m_samples3D = samples; }

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
    void updateDistributionType(DistributionType distributionType);

private:
    void drawGrids();
    void drawAxes();
    void drawEigenMatrix();
    void drawCovMatrix();
    void drawPCA();
    void drawProbability();
    void drawBernoulli();
    void drawNormal();
    void drawNormal2D();
    
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

    QImage m_imageRaw;
    QImage m_encodered;
    QImage m_decodered;

    DistributionType m_distributionType;

    QList<QVector2D> m_samples;
    qreal m_avg;
    qreal m_var;
    qreal m_std;
    QList<QVector3D> m_samples3D;
};

#endif // CANVASVIEW_H
