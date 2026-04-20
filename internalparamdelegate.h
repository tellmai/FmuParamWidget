#ifndef INTERNALPARAMDELEGATE_H
#define INTERNALPARAMDELEGATE_H

#include <QModelIndex>
#include <QStyledItemDelegate>

class InternalParamDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit InternalParamDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option, const QModelIndex &index) override;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;

signals:
    void ExpandClicked(const QModelIndex &index);
    void RowDoubleClicked(const QModelIndex &index);

private:
    bool IsSectionHeader(const QModelIndex &index) const;
    int DepthForIndex(const QModelIndex &index) const;
    QRect ArrowRect(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool HasChildren(const QModelIndex &index) const;
    bool IsLastChildInParent(const QModelIndex &index) const;
};

#endif // INTERNALPARAMDELEGATE_H
