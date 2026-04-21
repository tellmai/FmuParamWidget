#ifndef INTERNALPARAMEDITORWIDGET_H
#define INTERNALPARAMEDITORWIDGET_H

#include <QIcon>
#include <QLineEdit>
#include <QModelIndex>
#include <QTreeView>
#include <QWidget>

#include "CustomHeaderView.h"
#include "internalparamdelegate.h"
#include "internalparammodel.h"

QT_BEGIN_NAMESPACE
class QResizeEvent;
class QShowEvent;
QT_END_NAMESPACE

class InternalParamEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InternalParamEditorWidget(QWidget *parent = nullptr);

    /**
     * @brief LoadDefaultData
     */
    void LoadDefaultData();

    /**
     * @brief ClearData
     */
    void ClearData();

protected:
    /**
     * @brief resizeEvent
     * @param event 尺寸变化事件。
     */
    void resizeEvent(QResizeEvent *event) override;

    /**
     * @brief showEvent
     * @param event 显示事件。
     */
    void showEvent(QShowEvent *event) override;

private slots:
    void OnExpandClicked(const QModelIndex &index);
    void OnFilterTextChanged(const QString &text);

private:
    QTreeView *m_treeView;
    InternalParamModel *m_model;
    InternalParamDelegate *m_delegate;
    CustomHeaderView *m_headerView;
    QLineEdit *m_filterEdit;

    /**
     * @brief SetupUI
     */
    void SetupUI();

    /**
     * @brief SetupConnections
     */
    void SetupConnections();

    /**
     * @brief ResizeColumnsToAvailableWidth
     */
    void ResizeColumnsToAvailableWidth();

    /**
     * @brief ToggleExpandForIndex
     * @param index 需要切换展开状态的节点索引。
     */
    void ToggleExpandForIndex(const QModelIndex &index);

    /**
     * @brief RestoreExpandedRows
     */
    void RestoreExpandedRows();

    /**
     * @brief RestoreExpandedRows
     * @param parentIndex 递归恢复展开状态的父节点索引。
     */
    void RestoreExpandedRows(const QModelIndex &parentIndex);

    /**
     * @brief ApplyFilter
     * @param parentIndex 递归过滤的父节点索引。
     * @param text 过滤文本。
     * @return 当前父节点下是否存在匹配项。
     */
    bool ApplyFilter(const QModelIndex &parentIndex, const QString &text);

    /**
     * @brief RowMatchesFilter
     * @param index 节点索引。
     * @param text 过滤文本。
     * @return 当前节点是否匹配过滤条件。
     */
    bool RowMatchesFilter(const QModelIndex &index, const QString &text) const;

    /**
     * @brief CreateSearchIcon
     * @return 搜索框左侧的搜索图标。
     */
    QIcon CreateSearchIcon() const;
};

#endif // INTERNALPARAMEDITORWIDGET_H
