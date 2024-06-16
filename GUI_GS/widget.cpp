#include "widget.h"
#include "qcustomplot.h"

#include <QLabel>
#include <QMovie>
#include <QPushButton>
#include <QMenu>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QString>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QGraphicsOpacityEffect>
#include <QDateTime>
#include <QLCDNumber>
#include <QTabWidget>
#include <QUdpSocket>



Widget::Widget(QWidget *parent)
    : QWidget(parent), currentIndex(0)
{
    this->setStyleSheet("background-color: #1e1b18;");
    setWindowTitle("TelemXplorer");
    resize(1600, 900);

    // Set up the tab widget
    tabWidget = new QTabWidget(this);

    // Create tabs
    createGpsTab();
    createAccelerometerTab();
    createGyroscopeTab();
    createBmeTab();
    // Create the bottom layout
    QHBoxLayout *bottomLayout = new QHBoxLayout();

    // TelemX logo at the bottom left corner
    QLabel *logo = new QLabel(this);
    QPixmap logoPixmap(":/telemxlogo.png");
    int logoWidth = 250;
    int logoHeight = 100;
    logo->setPixmap(logoPixmap.scaled(logoWidth, logoHeight, Qt::KeepAspectRatio));
    bottomLayout->addWidget(logo, 0, Qt::AlignLeft | Qt::AlignBottom);

    // Start button in the middle bottom
    QPushButton *btnReset = new QPushButton("RESET", this);
    btnReset->setFixedSize(150, 75);
    btnReset->setFont(QFont("Times", 36));
    btnReset->setStyleSheet(
        "QPushButton {"
        "   background-color: #007BFF;"    // Default background color
        "   color: white;"                  // Default text color
        "   border: 1px solid #0056b3;"     // Default border color
        "   border-radius: 5px;"            // Rounded corners
        "   font-weight: bold;"             // Default font weight
        "   font-size: 20px;"               // Default font size
        "}"
        "QPushButton:hover {"
        "   background-color: #0056b3;"     // Background color when hovered
        "}"
        "QPushButton:pressed {"           // Style when button is pressed
        "   background-color: #00468f;"     // Background color when pressed
        "}"
        );
    connect(btnReset, SIGNAL(clicked()), this, SLOT(reset()));
    bottomLayout->addWidget(btnReset, 0, Qt::AlignHCenter | Qt::AlignBottom);

    // Apply style sheet to the tab widget
    tabWidget->setStyleSheet(
        "QTabWidget::pane {"
        "   border: 1px solid dark;"     // Default border color
        "   border-top: 1px solid white;"    // Top border color of the tab widget
        "}"
        "QTabBar::tab:selected {"
        "   background-color: darkgreen;"        // Background color of the selected tab
        "   color: white;"                      // Text color of the selected tab
        "   font-weight: bold;"             // Default font weight
        "}"
        "QTabBar::tab:!selected {"
        "   background-color: dark;"         // Background color of the unselected tabs
        "   color: white;"                       // Text color of the unselected tabs
        "   font-weight: bold;"             // Default font weight
        "}"
        );

    // Timer in the bottom right corner
    elapsedTimeLCD = new QLCDNumber(this);
    elapsedTimeLCD->setDigitCount(5);
    elapsedTimeLCD->setSegmentStyle(QLCDNumber::Flat);
    elapsedTimeLCD->setFixedSize(150, 75);
    bottomLayout->addWidget(elapsedTimeLCD, 0, Qt::AlignRight | Qt::AlignBottom);

    // Create the main layout and add widgets
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(tabWidget);
    mainLayout->addLayout(bottomLayout);
    setLayout(mainLayout);

    qDebug() << "Widget setup done";
    // Bind the UDP socket to port
    udpSocket = new QUdpSocket(this);
    bool bindResult = udpSocket->bind(QHostAddress::Any, 5005);
    //udpSocket->bind(QHostAddress("192.168.4.255"), 8888, QUdpSocket::ShareAddress);
    if (bindResult) {
        qDebug() << "UDP socket bound to port";
    } else {
        qDebug() << "Failed to bind UDP socket to port";
    }
    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));

    startTime = QDateTime::currentDateTime();
    elapsedTimeInSeconds = 0;

    timer = new QTimer(this);
    //connect(timer, SIGNAL(timeout()), this, SLOT(updateLabels()));
    connect(timer, SIGNAL(timeout()), this, SLOT(updateElapsedTime()));
    timer->stop();
    timer->start(1000); // making it a higher frequency (lower delay) might make it more prone to crashes

    qDebug() << "SETUP DONE";
}


