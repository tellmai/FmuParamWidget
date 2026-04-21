#include <QApplication>

#include "paramsetwidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setStyle("Fusion");

    ParamCategoryContentData categoryData(QStringLiteral("fmu"),
                                          QStringList{QObject::tr("设备参数"), QObject::tr("飞行控制")});

    ParamSetWidget widget(categoryData);
    widget.show();

    return app.exec();
}
