#ifndef BORDEREDTREEVIEW_H
#define BORDEREDTREEVIEW_H

#include <QTreeView>

// QTreeView 的样式表边框在内容被裁剪或滚动条出现时，底边有时会被视口覆盖。
// 这个轻量子类在默认绘制完成后，额外在可视区域底部补一条网格线。
class BorderedTreeView : public QTreeView
{
    Q_OBJECT

public:
    explicit BorderedTreeView(QWidget *parent = nullptr);

protected:
    /**
     * @brief paintEvent
     * @param event 绘制事件。
     */
    void paintEvent(QPaintEvent *event) override;
};

#endif // BORDEREDTREEVIEW_H
