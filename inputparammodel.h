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

    /**
     * @brief index
     * @param row 子节点行号。
     * @param column 列号。
     * @param parent 父节点索引。
     * @return 指定行列对应的模型索引。
     */
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief parent
     * @param index 当前节点索引。
     * @return 当前节点的父节点索引。
     */
    QModelIndex parent(const QModelIndex &index) const override;

    /**
     * @brief rowCount
     * @param parent 父节点索引。
     * @return 父节点下的子节点数量。
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief columnCount
     * @param parent 父节点索引。
     * @return 输入参数表格列数。
     */
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief data
     * @param index 单元格索引。
     * @param role 数据角色。
     * @return 指定角色下的单元格数据。
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief headerData
     * @param section 表头列号。
     * @param orientation 表头方向。
     * @param role 数据角色。
     * @return 指定角色下的表头数据。
     */
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    /**
     * @brief setData
     * @param index 单元格索引。
     * @param value 新值。
     * @param role 数据角色。
     * @return 数据是否写入成功。
     */
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    /**
     * @brief flags
     * @param index 单元格索引。
     * @return 单元格交互标志。
     */
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /**
     * @brief ToggleExpand
     * @param index 需要切换展开状态的节点索引。
     */
    void ToggleExpand(const QModelIndex &index);

    /**
     * @brief SortByColumn
     * @param column 排序列。
     * @param mode 排序模式。
     */
    void SortByColumn(int column, ParamSortMode mode); // 当前只开放名称列排序。

    /**
     * @brief ToJson
     * @return 当前模型数据的 JSON 对象。
     */
    QJsonObject ToJson() const;

    /**
     * @brief FromJson
     * @param json 用于恢复模型数据的 JSON 对象。
     */
    void FromJson(const QJsonObject &json);

    /**
     * @brief ToJsonString
     * @return 当前模型数据的 JSON 字符串。
     */
    QString ToJsonString() const;

    /**
     * @brief FromJsonString
     * @param jsonString 用于恢复模型数据的 JSON 字符串。
     * @return JSON 字符串是否解析并加载成功。
     */
    bool FromJsonString(const QString &jsonString);

    /**
     * @brief SetupModelData
     */
    void SetupModelData();

    /**
     * @brief Clear
     */
    void Clear();

    /**
     * @brief GetItem
     * @param index 模型索引。
     * @return 索引对应的参数节点。
     */
    ParamItem *GetItem(const QModelIndex &index) const;

private:
    QVector<ParamItem*> m_rootItems;

    /**
     * @brief AssignItemMetadata
     */
    void AssignItemMetadata();

    /**
     * @brief AssignItemMetadata
     * @param parent 需要递归处理的父节点。
     */
    void AssignItemMetadata(ParamItem *parent);

    // 递归排序同级节点，不打散父子层级。
    /**
     * @brief SortItems
     * @param items 同级节点列表。
     * @param column 排序列。
     * @param mode 排序模式。
     */
    void SortItems(QVector<ParamItem*> &items, int column, ParamSortMode mode);

    /**
     * @brief CompareItems
     * @param left 左侧比较节点。
     * @param right 右侧比较节点。
     * @param column 排序列。
     * @param mode 排序模式。
     * @return 比较结果，小于 0 表示 left 排在 right 前。
     */
    int CompareItems(const ParamItem *left, const ParamItem *right, int column, ParamSortMode mode) const;

    /**
     * @brief CompareValues
     * @param left 左侧比较节点。
     * @param right 右侧比较节点。
     * @return 值列比较结果。
     */
    int CompareValues(const ParamItem *left, const ParamItem *right) const;

    /**
     * @brief ValueTypeRank
     * @param type FMU 值类型。
     * @return 值类型排序优先级。
     */
    static int ValueTypeRank(FmuValueType type);

    /**
     * @brief BoolValue
     * @param value 布尔值文本。
     * @return 文本是否表示 true。
     */
    static bool BoolValue(const QString &value);

    /**
     * @brief NumericValue
     * @param value 数值文本。
     * @param ok 是否转换成功。
     * @return 转换后的数值。
     */
    static double NumericValue(const QString &value, bool *ok);
};

#endif // INPUTPARAMMODEL_H
