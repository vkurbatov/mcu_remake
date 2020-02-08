#include <QtCore>
#include <QtGui>
#include <QApplication>
#include <QDialog>
#include <QLabel>

#include "video_form.h"

#include <iostream>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavfilter/avfilter.h>
}

#include "media/common/media_control_parameter.h"

int main(int argc, char* argv[])
{  
    avcodec_register_all();
    avformat_network_init();
    avdevice_register_all();
    avfilter_register_all();
    av_register_all();

    //base::test();

    core::media::control_parameter_test();

    QApplication app(argc, argv);

    video_form form;

    form.show();

    // delete form;

    return app.exec();
}
