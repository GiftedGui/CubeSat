#ifndef WIDGET_H
#define WIDGET_H

#include "qcustomplot.h"

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QFile>
#include <QDebug>
#include <QTextStream>
#include <QStringList>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QDateTime>
#include <QLCDNumber>
#include <QTabWidget>
#include <QUdpSocket>


class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    QLabel *label1, *label2, *label3;
    QLabel *label4, *label5, *label6;
    QLabel *label7, *label8, *label9, *label10;
    QLabel *label11, *label12, *label13, *label14;

    QList<double> values1, values2, values3;
    QList<double> values4, values5, values6;
    QList<double> values7, values8, values9, values10;
    QList<double> values11, values12;//, values13; //, values14;

    QPushButton *btnStart;
    QTimer *timer;

    QCustomPlot *altitudePlot;
    QCustomPlot *temperaturePlot;
    QCustomPlot *gyroscopePlot;
    QCustomPlot *accelerometerPlot;

    QCPRange xRange, yRange;

    QDateTime startTime;
    int elapsedTimeInSeconds;
    QLCDNumber *elapsedTimeLCD;

    int currentIndex;

    QTabWidget *tabWidget;

    QVBoxLayout *gpsLayout, *bmeLayout;

    bool zoomPanEnabled = false;

    void createOverviewTab();
    void createGpsTab();
    void createAccelerometerTab();
    void createGyroscopeTab();
    void createBmeTab();

    QUdpSocket *udpSocket;

public slots:
    void reset();
    void updateLabels();
    void updateElapsedTime();
    void toggleZoomPanMode();
    void readPendingDatagrams();
    void updateLabelsWithData(double value1, double value2, double value3, double value4, double value5, double value6,
                              double value7, double value8, double value9, double value10, double value11, double value12);//,
                              //double value13); //, double value14);

};
#endif // WIDGET_H
