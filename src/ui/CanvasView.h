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

    QMatrix fromSceneMatrix() const;
    QMatrix toSceneMatrix() const;
    qreal lineFactor() const;
    qreal lineWidth(qreal width = 1.0f) const;

    void setImageRaw(const QImage& image) { m_imageRaw = image; }
    void setEncodered(const QImage& image) { m_encodered = image; }
    void setDecodered(const QImage& image) { m_decodered = image; }

    void setBernoulliProbability(qreal probability) { m_bernoulliProbability = probability; }
    qreal bernoulliProbability() const { return m_bernoulliProbability; }

    void setBernoulliCount(int count) { m_bernoulliCount = count; }
    int bernoulliCount() { return m_bernoulliCount; }

    void setNormalU(qreal u) { m_normalU = u; }
    qreal normalU() const { return m_normalU; }

    void setNormalSigma(qreal sigma) { m_normalSigma = sigma; }
    qreal normalSigma() const { return m_normalSigma; }

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
    qreal m_bernoulliProbability;
    int m_bernoulliCount;

    qreal m_normalU;
    qreal m_normalSigma;
};

#endif // CANVASVIEW_H
