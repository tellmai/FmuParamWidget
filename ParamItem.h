#ifndef PARAMITEM_H
#define PARAMITEM_H

#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QVector>
#include <QtAlgorithms>

// 单元格编辑器类型，m_delegate 根据它创建不同的编辑控件。
enum class ParamDataType {
    LineEdit,
    CheckBox,
    ComboBox,
    SpinBox,
    DoubleSpinBox
};

// FMU 起始值的语义类型，用于保留后续按类型排序的扩展能力。
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
    QString m_param;
    QString m_value;
    QString m_unit;
    QString m_description;
    bool m_isSectionHeader;
    bool m_isExpanded;
    int m_rampIndex;
    bool m_editable;
    ParamDataType m_dataType;
    bool m_visible;
    FmuValueType m_valueType;
    QVector<ParamItem*> m_children;
    ParamItem *m_parentItem;  // 标准树模型需要反向找到父节点。
    int m_originalOrder;      // 默认排序时恢复同级节点的初始顺序。

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

    void AppendChild(ParamItem *child)
    {
        if (!child) {
            return;
        }

        // 在插入时维护父指针和默认顺序，避免排序后丢失层级关系。
        child->m_parentItem = this;
        child->m_originalOrder = m_children.size();
        m_children.append(child);
    }

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

    static ParamItem *FromJson(const QJsonObject &json)
    {
        // 旧数据可能没有 valueType 字段，因此按编辑器类型推导一个默认类型。
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
