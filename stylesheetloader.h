#ifndef STYLESHEETLOADER_H
#define STYLESHEETLOADER_H

#include <QString>

/**
 * @brief LoadStyleSheet
 * @param resourcePath qss 资源路径。
 * @return qss 文件内容，读取失败时返回空字符串。
 */
QString LoadStyleSheet(const QString &resourcePath);

#endif // STYLESHEETLOADER_H
