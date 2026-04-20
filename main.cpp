#include <QApplication>

#include "paramsetwidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setStyle("Fusion");

    ParamSetWidget widget;
    widget.show();

    return app.exec();
}
