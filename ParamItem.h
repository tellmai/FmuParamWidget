#ifndef PARAMITEM_H
#define PARAMITEM_H

#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QVector>
#include <QtAlgorithms>

// 单元格编辑器类型，delegate 会根据它创建不同的编辑控件。（TODO需要根据仿真具体的数据类型修改）
enum class ParamDataType {
    LineEdit,
    CheckBox,
    ComboBox,
    SpinBox,
    DoubleSpinBox
};

// FMU 参数值的语义类型，用于值列排序时先按类型分组，再按具体值排序。
enum class FmuValueType {
    Boolean,
    Int8,
    Int16,
    Int32,
    Int64,
    Float,
    Double,
    Char,
    String,
    ClassifiedArray,
    Enumeration,
    Unknown
};

struct ParamItem {
    // 参数名称，对应表格中的“名称”或“参数”列。
    QString m_param;
    // 参数当前值，对应输入参数的“开始”列或内部参数的“值”列。
    QString m_value;
    // 参数单位，对应表格中的“单位”列。
    QString m_unit;
    // 参数描述，对应内部参数表格中的“描述”列。
    QString m_description;
    // 是否为分组/标题节点；标题节点一般不可编辑，可包含子节点。
    bool m_isSectionHeader;
    // 当前节点是否展开；排序、过滤或刷新视图后用它恢复展开状态。
    bool m_isExpanded;
    // 预留的斜坡/序号索引字段；没有索引时使用 -1。
    int m_rampIndex;
    // 当前节点的值单元格是否允许编辑。
    bool m_editable;
    // 值单元格进入编辑态时使用的编辑器类型。
    ParamDataType m_dataType;
    // 输入参数“可见性”列的勾选状态。
    bool m_visible;
    // FMU 值的语义类型，用于按类型和值进行排序。
    FmuValueType m_valueType;
    // 子节点列表，用于构造多级参数树。
    QVector<ParamItem*> m_children;
    // 父节点指针，QAbstractItemModel::parent() 需要通过它向上查找。
    ParamItem *m_parentItem;
    // 同级节点加载时的初始顺序，默认排序时用它恢复原始顺序。
    int m_originalOrder;

    ParamItem(const QString &p = QString(),
              const QString &v = QString(),
              const QString &u = QString(),
              const QString &d = QString(),
              bool isHeader = false,
              bool expanded = false,
              int idx = -1,
              bool edit = false,
              ParamDataType editorType = ParamDataType::LineEdit,
              bool isVisible = false,
              FmuValueType fmuType = FmuValueType::String)
        : m_param(p),
          m_value(v),
          m_unit(u),
          m_description(d),
          m_isSectionHeader(isHeader),
          m_isExpanded(expanded),
          m_rampIndex(idx),
          m_editable(edit),
          m_dataType(editorType),
          m_visible(isVisible),
          m_valueType(fmuType),
          m_parentItem(nullptr),
          m_originalOrder(0)
    {}

    ~ParamItem()
    {
        qDeleteAll(m_children);
    }

    /**
     * @brief AppendChild
     * @param child 需要追加的子参数节点。
     */
    void AppendChild(ParamItem *child)
    {
        if (!child) {
            return;
        }

        // 插入时维护父指针和默认顺序，避免排序后丢失层级关系。
        child->m_parentItem = this;
        child->m_originalOrder = m_children.size();
        m_children.append(child);
    }

    /**
     * @brief DefaultValueTypeFor
     * @param editorType 单元格编辑器类型。
     * @return 编辑器类型对应的默认 FMU 值类型。
     */
    static FmuValueType DefaultValueTypeFor(ParamDataType editorType)
    {
        switch (editorType) {
        case ParamDataType::CheckBox:
            return FmuValueType::Boolean;
        case ParamDataType::SpinBox:
            return FmuValueType::Int32;
        case ParamDataType::DoubleSpinBox:
            return FmuValueType::Double;
        case ParamDataType::ComboBox:
            return FmuValueType::Enumeration;
        case ParamDataType::LineEdit:
        default:
            return FmuValueType::String;
        }
    }

    /**
     * @brief ToJson
     * @return 当前参数节点及其子节点的 JSON 对象。
     */
    QJsonObject ToJson() const
    {
        QJsonObject json;
        json["param"] = m_param;
        json["value"] = m_value;
        json["unit"] = m_unit;
        json["description"] = m_description;
        json["isSectionHeader"] = m_isSectionHeader;
        json["isExpanded"] = m_isExpanded;
        json["rampIndex"] = m_rampIndex;
        json["editable"] = m_editable;
        json["dataType"] = static_cast<int>(m_dataType);
        json["visible"] = m_visible;
        json["valueType"] = static_cast<int>(m_valueType);
        json["originalOrder"] = m_originalOrder;

        if (!m_children.isEmpty()) {
            // 递归保存子节点，确保展开层级可以完整还原。
            QJsonArray childrenArray;
            for (const ParamItem *child : m_children) {
                childrenArray.append(child->ToJson());
            }
            json["children"] = childrenArray;
        }

        return json;
    }

    /**
     * @brief FromJson
     * @param json 用于创建参数节点的 JSON 对象。
     * @return 根据 JSON 创建的参数节点。
     */
    static ParamItem *FromJson(const QJsonObject &json)
    {
        // 旧数据可能没有 valueType 字段，因此按编辑器类型推导默认类型。
        const auto editorType = static_cast<ParamDataType>(json["dataType"].toInt(0));
        const auto fmuType = json.contains("valueType")
            ? static_cast<FmuValueType>(json["valueType"].toInt(static_cast<int>(DefaultValueTypeFor(editorType))))
            : DefaultValueTypeFor(editorType);

        ParamItem *item = new ParamItem(
            json["param"].toString(),
            json["value"].toString(),
            json["unit"].toString(),
            json["description"].toString(),
            json.contains("isSectionHeader") ? json["isSectionHeader"].toBool() : json["IsSectionHeader"].toBool(),
            json["isExpanded"].toBool(),
            json["rampIndex"].toInt(),
            json["editable"].toBool(),
            editorType,
            json["visible"].toBool(false),
            fmuType);

        item->m_originalOrder = json["originalOrder"].toInt(0);

        if (json.contains("children") && json["children"].isArray()) {
            const QJsonArray childrenArray = json["children"].toArray();
            for (const QJsonValue &childValue : childrenArray) {
                if (childValue.isObject()) {
                    item->AppendChild(FromJson(childValue.toObject()));
                }
            }
        }

        return item;
    }
};

Q_DECLARE_METATYPE(ParamItem*)

#endif // PARAMITEM_H
