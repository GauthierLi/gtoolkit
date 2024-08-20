#include <QApplication>
#include <QWidget>
#include <QTimer>
#include <QTime>
#include <QLabel>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QMenu>
#include <QColorDialog>
#include <QSettings>

class FloatingClock : public QWidget {
    Q_OBJECT

public:
    FloatingClock(QWidget *parent = nullptr) : QWidget(parent) {
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
        setAttribute(Qt::WA_TranslucentBackground);

        QVBoxLayout *layout = new QVBoxLayout(this);
        timeLabel = new QLabel(this);
        layout->addWidget(timeLabel);
        layout->setContentsMargins(10, 10, 10, 10);

        loadSettings();  // 加载上次保存的设置
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

    void closeEvent(QCloseEvent *event) override {
        saveSettings();  // 保存设置
        QWidget::closeEvent(event);
    }

private slots:
    void updateClock() {
        timeLabel->setText(QTime::currentTime().toString("hh:mm:ss"));
    }

    void changeFontColor() {
        QColor color = QColorDialog::getColor(Qt::white, this, "Select Font Color");
        if (color.isValid()) {
            QString styleSheet = QString("QLabel { color : %1; font-size: 24px; }").arg(color.name());
            timeLabel->setStyleSheet(styleSheet);
            saveSettings();  // 保存设置
        }
    }

    void exitApplication() {
	saveSettings();
        QApplication::quit();  // 退出应用程序
    }

private:
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

    void saveSettings() {
    	QSettings settings("MyCompany", "FloatingClock");

    	// 提取颜色值
    	QString styleSheet = timeLabel->styleSheet();
    	QString colorStyle = styleSheet.split(":").value(1).split(";").value(0).trimmed(); // 提取颜色部分

    	settings.setValue("fontColor", colorStyle);
    	settings.setValue("position", pos());
    }


    void loadSettings() {
    	QSettings settings("MyCompany", "FloatingClock");

    	// 加载颜色设置
    	QString colorName = settings.value("fontColor", "#FFFFFF").toString(); // 默认值为白色

    	// 确保 colorName 是有效的颜色
   	QString styleSheet = QString("QLabel { color : %1; font-size: 24px; }").arg(colorName);
    	timeLabel->setStyleSheet(styleSheet);

    	QPoint position = settings.value("position", QPoint(200, 200)).toPoint(); // 默认位置
    	move(position);
    }


    QLabel *timeLabel;
    QPoint dragPosition;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    FloatingClock clock;
    clock.show();
    return app.exec();
}

#include "main.moc"

