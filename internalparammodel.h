#ifndef INTERNALPARAMMODEL_H
#define INTERNALPARAMMODEL_H

#include <QAbstractItemModel>
#include <QVector>

#include "ParamItem.h"
#include "inputparammodel.h"

class InternalParamModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Column {
        ParamColumn = 0,
        ValueColumn = 1,
        UnitColumn = 2,
        DescriptionColumn = 3,
        ColumnCount = 4
    };

    explicit InternalParamModel(QObject *parent = nullptr);
    ~InternalParamModel() override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void ToggleExpand(const QModelIndex &index);
    void SortByColumn(int column, ParamSortMode mode);
    void SetupModelData();
    void Clear();

    ParamItem *GetItem(const QModelIndex &index) const;

private:
    QVector<ParamItem*> m_rootItems;

    void AppendRoot(ParamItem *item);
    void AssignItemMetadata();
    void AssignItemMetadata(ParamItem *parent);
    void SortItems(QVector<ParamItem*> &items, int column, ParamSortMode mode);
    int CompareItems(const ParamItem *left, const ParamItem *right, int column, ParamSortMode mode) const;
    int CompareValues(const ParamItem *left, const ParamItem *right) const;
    static int ValueTypeRank(FmuValueType type);
    static bool BoolValue(const QString &value);
    static double NumericValue(const QString &value, bool *ok);
};

#endif // INTERNALPARAMMODEL_H
