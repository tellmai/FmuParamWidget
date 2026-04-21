#include "stylesheetloader.h"

#include <QFile>

QString LoadStyleSheet(const QString &resourcePath)
{
    QFile styleFile(resourcePath);
    if (!styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }

    return QString::fromUtf8(styleFile.readAll());
}
