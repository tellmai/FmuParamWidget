#ifndef INPUTPARAMDELEGATE_H
#define INPUTPARAMDELEGATE_H

#include <QStyledItemDelegate>

class InputParamDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit InputParamDelegate(QObject *parent = nullptr);

    /**
     * @brief paint
     * @param painter 绘制对象。
     * @param option 单元格绘制选项。
     * @param index 单元格索引。
     */
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    /**
     * @brief sizeHint
     * @param option 单元格样式选项。
     * @param index 单元格索引。
     * @return 单元格建议尺寸。
     */
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    /**
     * @brief editorEvent
     * @param event 编辑器事件。
     * @param model 数据模型。
     * @param option 单元格样式选项。
     * @param index 单元格索引。
     * @return 事件是否已被处理。
     */
    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option, const QModelIndex &index) override;

    /**
     * @brief createEditor
     * @param parent 编辑器父窗口。
     * @param option 单元格样式选项。
     * @param index 单元格索引。
     * @return 当前单元格使用的编辑器控件。
     */
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    /**
     * @brief setEditorData
     * @param editor 编辑器控件。
     * @param index 单元格索引。
     */
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    /**
     * @brief setModelData
     * @param editor 编辑器控件。
     * @param model 数据模型。
     * @param index 单元格索引。
     */
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    /**
     * @brief updateEditorGeometry
     * @param editor 编辑器控件。
     * @param option 单元格样式选项。
     * @param index 单元格索引。
     */
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;

signals:
    /**
     * @brief ExpandClicked
     * @param index 被点击展开按钮的节点索引。
     */
    void ExpandClicked(const QModelIndex &index);

    /**
     * @brief RowDoubleClicked
     * @param index 被双击的节点索引。
     */
    void RowDoubleClicked(const QModelIndex &index);

private:
    /**
     * @brief IsSectionHeader
     * @param index 节点索引。
     * @return 当前节点是否为分组标题。
     */
    bool IsSectionHeader(const QModelIndex &index) const;

    /**
     * @brief DepthForIndex
     * @param index 节点索引。
     * @return 当前节点在参数树中的层级深度。
     */
    int DepthForIndex(const QModelIndex &index) const;

    /**
     * @brief ArrowRect
     * @param option 单元格样式选项。
     * @param index 节点索引。
     * @return 展开/折叠箭头的绘制区域。
     */
    QRect ArrowRect(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    /**
     * @brief HasChildren
     * @param index 节点索引。
     * @return 当前节点是否存在子节点。
     */
    bool HasChildren(const QModelIndex &index) const;

    /**
     * @brief IsLastChildInParent
     * @param index 节点索引。
     * @return 当前节点是否为父节点下的最后一个子节点。
     */
    bool IsLastChildInParent(const QModelIndex &index) const;
};

#endif // INPUTPARAMDELEGATE_H
