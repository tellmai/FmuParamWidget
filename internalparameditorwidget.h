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

    void LoadDefaultData();
    void ClearData();

protected:
    void resizeEvent(QResizeEvent *event) override;
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

    void SetupUI();
    void SetupConnections();
    void ResizeColumnsToAvailableWidth();
    void ToggleExpandForIndex(const QModelIndex &index);
    void RestoreExpandedRows();
    void RestoreExpandedRows(const QModelIndex &parentIndex);
    bool ApplyFilter(const QModelIndex &parentIndex, const QString &text);
    bool RowMatchesFilter(const QModelIndex &index, const QString &text) const;
    QIcon CreateSearchIcon() const;
};

#endif // INTERNALPARAMEDITORWIDGET_H
