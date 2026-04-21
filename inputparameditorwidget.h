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

    QJsonObject GetModelData() const;
    QString GetModelDataString() const;
    bool SetModelData(const QJsonObject &json);
    bool SetModelDataFromString(const QString &jsonString);

    void LoadDefaultData();
    void ClearData();

public slots:
    void SaveToFile(const QString &filePath);
    void LoadFromFile(const QString &filePath);

protected:
    void resizeEvent(QResizeEvent *event) override;
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


    void SetupUI();
    void SetupConnections();
    void SetupModelData();
    void ResizeColumnsToAvailableWidth();
    void ToggleExpandForIndex(const QModelIndex &index);
    // 模型 reset 或排序后，按 ParamItem::isExpanded 恢复视图展开状态。
    void RestoreExpandedRows();
    void RestoreExpandedRows(const QModelIndex &parentIndex);
    // 过滤时保留命中的父节点和子节点，避免层级上下文丢失。
    bool ApplyFilter(const QModelIndex &parentIndex, const QString &text);
    bool RowMatchesFilter(const QModelIndex &index, const QString &text) const;
    QIcon CreateSearchIcon() const;
};

#endif // INPUTPARAMEDITORWIDGET_H