Widget::~Widget()
{



}

void Widget::createGpsTab()
{
    QColor Dark("#1e1b18");
    QWidget *gpsTab = new QWidget(); // Create GPS tab widget
    // Create a horizontal layout to separate the plot and the labels
    QHBoxLayout *hLayout = new QHBoxLayout(gpsTab);

    // Create a vertical layout for the labels on the left
    QVBoxLayout *vLabelLayout = new QVBoxLayout();

    int maxWidth = 200;

    // Add other widgets and labels to the GPS tab
    QLabel *gpsTitle = new QLabel("GPS", gpsTab);
    gpsTitle->setAlignment(Qt::AlignCenter);
    gpsTitle->setFont(QFont("Arial", 14, QFont::Bold));
    vLabelLayout->addWidget(gpsTitle, 0, Qt::AlignCenter);

    label8 = new QLabel("Latitude: No Data", gpsTab);
    label8->setFixedWidth(maxWidth);
    vLabelLayout->addWidget(label8, 0, Qt::AlignRight);

    label9 = new QLabel("Longitude: No Data", gpsTab);
    label9->setFixedWidth(maxWidth);
    vLabelLayout->addWidget(label9, 0, Qt::AlignRight);

    label10 = new QLabel("Altitude: No Data", gpsTab);
    label10->setFixedWidth(maxWidth);
    vLabelLayout->addWidget(label10, 0, Qt::AlignRight);

    label11 = new QLabel("Number of satellites: No Data", gpsTab);
    label11->setFixedWidth(maxWidth);
    vLabelLayout->addWidget(label11, 0, Qt::AlignRight);

    label12 = new QLabel("Speed over ground: No Data", gpsTab);
    label12->setFixedWidth(maxWidth);
    vLabelLayout->addWidget(label12, 0, Qt::AlignRight);

    // Add the label layout to the horizontal layout
    hLayout->addLayout(vLabelLayout);

    // Add altitude plot to the GPS tab
    altitudePlot = new QCustomPlot();
    altitudePlot->addGraph();
    //altitudePlot->addGraph();
    //altitudePlot->addGraph();
    altitudePlot->xAxis->setLabel("Time (s)");
    altitudePlot->yAxis->setLabel("Altitude (m)");
    altitudePlot->setBackground(QBrush(QColor(Dark)));
    altitudePlot->xAxis->setTickLabelColor(QColor(Qt::white));
    altitudePlot->yAxis->setTickLabelColor(QColor(Qt::white));
    altitudePlot->xAxis->setBasePen(QPen(QColor(Qt::white)));
    altitudePlot->yAxis->setBasePen(QPen(QColor(Qt::white)));
    altitudePlot->xAxis->setLabelColor(QColor(Qt::white));
    altitudePlot->yAxis->setLabelColor(QColor(Qt::white));
    //altitudePlot->graph(0)->setPen(QPen(QColor(Qt::cyan), 2)); // Set color and width of the plot
    //altitudePlot->graph(1)->setPen(QPen(QColor(Qt::red), 2)); // Set color and width of the plot
    //altitudePlot->graph(2)->setPen(QPen(QColor(Qt::green), 2)); // Set color and width of the plot
    altitudePlot->graph(0)->setPen(QPen(QColor(Qt::green), 2));

    //altitudePlot->graph(0)->setName("Latitude");
    //altitudePlot->graph(1)->setName("Longitude");
    //altitudePlot->graph(2)->setName("Altitude");
    altitudePlot->graph(0)->setName("Altitude");

    // Enable and configure legend
    altitudePlot->legend->setVisible(true);
    altitudePlot->legend->setBrush(QColor(Dark));  // Semi-transparent white background
    altitudePlot->legend->setBorderPen(QPen(QColor(Qt::white))); // White border

    // Customize legend text color
    QFont legendFont = font();  // Start with the same font as the rest of the application
    legendFont.setPointSize(10);  // Make the legend font size 10
    altitudePlot->legend->setFont(legendFont);
    altitudePlot->legend->setTextColor(QColor(Qt::white)); // White text color

    // Position legend at the top right corner
    altitudePlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop | Qt::AlignLeft);

    // Create a vertical layout to hold the plot and the button
    QVBoxLayout *plotLayout = new QVBoxLayout();

    // Create the pan/zoom button
    QPushButton *zoomPanButton = new QPushButton("+", this);
    zoomPanButton->setFixedSize(20, 20); // Set the size of the button
    zoomPanButton->setToolTip("Toggle Zoom/Pan Mode");
    // Set stylesheet to change the button's appearance
    zoomPanButton->setStyleSheet(
        "QPushButton {"
        "background-color: #45c039;"  // background
        "color: white;"               // text
        "border: 1px solid #388E3C;"  // border
        "border-radius: 5px;"         // Rounded corners
        "font-weight: bold;"          // text
        "font-size: 20px;"            // font size
        "}"
        "QPushButton:hover {"
        "background-color: #4CAF50;"  // when hovered
        "}"
        "QPushButton:pressed {"           // Style when button is pressed
        "background-color: #3cae31;"     // Background color when pressed
        "}"
        );

    // Connect the button's clicked signal to a slot
    connect(zoomPanButton, &QPushButton::clicked, this, &Widget::toggleZoomPanMode);

    // Add the plot and the button to the vertical layout
    plotLayout->addWidget(zoomPanButton, 0, Qt::AlignRight); // Align the button to the right
    plotLayout->addWidget(altitudePlot);

    // Add the vertical layout to the horizontal layout
    hLayout->addLayout(plotLayout, 1);

    // Set the horizontal layout on the GPS tab widget
    gpsTab->setLayout(hLayout);

    // Add the GPS tab to the tab widget
    tabWidget->addTab(gpsTab, "GPS");
}

