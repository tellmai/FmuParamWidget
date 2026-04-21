#ifndef CUSTOMHEADERVIEW_H
#define CUSTOMHEADERVIEW_H

#include <QHeaderView>
#include <QSet>

#include "inputparammodel.h"

// 自定义表头负责绘制排序按钮、列宽热区，并区分点击排序和拖拽改列宽。
class CustomHeaderView : public QHeaderView
{
    Q_OBJECT

public:
    explicit CustomHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);

    /**
     * @brief SetSortableSections
     * @param sections 允许点击排序的列集合。
     */
    void SetSortableSections(const QSet<int> &sections);

    /**
     * @brief IsSortableSection
     * @param logicalIndex 表头逻辑列号。
     * @return 当前列是否允许排序。
     */
    bool IsSortableSection(int logicalIndex) const;

    /**
     * @brief AdvanceSortState
     * @param logicalIndex 被点击的表头逻辑列号。
     * @return 切换后的排序状态。
     */
    ParamSortMode AdvanceSortState(int logicalIndex);

    /**
     * @brief SortMode
     * @return 当前排序模式。
     */
    ParamSortMode SortMode() const;

    /**
     * @brief SortSection
     * @return 当前排序列号。
     */
    int SortSection() const;

signals:
    // 表头确认是一次有效排序点击后发出，避免拖拽列宽时误触排序。
    /**
     * @brief SortRequested
     * @param logicalIndex 请求排序的逻辑列号。
     * @param mode 排序模式。
     */
    void SortRequested(int logicalIndex, ParamSortMode mode);

protected:
    /**
     * @brief paintSection
     * @param painter 绘制对象。
     * @param rect 表头单元格区域。
     * @param logicalIndex 表头逻辑列号。
     */
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;

    /**
     * @brief sectionSizeFromContents
     * @param logicalIndex 表头逻辑列号。
     * @return 当前列内容建议尺寸。
     */
    QSize sectionSizeFromContents(int logicalIndex) const override;

    /**
     * @brief mousePressEvent
     * @param event 鼠标按下事件。
     */
    void mousePressEvent(QMouseEvent *event) override;

    /**
     * @brief mouseReleaseEvent
     * @param event 鼠标释放事件。
     */
    void mouseReleaseEvent(QMouseEvent *event) override;

    /**
     * @brief mouseMoveEvent
     * @param event 鼠标移动事件。
     */
    void mouseMoveEvent(QMouseEvent *event) override;

    /**
     * @brief leaveEvent
     * @param event 鼠标离开事件。
     */
    void leaveEvent(QEvent *event) override;

private:
    /**
     * @brief DrawSortButton
     * @param painter 绘制对象。
     * @param rect 排序按钮所在表头区域。
     * @param mode 当前排序模式。
     */
    void DrawSortButton(QPainter *painter, const QRect &rect, ParamSortMode mode) const;

    /**
     * @brief IsOnResizeHandle
     * @param pos 鼠标在表头中的位置。
     * @return 鼠标是否位于列宽调整热区。
     */
    bool IsOnResizeHandle(const QPoint &pos) const;

    QSet<int> m_sortableSections; // 只有集合里的列会显示排序按钮并响应点击。
    int m_currentSortSection;
    ParamSortMode m_currentSortMode;
    int m_pressedSection;
    bool m_pressedOnResizeHandle;
};

#endif // CUSTOMHEADERVIEW_H
