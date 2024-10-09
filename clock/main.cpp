#include <QApplication>
#include <QColorDialog>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QSettings>
#include <QTime>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QInputDialog>
#include <QMessageBox>
#include <QResizeEvent>
#include <QtMath>
#include <fstream>
#include <sstream>
#include <string>

class FloatingClock : public QWidget {
  Q_OBJECT

public:
  FloatingClock(QWidget *parent = nullptr) : QWidget(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);

    QVBoxLayout *layout = new QVBoxLayout(this);
    timeLabel = new QLabel(this);
    memoryLabel = new QLabel(this);  // 内存标签
    gpuLabel = new QLabel(this);     // 显存标签
    layout->addWidget(timeLabel);
    layout->addWidget(memoryLabel);  // 添加到布局
    layout->addWidget(gpuLabel);     // 添加到布局
    layout->setContentsMargins(10, 10, 10, 10);

    loadSettings(); 
    updateClock();

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &FloatingClock::updateClock);
    timer->start(1000); // 每秒更新一次
  }

protected:
  void mousePressEvent(QMouseEvent *event) override {
    if (event->button() == Qt::LeftButton) {
      dragPosition = event->globalPos() - frameGeometry().topLeft();
      event->accept();
    } else if (event->button() == Qt::RightButton) {
      showContextMenu(event->globalPos());
      event->accept();
    }
  }

  void mouseMoveEvent(QMouseEvent *event) override {
    if (event->buttons() & Qt::LeftButton) {
      move(event->globalPos() - dragPosition);
      event->accept();
    }
  }

  void resizeEvent(QResizeEvent *event) override {
    QWidget::resizeEvent(event);
    updateEventLabels();
  }

  void closeEvent(QCloseEvent *event) override {
    saveSettings();
    QWidget::closeEvent(event);
  }

private slots:
  void updateClock() {
    timeLabel->setText(QTime::currentTime().toString("hh:mm:ss"));

    // 更新内存和显存使用情况
    double memoryUsage = getMemoryUsage();
    double gpuUsage = getGPUMemoryUsage();
    
    // 根据使用率设置内存颜色
    QString memoryColor = getColor(memoryUsage);
    memoryLabel->setText(QString("Memory Usage: %1%").arg(memoryUsage, 0, 'f', 2));
    memoryLabel->setStyleSheet(QString("color: %1; font-size: 18px;").arg(memoryColor));
    
    // 根据使用率设置显存颜色
    QString gpuColor = getColor(gpuUsage);
    gpuLabel->setText(QString("GPU Usage: %1%").arg(gpuUsage, 0, 'f', 2));
    gpuLabel->setStyleSheet(QString("color: %1; font-size: 18px;").arg(gpuColor));

    updateEventLabels();
  }

  void changeFontColor() {
    QColor color = QColorDialog::getColor(Qt::white, this, "Select Font Color");
    if (color.isValid()) {
      QString styleSheet = QString("QLabel { color : %1; font-size: 24px; }").arg(color.name());
      timeLabel->setStyleSheet(styleSheet);
      memoryLabel->setStyleSheet(styleSheet);  // 应用到内存标签
      gpuLabel->setStyleSheet(styleSheet);     // 应用到显存标签
      saveSettings();
    }
  }

  void exitApplication() {
    saveSettings();
    QApplication::quit();
  }

  void showContextMenu(const QPoint &pos) {
    QMenu contextMenu(tr("Context menu"), this);

    QAction actionChangeColor("Change Font Color", this);
    connect(&actionChangeColor, &QAction::triggered, this, &FloatingClock::changeFontColor);

    QAction actionExit("Exit", this);
    connect(&actionExit, &QAction::triggered, this, &FloatingClock::exitApplication);

    contextMenu.addAction(&actionChangeColor);
    contextMenu.addAction(&actionExit);
    contextMenu.exec(pos);
  }

  // 内存和显存的读取函数
  double getMemoryUsage() {
    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    long long totalMemory = 0;
    long long availableMemory = 0;

    while (std::getline(meminfo, line)) {
      std::istringstream iss(line);
      std::string key;
      long long value;
      std::string unit;

      iss >> key >> value >> unit;

      if (key == "MemTotal:") {
        totalMemory = value;
      } else if (key == "MemAvailable:") {
        availableMemory = value;
      }

      if (totalMemory > 0 && availableMemory > 0) {
        break;
      }
    }

    long long usedMemory = totalMemory - availableMemory;
    return static_cast<double>(usedMemory) / totalMemory * 100;
  }

  double getGPUMemoryUsage() {
    std::string cmd = "nvidia-smi --query-gpu=memory.used,memory.total --format=csv,nounits,noheader";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return 0.0;

    char buffer[128];
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
      result += buffer;
    }
    pclose(pipe);

    double used = 0.0, total = 0.0;
    size_t commaPos = result.find(',');
    if (commaPos != std::string::npos) {
      used = std::stod(result.substr(0, commaPos));
      total = std::stod(result.substr(commaPos + 1));
    }

    if (total > 0) {
      return (used / total) * 100;
    } else {
      return 0.0;
    }
  }

  // 根据使用率返回颜色
  QString getColor(double usage) {
    if (usage < 40) {
      return "green";
    } else if (usage >= 40 && usage <= 80) {
      return "#DAA520";  // 柔和的黄色
    } else {
      return "red";
    }
  }

