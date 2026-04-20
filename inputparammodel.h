#ifndef INPUTPARAMMODEL_H
#define INPUTPARAMMODEL_H

#include <QAbstractItemModel>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVector>

#include "ParamItem.h"

enum class ParamSortMode {
    Default,
    Ascending,
    Descending
};

// InputParamModel 负责保存 FMU 参数树，并把树结构暴露给 QTreeView。
class InputParamModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Column {
        NameColumn = 0,
        VisibleColumn = 1,
        StartColumn = 2,
        UnitColumn = 3,
        ColumnCount = 4
    };

    explicit InputParamModel(QObject *parent = nullptr);
    ~InputParamModel() override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void ToggleExpand(const QModelIndex &index);
    void SortByColumn(int column, ParamSortMode mode); // 当前只开放名称列排序。

    QJsonObject ToJson() const;
    void FromJson(const QJsonObject &json);
    QString ToJsonString() const;
    bool FromJsonString(const QString &jsonString);

    void SetupModelData();
    void Clear();

    ParamItem *GetItem(const QModelIndex &index) const;

private:
    QVector<ParamItem*> m_rootItems;

    void AssignItemMetadata();
    void AssignItemMetadata(ParamItem *parent);
    // 递归排序同级节点，不打散父子层级。
    void SortItems(QVector<ParamItem*> &items, int column, ParamSortMode mode);
    int CompareItems(const ParamItem *left, const ParamItem *right, int column, ParamSortMode mode) const;
    int CompareValues(const ParamItem *left, const ParamItem *right) const;
    static int ValueTypeRank(FmuValueType type);
    static bool BoolValue(const QString &value);
    static double NumericValue(const QString &value, bool *ok);
};

#endif // INPUTPARAMMODEL_H
