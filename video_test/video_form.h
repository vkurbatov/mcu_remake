#ifndef VIDEO_FORM_H
#define VIDEO_FORM_H

#include <QMainWindow>
#include "video_surface.h"

namespace Ui {
class video_form;
}

class video_form : public QMainWindow
{
    Q_OBJECT

public:
    explicit video_form(QWidget *parent = 0);
    ~video_form();

    void prepare_image();

private slots:
    void on_pushButton_clicked();
    void on_update();

    void on_cbScalingMethod_currentIndexChanged(int index);

    void on_cbScaling_currentIndexChanged(int index);

private:
    Ui::video_form *ui;
    video_surface m_surface;


    // QWidget interface
protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif // VIDEO_FORM_H