void Widget::createAccelerometerTab()
{
    QColor Dark("#1e1b18");
    QWidget *accelerometerTab = new QWidget(); // Create accel tab widget
    // Create a horizontal layout to separate the plot and the labels
    QHBoxLayout *hLayout = new QHBoxLayout(accelerometerTab);

    // Create a vertical layout for the labels on the left
    QVBoxLayout *vLabelLayout = new QVBoxLayout();

    int maxWidth = 200;

    // Add other widgets and labels to the accel tab
    QLabel *accelerometerTitle = new QLabel("Accelerometer", accelerometerTab);
    accelerometerTitle->setAlignment(Qt::AlignCenter);
    accelerometerTitle->setFont(QFont("Arial", 14, QFont::Bold));
    vLabelLayout->addWidget(accelerometerTitle, 0, Qt::AlignCenter);

    label1 = new QLabel("AccelerationX: No Data", accelerometerTab);
    label1->setFixedWidth(maxWidth);
    vLabelLayout->addWidget(label1, 0, Qt::AlignRight);

    label2 = new QLabel("AccelerationY: No Data", accelerometerTab);
    label2->setFixedWidth(maxWidth);
    vLabelLayout->addWidget(label2, 0, Qt::AlignRight);

    label3 = new QLabel("AccelerationZ: No Data", accelerometerTab);
    label3->setFixedWidth(maxWidth);
    vLabelLayout->addWidget(label3, 0, Qt::AlignRight);

    // Add the label layout to the horizontal layout
    hLayout->addLayout(vLabelLayout);

    // Add altitude plot to the accel tab
    accelerometerPlot = new QCustomPlot();
    accelerometerPlot->addGraph();
    accelerometerPlot->addGraph();
    accelerometerPlot->addGraph();
    accelerometerPlot->xAxis->setLabel("Time (s)");
    accelerometerPlot->yAxis->setLabel("Altitude (m)");
    accelerometerPlot->setBackground(QBrush(QColor(Dark)));
    accelerometerPlot->xAxis->setTickLabelColor(QColor(Qt::white));
    accelerometerPlot->yAxis->setTickLabelColor(QColor(Qt::white));
    accelerometerPlot->xAxis->setBasePen(QPen(QColor(Qt::white)));
    accelerometerPlot->yAxis->setBasePen(QPen(QColor(Qt::white)));
    accelerometerPlot->xAxis->setLabelColor(QColor(Qt::white));
    accelerometerPlot->yAxis->setLabelColor(QColor(Qt::white));
    accelerometerPlot->graph(0)->setPen(QPen(QColor(Qt::cyan), 2));
    accelerometerPlot->graph(1)->setPen(QPen(QColor(Qt::red), 2));
    accelerometerPlot->graph(2)->setPen(QPen(QColor(Qt::green), 2));

    // Set names for each graph (to be displayed in the legend)
    accelerometerPlot->graph(2)->setName("AccelZ");
    accelerometerPlot->graph(1)->setName("AccelY");
    accelerometerPlot->graph(0)->setName("AccelX");

    // Enable and configure legend
    accelerometerPlot->legend->setVisible(true);
    accelerometerPlot->legend->setBrush(QColor(Dark));  // Semi-transparent white background
    accelerometerPlot->legend->setBorderPen(QPen(QColor(Qt::white))); // White border

    // Customize legend text color
    QFont legendFont = font();  // Start with the same font as the rest of the application
    legendFont.setPointSize(10);  // Make the legend font size 10
    accelerometerPlot->legend->setFont(legendFont);
    accelerometerPlot->legend->setTextColor(QColor(Qt::white)); // White text color

    // Position legend at the top right corner
    accelerometerPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop | Qt::AlignLeft);

    // Create a vertical layout to hold the plot and the button
    QVBoxLayout *plotLayout = new QVBoxLayout();

    // Create the pan/zoom button
    QPushButton *zoomPanButton = new QPushButton("+", this);
    zoomPanButton->setFixedSize(20, 20); // Set the size of the button
    zoomPanButton->setToolTip("Toggle Zoom/Pan Mode");
    // Set stylesheet to change the button's appearance
    zoomPanButton->setStyleSheet(
        "QPushButton {"
        "background-color: #45c039;"  // background
        "color: white;"               // text
        "border: 1px solid #388E3C;"  // border
        "border-radius: 5px;"         // Rounded corners
        "font-weight: bold;"          // text
        "font-size: 20px;"            // font size
        "}"
        "QPushButton:hover {"
        "background-color: #4CAF50;"  // when hovered
        "}"
        "QPushButton:pressed {"           // Style when button is pressed
        "background-color: #3cae31;"     // Background color when pressed
        "}"
        );

    // Connect the button's clicked signal to a slot
    connect(zoomPanButton, &QPushButton::clicked, this, &Widget::toggleZoomPanMode);

    // Add the plot and the button to the vertical layout
    plotLayout->addWidget(zoomPanButton, 0, Qt::AlignRight); // Align the button to the right
    plotLayout->addWidget(accelerometerPlot);

    // Add the vertical layout to the horizontal layout
    hLayout->addLayout(plotLayout, 1);

    // Set the horizontal layout on the accel tab widget
    accelerometerTab->setLayout(hLayout);

    // Add the accel tab to the tab widget
    tabWidget->addTab(accelerometerTab, "Accelerometer");
}