private:
  QLabel *timeLabel;
  QLabel *memoryLabel;  // 新增：内存标签
  QLabel *gpuLabel;     // 新增：显存标签
  QPoint dragPosition;
  QList<QPair<QString, QString>> events;
  QList<QLabel*> eventLabels;

  void saveSettings() {
    QSettings settings("MyCompany", "FloatingClock");
    QString styleSheet = timeLabel->styleSheet();
    QString colorStyle = styleSheet.split(":").value(1).split(";").value(0).trimmed();
    settings.setValue("fontColor", colorStyle);
    settings.setValue("position", pos());
  }

  void loadSettings() {
    QSettings settings("MyCompany", "FloatingClock");
    QString colorName = settings.value("fontColor", "#FFFFFF").toString();
    QString styleSheet = QString("QLabel { color : %1; font-size: 24px; }").arg(colorName);
    timeLabel->setStyleSheet(styleSheet);
    memoryLabel->setStyleSheet(styleSheet);
    gpuLabel->setStyleSheet(styleSheet);

    QPoint position = settings.value("position", QPoint(200, 200)).toPoint();
    move(position);
  }

  void updateEventLabels() {
    // Remove all previous event labels
    for (QLabel *label : eventLabels) {
      label->deleteLater();
    }
    eventLabels.clear();

    int eventCount = events.size();
    if (eventCount == 0) {
      return; // No events to display
    }

    int radius = 100;                 // Radius of the circular arrangement
    int angleStep = 360 / eventCount; // Angle between each label

    for (int i = 0; i < eventCount; ++i) {
      QLabel *eventLabel = new QLabel(this);
      eventLabel->setText(events[i].first); // Use the event title
      eventLabel->setStyleSheet("QLabel { font-size: 14px; color: black; }");

      // Calculate position
      double angle = i * angleStep * M_PI / 180; // Convert angle to radians
      int centerX = timeLabel->geometry().center().x();
      int centerY = timeLabel->geometry().center().y();
      int x = centerX + radius * qCos(angle) -
              eventLabel->width() / 2; // Adjust X position
      int y = centerY + radius * qSin(angle) -
              eventLabel->height() / 2; // Adjust Y position
      eventLabel->setGeometry(
          x, y, eventLabel->sizeHint().width(),
          eventLabel->sizeHint().height()); // Set position and size

      eventLabel->raise(); // Ensure the label is above the timeLabel
      eventLabel->show();
      eventLabels.append(eventLabel);
    }
  }
};

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  FloatingClock clock;
  clock.show();
  return app.exec();
}

#include "main.moc"
