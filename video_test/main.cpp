#include <QtCore>
#include <QtGui>
#include <QApplication>
#include <QDialog>
#include <QLabel>

#include "video_form.h"

#include <iostream>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    video_form form;

    form.show();

    // delete form;

    return app.exec();
}
