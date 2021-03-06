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
    void prepare_image2();

private slots:
    void on_pushButton_clicked();
    void on_update();

    void on_cbScalingMethod_currentIndexChanged(int index);

    void on_cbScaling_currentIndexChanged(int index);

    void on_cbResoulution_activated(const QString &arg1);

    void on_cbResoulution_activated(int index);

    void on_cbControlList_activated(int index);

    void on_slControl_actionTriggered(int action);

    void on_slControl_sliderMoved(int position);

    void on_cbAspectRatio_currentIndexChanged(int index);

    void on_pushButton_2_clicked();

    void on_cbControlList_activated(const QString &arg1);

    void on_teControl_selectionChanged();

    void on_teControl_textChanged();

    void on_cbbControl_currentIndexChanged(const QString &arg1);

    void on_cbControl_clicked(bool checked);

    void on_spMCU_valueChanged(int arg1);

protected:
    void keyPressEvent(QKeyEvent *key_event);
    void keyReleaseEvent(QKeyEvent *key_event);

private:
    Ui::video_form *ui;
    video_surface m_surface;

    void test1();


    // QWidget interface
protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif // VIDEO_FORM_H