void Widget::createGyroscopeTab()
{
    QColor Dark("#1e1b18");
    QWidget *gyroscopeTab = new QWidget(); // Create gyro tab widget
    // Create a horizontal layout to separate the plot and the labels
    QHBoxLayout *hLayout = new QHBoxLayout(gyroscopeTab);

    // Create a vertical layout for the labels on the left
    QVBoxLayout *vLabelLayout = new QVBoxLayout();

    int maxWidth = 200;

    // Add other widgets and labels to the gyro tab
    QLabel *gyroscopeTitle = new QLabel("Gyroscope", gyroscopeTab);
    gyroscopeTitle->setAlignment(Qt::AlignCenter);
    gyroscopeTitle->setFont(QFont("Arial", 14, QFont::Bold));
    vLabelLayout->addWidget(gyroscopeTitle, 0, Qt::AlignCenter);

    label4 = new QLabel("GyroscopeX: No Data", gyroscopeTab);
    label4->setFixedWidth(maxWidth);
    vLabelLayout->addWidget(label4, 0, Qt::AlignRight);

    label5 = new QLabel("Gyroscope: No Data", gyroscopeTab);
    label5->setFixedWidth(maxWidth);
    vLabelLayout->addWidget(label5, 0, Qt::AlignRight);

    label6 = new QLabel("GyroscopeZ: No Data", gyroscopeTab);
    label6->setFixedWidth(maxWidth);
    vLabelLayout->addWidget(label6, 0, Qt::AlignRight);

    // Add the label layout to the horizontal layout
    hLayout->addLayout(vLabelLayout);

    // Add altitude plot to the gyro tab
    gyroscopePlot = new QCustomPlot();
    gyroscopePlot->addGraph();
    gyroscopePlot->addGraph();
    gyroscopePlot->addGraph();
    gyroscopePlot->xAxis->setLabel("Time (s)");
    gyroscopePlot->yAxis->setLabel("(rad/s)");
    gyroscopePlot->setBackground(QBrush(QColor(Dark)));
    gyroscopePlot->xAxis->setTickLabelColor(QColor(Qt::white));
    gyroscopePlot->yAxis->setTickLabelColor(QColor(Qt::white));
    gyroscopePlot->xAxis->setBasePen(QPen(QColor(Qt::white)));
    gyroscopePlot->yAxis->setBasePen(QPen(QColor(Qt::white)));
    gyroscopePlot->xAxis->setLabelColor(QColor(Qt::white));
    gyroscopePlot->yAxis->setLabelColor(QColor(Qt::white));
    gyroscopePlot->graph(0)->setPen(QPen(QColor(Qt::cyan), 2));
    gyroscopePlot->graph(1)->setPen(QPen(QColor(Qt::red), 2));
    gyroscopePlot->graph(2)->setPen(QPen(QColor(Qt::green), 2));

    // Set names for each graph (to be displayed in the legend)
    gyroscopePlot->graph(2)->setName("GyroZ");
    gyroscopePlot->graph(1)->setName("GyroY");
    gyroscopePlot->graph(0)->setName("GyroX");

    // Enable and configure legend
    gyroscopePlot->legend->setVisible(true);
    gyroscopePlot->legend->setBrush(QColor(Dark));  // Semi-transparent white background
    gyroscopePlot->legend->setBorderPen(QPen(QColor(Qt::white))); // White border

    // Customize legend text color
    QFont legendFont = font();  // Start with the same font as the rest of the application
    legendFont.setPointSize(10);  // Make the legend font size 10
    gyroscopePlot->legend->setFont(legendFont);
    gyroscopePlot->legend->setTextColor(QColor(Qt::white)); // White text color

    // Position legend at the top right corner
    gyroscopePlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop | Qt::AlignLeft);

    // Create a vertical layout to hold the plot and the button
    QVBoxLayout *plotLayout = new QVBoxLayout();

    // Create the pan/zoom button
    QPushButton *zoomPanButton = new QPushButton("+", this);
    zoomPanButton->setFixedSize(20, 20); // Set the size of the button
    zoomPanButton->setToolTip("Toggle Zoom/Pan Mode");
    // Set stylesheet to change the button's appearance
    zoomPanButton->setStyleSheet(
        "QPushButton {"
        "background-color: #45c039;"  // background
        "color: white;"               // text
        "border: 1px solid #388E3C;"  // border
        "border-radius: 5px;"         // Rounded corners
        "font-weight: bold;"          // text
        "font-size: 20px;"            // font size
        "}"
        "QPushButton:hover {"
        "background-color: #4CAF50;"  // when hovered
        "}"
        "QPushButton:pressed {"           // Style when button is pressed
        "background-color: #3cae31;"     // Background color when pressed
        "}"
        );

    // Connect the button's clicked signal to a slot
    connect(zoomPanButton, &QPushButton::clicked, this, &Widget::toggleZoomPanMode);

    // Add the plot and the button to the vertical layout
    plotLayout->addWidget(zoomPanButton, 0, Qt::AlignRight); // Align the button to the right
    plotLayout->addWidget(gyroscopePlot);

    // Add the vertical layout to the horizontal layout
    hLayout->addLayout(plotLayout, 1);

    // Set the horizontal layout on the gyro tab widget
    gyroscopeTab->setLayout(hLayout);

    // Add the gyro tab to the tab widget
    tabWidget->addTab(gyroscopeTab, "Gyroscope");
}

