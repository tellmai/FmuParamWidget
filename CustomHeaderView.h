#ifndef CUSTOMHEADERVIEW_H
#define CUSTOMHEADERVIEW_H

#include <QHeaderView>
#include <QSet>

#include "parammodel.h"

// 自定义表头负责绘制排序按钮、列宽热区，并区分点击排序和拖拽改列宽。
class CustomHeaderView : public QHeaderView
{
    Q_OBJECT

public:
    explicit CustomHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);

    void SetSortableSections(const QSet<int> &sections);
    bool IsSortableSection(int logicalIndex) const;
    ParamSortMode AdvanceSortState(int logicalIndex);
    ParamSortMode SortMode() const;
    int SortSection() const;

signals:
    // 表头确认是一次有效排序点击后发出，避免拖拽列宽时误触排序。
    void SortRequested(int logicalIndex, ParamSortMode mode);

protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;
    QSize sectionSizeFromContents(int logicalIndex) const override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    void DrawSortButton(QPainter *painter, const QRect &rect, ParamSortMode mode) const;
    bool IsOnResizeHandle(const QPoint &pos) const;

    QSet<int> m_sortableSections; // 只有集合里的列会显示排序按钮并响应点击。
    int m_currentSortSection;
    ParamSortMode m_currentSortMode;
    int m_pressedSection;
    bool m_pressedOnResizeHandle;
};

#endif // CUSTOMHEADERVIEW_H
