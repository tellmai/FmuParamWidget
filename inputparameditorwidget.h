#ifndef INPUTPARAMEDITORWIDGET_H
#define INPUTPARAMEDITORWIDGET_H

#include <QLabel>
#include <QLineEdit>
#include <QIcon>
#include <QJsonObject>
#include <QModelIndex>
#include <QTreeView>
#include <QWidget>

#include "CustomHeaderView.h"
#include "inputparamdelegate.h"
#include "inputparammodel.h"

QT_BEGIN_NAMESPACE
class QResizeEvent;
class QShowEvent;
QT_END_NAMESPACE

// 组合搜索框、表头、树模型和 m_delegate 的输入参数编辑主控件。
class InputParamEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InputParamEditorWidget(QWidget *parent = nullptr);

    /**
     * @brief GetModelData
     * @return 当前输入参数模型数据。
     */
    QJsonObject GetModelData() const;

    /**
     * @brief GetModelDataString
     * @return 当前输入参数模型数据的 JSON 字符串。
     */
    QString GetModelDataString() const;

    /**
     * @brief SetModelData
     * @param json 输入参数模型数据。
     * @return 数据是否加载成功。
     */
    bool SetModelData(const QJsonObject &json);

    /**
     * @brief SetModelDataFromString
     * @param jsonString 输入参数模型数据的 JSON 字符串。
     * @return 数据是否解析并加载成功。
     */
    bool SetModelDataFromString(const QString &jsonString);

    /**
     * @brief LoadDefaultData
     */
    void LoadDefaultData();

    /**
     * @brief ClearData
     */
    void ClearData();

public slots:
    /**
     * @brief SaveToFile
     * @param filePath 保存目标文件路径。
     */
    void SaveToFile(const QString &filePath);

    /**
     * @brief LoadFromFile
     * @param filePath 读取源文件路径。
     */
    void LoadFromFile(const QString &filePath);

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
    void OnRowDoubleClicked(const QModelIndex &index);
    void OnFilterTextChanged(const QString &text);

private:
    QTreeView *m_treeView;
    InputParamModel *m_model;
    InputParamDelegate *m_delegate;
    CustomHeaderView *m_headerView;
    QLineEdit *m_filterEdit;
    QLabel *m_filterLabel;

    /**
     * @brief SetupUI
     */
    void SetupUI();

    /**
     * @brief SetupConnections
     */
    void SetupConnections();

    /**
     * @brief SetupModelData
     */
    void SetupModelData();

    /**
     * @brief ResizeColumnsToAvailableWidth
     */
    void ResizeColumnsToAvailableWidth();

    /**
     * @brief ToggleExpandForIndex
     * @param index 需要切换展开状态的节点索引。
     */
    void ToggleExpandForIndex(const QModelIndex &index);

    // 模型 reset 或排序后，按 ParamItem::isExpanded 恢复视图展开状态。
    /**
     * @brief RestoreExpandedRows
     */
    void RestoreExpandedRows();

    /**
     * @brief RestoreExpandedRows
     * @param parentIndex 递归恢复展开状态的父节点索引。
     */
    void RestoreExpandedRows(const QModelIndex &parentIndex);

    // 过滤时保留命中的父节点和子节点，避免层级上下文丢失。
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

#endif // INPUTPARAMEDITORWIDGET_H