void Widget::createBmeTab()
{
    QColor Dark("#1e1b18");
    QWidget *bmeTab = new QWidget(); // Create BME tab widget
    // Create a horizontal layout to separate the plot and the labels
    QHBoxLayout *hLayout = new QHBoxLayout(bmeTab);

    // Create a vertical layout for the labels on the left
    QVBoxLayout *vLabelLayout = new QVBoxLayout();

    int maxWidth = 200;

    // Add other widgets and labels to the BME tab
    QLabel *bmeTitle = new QLabel("BME", bmeTab);
    bmeTitle->setAlignment(Qt::AlignCenter);
    bmeTitle->setFont(QFont("Arial", 14, QFont::Bold));
    vLabelLayout->addWidget(bmeTitle, 0, Qt::AlignCenter);

    label7 = new QLabel("Temperature: No Data", bmeTab);
    label7->setFixedWidth(maxWidth);
    vLabelLayout->addWidget(label7, 0, Qt::AlignRight);


    label13 = new QLabel("Pressure: No Data\nHumidity: No Data\nGas resistance: No Data", bmeTab);
    label13->setFixedWidth(maxWidth);
    vLabelLayout->addWidget(label13, 0, Qt::AlignRight);


    // Add the label layout to the horizontal layout
    hLayout->addLayout(vLabelLayout);

    // Add temperature plot to the BME tab
    temperaturePlot = new QCustomPlot();
    temperaturePlot->addGraph();
    temperaturePlot->xAxis->setLabel("Time (s)");
    temperaturePlot->yAxis->setLabel("Temperature (ºC)");
    temperaturePlot->setBackground(QBrush(QColor(Dark)));
    temperaturePlot->xAxis->setTickLabelColor(QColor(Qt::white));
    temperaturePlot->yAxis->setTickLabelColor(QColor(Qt::white));
    temperaturePlot->xAxis->setBasePen(QPen(QColor(Qt::white)));
    temperaturePlot->yAxis->setBasePen(QPen(QColor(Qt::white)));
    temperaturePlot->xAxis->setLabelColor(QColor(Qt::white));
    temperaturePlot->yAxis->setLabelColor(QColor(Qt::white));
    temperaturePlot->graph(0)->setPen(QPen(QColor(Qt::green), 2)); // Set color and width of the plot

    // Set names for each graph (to be displayed in the legend)
    temperaturePlot->graph(0)->setName("Temperature");

    // Enable and configure legend
    temperaturePlot->legend->setVisible(true);
    temperaturePlot->legend->setBrush(QColor(Dark));  // Semi-transparent white background
    temperaturePlot->legend->setBorderPen(QPen(QColor(Qt::white))); // White border

    // Customize legend text color
    QFont legendFont = font();  // Start with the same font as the rest of the application
    legendFont.setPointSize(10);  // Make the legend font size 10
    temperaturePlot->legend->setFont(legendFont);
    temperaturePlot->legend->setTextColor(QColor(Qt::white)); // White text color

    // Position legend at the top right corner
    temperaturePlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop | Qt::AlignLeft);

    // Create a vertical layout to hold the plot and the button
    QVBoxLayout *plotLayout = new QVBoxLayout();

    // Create the pan/zoom button
    QPushButton *zoomPanButton = new QPushButton("+", this);
    zoomPanButton->setFixedSize(20, 20); // Set the size of the button
    zoomPanButton->setToolTip("Toggle Zoom/Pan Mode");
    // Set stylesheet to change the button's appearance
    zoomPanButton->setStyleSheet(
        "QPushButton {"
        "background-color: #45c039;"  // background
        "color: white;"               // text
        "border: 1px solid #388E3C;"  // border
        "border-radius: 5px;"         // Rounded corners
        "font-weight: bold;"          // text
        "font-size: 20px;"            // font size
        "}"
        "QPushButton:hover {"
        "background-color: #4CAF50;"  // when hovered
        "}"
        "QPushButton:pressed {"           // Style when button is pressed
        "background-color: #3cae31;"     // Background color when pressed
        "}"
        );

    // Connect the button's clicked signal to a slot
    connect(zoomPanButton, &QPushButton::clicked, this, &Widget::toggleZoomPanMode);

    // Add the plot and the button to the vertical layout
    plotLayout->addWidget(zoomPanButton, 0, Qt::AlignRight); // Align the button to the right
    plotLayout->addWidget(temperaturePlot);

    // Add the vertical layout to the horizontal layout
    hLayout->addLayout(plotLayout, 1);

    // Set the horizontal layout on the GPS tab widget
    bmeTab->setLayout(hLayout);

    // Add the BME tab to the tab widget
    tabWidget->addTab(bmeTab, "BME");
}

