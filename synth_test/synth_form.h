#ifndef SYNTH_FORM_H
#define SYNTH_FORM_H

#include <QMainWindow>

namespace Ui {
class synth_form;
}

class synth_form : public QMainWindow
{
    Q_OBJECT

public:
    explicit synth_form(QWidget *parent = 0);
    ~synth_form();

    virtual bool eventFilter(QObject *watched, QEvent *event) override;
    

private slots:


protected:


private:
    Ui::synth_form *ui;

    // QWidget interface
protected:

private slots:
    void on_btPlay_clicked();
};

#endif // VIDEO_FORM_H
