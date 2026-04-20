#include "borderedtreeview.h"

#include <QPainter>
#include <QPaintEvent>
#include <QScrollBar>

BorderedTreeView::BorderedTreeView(QWidget *parent)
    : QTreeView(parent)
{}

void BorderedTreeView::paintEvent(QPaintEvent *event)
{
    QTreeView::paintEvent(event);

    // 当内容高度超过视口时，最后一行的底部网格线可能被裁剪在视口外。
    // 这里在 viewport 内侧补线，确保用户看到的表格底部始终有边界。
    QPainter painter(viewport());
    painter.setPen(QColor(204, 204, 204));

    const int y = viewport()->height() - 1;
    const int right = viewport()->width() - 1;
    painter.drawLine(0, y, right, y);

    // 竖向滚动条会占据表格右侧，viewport 补线只覆盖内容区。
    // 滚动条可见时再在滚动条底部补一段线，让底边视觉上贯穿整张表格。
    if (verticalScrollBar() && verticalScrollBar()->isVisible()) {
        QPainter scrollPainter(verticalScrollBar());
        scrollPainter.setPen(QColor(204, 204, 204));
        const int scrollY = verticalScrollBar()->height() - 1;
        scrollPainter.drawLine(0, scrollY, verticalScrollBar()->width(), scrollY);
    }
}