void Widget::toggleZoomPanMode() {
    zoomPanEnabled = !zoomPanEnabled;

    if (zoomPanEnabled) {
        altitudePlot->setInteraction(QCP::iRangeDrag, true); // Enable dragging
        altitudePlot->setInteraction(QCP::iRangeZoom, true); // Enable zooming
        altitudePlot->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
        altitudePlot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);

        temperaturePlot->setInteraction(QCP::iRangeDrag, true); // Enable dragging
        temperaturePlot->setInteraction(QCP::iRangeZoom, true); // Enable zooming
        temperaturePlot->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
        temperaturePlot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);

        accelerometerPlot->setInteraction(QCP::iRangeDrag, true); // Enable dragging
        accelerometerPlot->setInteraction(QCP::iRangeZoom, true); // Enable zooming
        accelerometerPlot->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
        accelerometerPlot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);

        gyroscopePlot->setInteraction(QCP::iRangeDrag, true); // Enable dragging
        gyroscopePlot->setInteraction(QCP::iRangeZoom, true); // Enable zooming
        gyroscopePlot->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
        gyroscopePlot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
    } else {
        altitudePlot->setInteraction(QCP::iRangeDrag, false); // Disable dragging
        altitudePlot->setInteraction(QCP::iRangeZoom, false); // Disable zooming

        temperaturePlot->setInteraction(QCP::iRangeDrag, false); // Disable dragging
        temperaturePlot->setInteraction(QCP::iRangeZoom, false); // Disable zooming

        accelerometerPlot->setInteraction(QCP::iRangeDrag, false); // Disable dragging
        accelerometerPlot->setInteraction(QCP::iRangeZoom, false); // Disable zooming

        gyroscopePlot->setInteraction(QCP::iRangeDrag, false); // Disable dragging
        gyroscopePlot->setInteraction(QCP::iRangeZoom, false); // Disable zooming
    }
}

