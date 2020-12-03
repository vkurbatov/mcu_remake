#include <QtCore>
#include <QtGui>
#include <QApplication>
#include <QDialog>
#include <QLabel>

#include "synth_form.h"

int main(int argc, char* argv[])
{  

    QApplication app(argc, argv);

    synth_form form;

    form.show();

    return app.exec();
}
