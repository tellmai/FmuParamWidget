#include <QApplication>

#include "fmuparamwidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setStyle("Fusion");

    FmuParamWidget widget;
    widget.show();

    return app.exec();
}