void Widget::reset()
{
    startTime = QDateTime::currentDateTime();
    elapsedTimeInSeconds = 0;

    // Clear the data from the plots
    altitudePlot->graph(0)->data()->clear();
    //altitudePlot->graph(1)->data()->clear();
    //altitudePlot->graph(2)->data()->clear();

    temperaturePlot->graph(0)->data()->clear();

    accelerometerPlot->graph(0)->data()->clear();
    accelerometerPlot->graph(1)->data()->clear();
    accelerometerPlot->graph(2)->data()->clear();

    gyroscopePlot->graph(0)->data()->clear();
    gyroscopePlot->graph(1)->data()->clear();
    gyroscopePlot->graph(2)->data()->clear();

    // Replot the graphs
    altitudePlot->replot();
    temperaturePlot->replot();
    accelerometerPlot->replot();
    gyroscopePlot->replot();

    // Reset the labels to their initial state
    label1->setText("AccelerationX: No Data");
    label2->setText("AccelerationY: No Data");
    label3->setText("AccelerationZ: No Data");
    label4->setText("GyroX: No Data");
    label5->setText("GyroY: No Data");
    label6->setText("GyroZ: No Data");
    label7->setText("Temperature: No Data");
    label8->setText("Latitude: No Data");
    label9->setText("Longitude: No Data");
    label10->setText("Altitude: No Data");
    label11->setText("Number of satellites: No Data");
    label12->setText("Speed over ground: No Data");
    label13->setText("Pressure: No Data\nHumidity: No Data\nGas resistance: No Data");
    //label14->setText("Gas Resistance: No Data");
}

void Widget::updateLabels()
{

}

void Widget::updateElapsedTime()
{
    // Calculate the elapsed time in (float) seconds
    elapsedTimeInSeconds = static_cast<float>(startTime.msecsTo(QDateTime::currentDateTime())) / 1000.0f;

    // Update the elapsed time in MM:SS format
    int minutes = static_cast<int>(elapsedTimeInSeconds) / 60;
    int seconds = static_cast<int>(elapsedTimeInSeconds) % 60;

    // Display the elapsed time in the LCD
    elapsedTimeLCD->display(QString("%1:%2").arg(minutes, 2, 10, QLatin1Char('0')).arg(seconds, 2, 10, QLatin1Char('0')));
}

