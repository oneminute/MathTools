#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QActionGroup;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected slots:
    void onApply(bool checked);
    void onToolsGroupTriggered(QAction* action);
    void onActionGenerate(bool checked = false);
    void onActionOpenImage(bool checked = false);
    void showDistribution(bool ckecked = false);

    void onComboBoxDistributionTypeChanged(int index);

private:
    Ui::MainWindow *ui;

    QActionGroup* m_toolsGroup;
};
#endif // MAINWINDOW_H