void Widget::readPendingDatagrams()
{
    qDebug() << "readPendingDiagrams called";

    //connect(udpSocket, &QUdpSocket::readyRead, this, &Widget::readPendingDatagrams);

    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        qDebug() << "Received datagram:" << datagram;  // Debug print

        QString data(datagram);
        QStringList parts = data.split(",");
        qDebug() << "Split data into parts:" << parts;  // Debug print

        if (parts.size() >= 12) {
            bool ok1, ok2, ok3, ok4, ok5, ok6, ok7, ok8, ok9, ok10, ok11, ok12; //, ok13; //, ok14;
            double value1 = parts[0].toDouble(&ok1);
            double value2 = parts[1].toDouble(&ok2);
            double value3 = parts[2].toDouble(&ok3);
            double value4 = parts[3].toDouble(&ok4);
            double value5 = parts[4].toDouble(&ok5);
            double value6 = parts[5].toDouble(&ok6);
            double value7 = parts[6].toDouble(&ok7);
            double value8 = parts[7].toDouble(&ok8);
            double value9 = parts[8].toDouble(&ok9);
            double value10 = parts[9].toDouble(&ok10);
            double value11 = parts[10].toDouble(&ok11);
            double value12 = parts[11].toDouble(&ok12);
            //double value13 = parts[12].toDouble(&ok13);
            //double value14 = parts[13].toDouble(&ok14);

            if (ok1 && ok2 && ok3 && ok4 && ok5 && ok6 && ok7 && ok8 && ok9 && ok10 && ok11 && ok12) {// && ok13 && ok14) {
                qDebug() << "Parsed values:" << value1 << value2 << value3 << value4 << value5 << value6 << value7 << value8 << value9 << value10 << value11 << value12;//; << value13; // << value14;  // Debug print
                updateLabelsWithData(value1, value2, value3, value4, value5, value6, value7, value8, value9, value10, value11, value12);//, value13); //, value14);
                updateElapsedTime();
            } else {
                qDebug() << "Failed to convert some parts to double";
            }
        } else {
            qDebug() << "Received data does not have enough parts";
        }
    }
    qDebug() << "No pending datagrams";
}

void Widget::updateLabelsWithData(double value1, double value2, double value3, double value4, double value5, double value6,
                                  double value7, double value8, double value9, double value10, double value11, double value12)//,
                                  //double value13) //, double value14)
{
    qDebug() << "Updating labels with new data";  // Debug print

    label1->setText("AccelerationX: " + QString::number(value1));
    label2->setText("AccelerationY: " + QString::number(value2));
    label3->setText("AccelerationZ: " + QString::number(value3));
    label4->setText("GyroX: " + QString::number(value4));
    label5->setText("GyroY: " + QString::number(value5));
    label6->setText("GyroZ: " + QString::number(value6));
    label7->setText("Temperature: " + QString::number(value7));
    label8->setText("Latitude: " + QString::number(value8));
    label9->setText("Longitude: " + QString::number(value9));
    label10->setText("Altitude: " + QString::number(value10));
    label11->setText("Number of satellites: " + QString::number(value11));
    label12->setText("Speed over ground: " + QString::number(value12));
    //label13->setText("Pressure: No Data\nHumidity: No Data\nGas resistance: No Data");
    //label14->setText("Gas resistance: " + QString::number(value14));

    elapsedTimeInSeconds = static_cast<float>(startTime.msecsTo(QDateTime::currentDateTime())) / 1000.0f;

    // Update altitude plot
    //altitudePlot->graph(0)->addData(elapsedTimeInSeconds, value8);
    //altitudePlot->graph(1)->addData(elapsedTimeInSeconds, value9);
    //altitudePlot->graph(2)->addData(elapsedTimeInSeconds, value10);
    altitudePlot->graph(0)->addData(elapsedTimeInSeconds, value10);
    if (!zoomPanEnabled) {
        altitudePlot->rescaleAxes();
    }
    altitudePlot->replot();

    // Update temperature plot
    temperaturePlot->graph(0)->addData(elapsedTimeInSeconds, value7);
    if (!zoomPanEnabled) {
        temperaturePlot->rescaleAxes();
    }
    temperaturePlot->replot();

    // Update accel plot
    accelerometerPlot->graph(0)->addData(elapsedTimeInSeconds, value1);
    accelerometerPlot->graph(1)->addData(elapsedTimeInSeconds, value2);
    accelerometerPlot->graph(2)->addData(elapsedTimeInSeconds, value3);
    if (!zoomPanEnabled) {
        accelerometerPlot->rescaleAxes();
    }
    accelerometerPlot->replot();

    // Update gyro plot
    gyroscopePlot->graph(0)->addData(elapsedTimeInSeconds, value4);
    gyroscopePlot->graph(1)->addData(elapsedTimeInSeconds, value5);
    gyroscopePlot->graph(2)->addData(elapsedTimeInSeconds, value6);
    if (!zoomPanEnabled) {
        gyroscopePlot->rescaleAxes();
    }
    gyroscopePlot->replot();
}
