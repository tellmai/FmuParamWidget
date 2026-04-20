#include "internalparammodel.h"

#include <QBrush>
#include <QColor>
#include <QFont>

#include <algorithm>

InternalParamModel::InternalParamModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    SetupModelData();
}

InternalParamModel::~InternalParamModel()
{
    Clear();
}

void InternalParamModel::Clear()
{
    beginResetModel();
    qDeleteAll(m_rootItems);
    m_rootItems.clear();
    endResetModel();
}

QModelIndex InternalParamModel::index(int row, int column, const QModelIndex &parentIndex) const
{
    if (!hasIndex(row, column, parentIndex)) {
        return QModelIndex();
    }

    const ParamItem *parentItem = GetItem(parentIndex);
    const QVector<ParamItem*> &siblings = parentItem ? parentItem->m_children : m_rootItems;
    if (row < 0 || row >= siblings.size()) {
        return QModelIndex();
    }

    return createIndex(row, column, siblings.at(row));
}

QModelIndex InternalParamModel::parent(const QModelIndex &childIndex) const
{
    if (!childIndex.isValid()) {
        return QModelIndex();
    }

    const ParamItem *childItem = GetItem(childIndex);
    if (!childItem || !childItem->m_parentItem) {
        return QModelIndex();
    }

    ParamItem *parentItem = childItem->m_parentItem;
    ParamItem *grandParent = parentItem->m_parentItem;
    const int row = grandParent
        ? grandParent->m_children.indexOf(parentItem)
        : m_rootItems.indexOf(parentItem);

    if (row < 0) {
        return QModelIndex();
    }

    return createIndex(row, ParamColumn, parentItem);
}

int InternalParamModel::rowCount(const QModelIndex &parentIndex) const
{
    if (parentIndex.isValid() && parentIndex.column() != ParamColumn) {
        return 0;
    }

    const ParamItem *parentItem = GetItem(parentIndex);
    return parentItem ? parentItem->m_children.size() : m_rootItems.size();
}

int InternalParamModel::columnCount(const QModelIndex &parentIndex) const
{
    Q_UNUSED(parentIndex);
    return ColumnCount;
}

QVariant InternalParamModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const ParamItem *item = GetItem(index);
    if (!item) {
        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case ParamColumn:
            return item->m_param;
        case ValueColumn:
            return item->m_value;
        case UnitColumn:
            return item->m_unit;
        case DescriptionColumn:
            return item->m_description;
        default:
            return QVariant();
        }
    }

    if (role == Qt::BackgroundRole) {
        if (!item->m_isSectionHeader && index.column() == ValueColumn && item->m_editable) {
            return QBrush(Qt::white);
        }

        return QBrush(QColor(225, 225, 225, 102));
    }

    if (role == Qt::FontRole) {
        QFont font;
        font.setPointSize(10);
        return font;
    }

    if (role == Qt::TextAlignmentRole) {
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    }

    if (role == Qt::UserRole) {
        return item->m_isSectionHeader;
    }

    if (role == Qt::UserRole + 1) {
        return index.column() == ValueColumn && item->m_editable;
    }

    if (role == Qt::UserRole + 3) {
        return static_cast<int>(item->m_valueType);
    }

    if (role == Qt::UserRole + 4) {
        return item->m_isExpanded;
    }

    return QVariant();
}

QVariant InternalParamModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        switch (section) {
        case ParamColumn:
            return QString::fromUtf8("参数");
        case ValueColumn:
            return QString::fromUtf8("值");
        case UnitColumn:
            return QString::fromUtf8("单位");
        case DescriptionColumn:
            return QString::fromUtf8("描述");
        default:
            return QVariant();
        }
    }

    if (role == Qt::TextAlignmentRole) {
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    }

    if (role == Qt::BackgroundRole) {
        return QBrush(QColor(250, 250, 250));
    }

    return QVariant();
}

bool InternalParamModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }

    ParamItem *item = GetItem(index);
    if (!item || role != Qt::EditRole || index.column() != ValueColumn || !item->m_editable) {
        return false;
    }

    item->m_value = value.toString();
    emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole, Qt::BackgroundRole});
    return true;
}

Qt::ItemFlags InternalParamModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags itemFlags = QAbstractItemModel::flags(index);
    const ParamItem *item = GetItem(index);
    if (index.column() == ValueColumn && item && item->m_editable) {
        itemFlags |= Qt::ItemIsEditable;
    }

    return itemFlags;
}

void InternalParamModel::ToggleExpand(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    ParamItem *item = GetItem(index);
    if (!item || item->m_children.isEmpty()) {
        return;
    }

    item->m_isExpanded = !item->m_isExpanded;

    const QModelIndex parentIndex = index.parent();
    const QModelIndex first = this->index(index.row(), ParamColumn, parentIndex);
    const QModelIndex last = this->index(index.row(), DescriptionColumn, parentIndex);
    emit dataChanged(first, last, {Qt::UserRole + 4});
}

void InternalParamModel::SortByColumn(int column, ParamSortMode mode)
{
    if (column != ParamColumn && column != ValueColumn) {
        return;
    }

    beginResetModel();
    SortItems(m_rootItems, column, mode);
    endResetModel();
}

void InternalParamModel::SetupModelData()
{
    beginResetModel();
    qDeleteAll(m_rootItems);
    m_rootItems.clear();

    AppendRoot(new ParamItem(QStringLiteral("dfhfss"), QStringLiteral("3"), QString(), QStringLiteral("mintj"),
                             false, false, -1, true, ParamDataType::SpinBox, false, FmuValueType::Int32));
    AppendRoot(new ParamItem(QStringLiteral("dfhfss"), QStringLiteral("3"), QStringLiteral("g/s"), QStringLiteral("mintj"),
                             false, false, -1, true, ParamDataType::SpinBox, false, FmuValueType::Int32));
    AppendRoot(new ParamItem(QStringLiteral("dfhfss"), QStringLiteral("3"), QString(), QStringLiteral("mintj"),
                             false, false, -1, true, ParamDataType::SpinBox, false, FmuValueType::Int16));

    ParamItem *pumpRate = new ParamItem(QStringLiteral("PumpRateKIX"), QString(), QString(), QString(),
                                        true, true, -1, false, ParamDataType::LineEdit, false, FmuValueType::String);
    pumpRate->AppendChild(new ParamItem(QStringLiteral("dfhfss"), QStringLiteral("3"), QString(), QStringLiteral("mintj"),
                                        false, false, -1, true, ParamDataType::SpinBox, false, FmuValueType::Int8));
    pumpRate->AppendChild(new ParamItem(QStringLiteral("dfhfss"), QStringLiteral("3"), QString(), QStringLiteral("mintj"),
                                        false, false, -1, true, ParamDataType::SpinBox, false, FmuValueType::Int16));
    pumpRate->AppendChild(new ParamItem(QStringLiteral("dfhfss"), QStringLiteral("3"), QString(), QStringLiteral("mintj"),
                                        false, false, -1, true, ParamDataType::SpinBox, false, FmuValueType::Int32));
    pumpRate->AppendChild(new ParamItem(QStringLiteral("dfhfss"), QStringLiteral("3"), QString(), QStringLiteral("mintj"),
                                        false, false, -1, true, ParamDataType::DoubleSpinBox, false, FmuValueType::Double));
    AppendRoot(pumpRate);

    ParamItem *pumpRate02 = new ParamItem(QStringLiteral("PumpRateKIX02"), QString(), QString(), QString(),
                                          true, true, -1, false, ParamDataType::LineEdit, false, FmuValueType::String);
    pumpRate02->AppendChild(new ParamItem(QStringLiteral("enabled"), QStringLiteral("false"), QString(), QStringLiteral("bool value"),
                                          false, false, -1, true, ParamDataType::CheckBox, false, FmuValueType::Boolean));
    pumpRate02->AppendChild(new ParamItem(QStringLiteral("rate_min"), QStringLiteral("1.5"), QStringLiteral("g/s"), QStringLiteral("double value"),
                                          false, false, -1, true, ParamDataType::DoubleSpinBox, false, FmuValueType::Double));
    pumpRate02->AppendChild(new ParamItem(QStringLiteral("rate_label"), QStringLiteral("alpha"), QString(), QStringLiteral("string value"),
                                          false, false, -1, true, ParamDataType::LineEdit, false, FmuValueType::String));
    AppendRoot(pumpRate02);

    ParamItem *nested = new ParamItem(QStringLiteral("FuelSystem"), QString(), QString(), QString(),
                                      true, true, -1, false, ParamDataType::LineEdit, false, FmuValueType::String);
    ParamItem *subGroup = new ParamItem(QStringLiteral("Limiter"), QString(), QString(), QString(),
                                        true, false, -1, false, ParamDataType::LineEdit, false, FmuValueType::String);
    subGroup->AppendChild(new ParamItem(QStringLiteral("active"), QStringLiteral("true"), QString(), QStringLiteral("switch"),
                                        false, false, -1, true, ParamDataType::CheckBox, false, FmuValueType::Boolean));
    subGroup->AppendChild(new ParamItem(QStringLiteral("threshold"), QStringLiteral("12"), QStringLiteral("bar"), QStringLiteral("pressure"),
                                        false, false, -1, true, ParamDataType::SpinBox, false, FmuValueType::Int32));

    ParamItem *safetyWindow = new ParamItem(QStringLiteral("SafetyWindow"), QString(), QString(), QString(),
                                            true, false, -1, false, ParamDataType::LineEdit, false, FmuValueType::String);
    safetyWindow->AppendChild(new ParamItem(QStringLiteral("min_enable"), QStringLiteral("false"), QString(), QStringLiteral("boolean order test"),
                                            false, false, -1, true, ParamDataType::CheckBox, false, FmuValueType::Boolean));
    safetyWindow->AppendChild(new ParamItem(QStringLiteral("min_value"), QStringLiteral("2"), QStringLiteral("bar"), QStringLiteral("int order test"),
                                            false, false, -1, true, ParamDataType::SpinBox, false, FmuValueType::Int16));
    safetyWindow->AppendChild(new ParamItem(QStringLiteral("max_value"), QStringLiteral("18.5"), QStringLiteral("bar"), QStringLiteral("double order test"),
                                            false, false, -1, true, ParamDataType::DoubleSpinBox, false, FmuValueType::Double));
    safetyWindow->AppendChild(new ParamItem(QStringLiteral("mode_name"), QStringLiteral("beta"), QString(), QStringLiteral("string order test"),
                                            false, false, -1, true, ParamDataType::LineEdit, false, FmuValueType::String));
    subGroup->AppendChild(safetyWindow);
    nested->AppendChild(subGroup);
    AppendRoot(nested);

    AssignItemMetadata();
    endResetModel();
}

ParamItem *InternalParamModel::GetItem(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return nullptr;
    }

    return static_cast<ParamItem*>(index.internalPointer());
}

void InternalParamModel::AppendRoot(ParamItem *item)
{
    if (!item) {
        return;
    }

    item->m_parentItem = nullptr;
    item->m_originalOrder = m_rootItems.size();
    m_rootItems.append(item);
}

void InternalParamModel::AssignItemMetadata()
{
    for (int i = 0; i < m_rootItems.size(); ++i) {
        ParamItem *item = m_rootItems.at(i);
        item->m_parentItem = nullptr;
        item->m_originalOrder = i;
        AssignItemMetadata(item);
    }
}

void InternalParamModel::AssignItemMetadata(ParamItem *parent)
{
    if (!parent) {
        return;
    }

    for (int i = 0; i < parent->m_children.size(); ++i) {
        ParamItem *child = parent->m_children.at(i);
        child->m_parentItem = parent;
        child->m_originalOrder = i;
        AssignItemMetadata(child);
    }
}

void InternalParamModel::SortItems(QVector<ParamItem*> &items, int column, ParamSortMode mode)
{
    std::stable_sort(items.begin(), items.end(), [this, column, mode](const ParamItem *left, const ParamItem *right) {
        return CompareItems(left, right, column, mode) < 0;
    });

    for (ParamItem *item : items) {
        SortItems(item->m_children, column, mode);
    }
}

int InternalParamModel::CompareItems(const ParamItem *left, const ParamItem *right, int column, ParamSortMode mode) const
{
    if (mode == ParamSortMode::Default) {
        return left->m_originalOrder - right->m_originalOrder;
    }

    int comparison = 0;
    if (column == ParamColumn) {
        comparison = QString::localeAwareCompare(left->m_param.toLower(), right->m_param.toLower());
    } else if (column == ValueColumn) {
        comparison = CompareValues(left, right);
    }

    if (comparison != 0 && mode == ParamSortMode::Descending) {
        comparison = -comparison;
    }

    if (comparison == 0) {
        comparison = left->m_originalOrder - right->m_originalOrder;
    }

    return comparison;
}

int InternalParamModel::CompareValues(const ParamItem *left, const ParamItem *right) const
{
    const int leftRank = ValueTypeRank(left->m_valueType);
    const int rightRank = ValueTypeRank(right->m_valueType);
    if (leftRank != rightRank) {
        return leftRank - rightRank;
    }

    switch (left->m_valueType) {
    case FmuValueType::Boolean: {
        const bool leftValue = BoolValue(left->m_value);
        const bool rightValue = BoolValue(right->m_value);
        if (leftValue == rightValue) {
            return 0;
        }
        return leftValue ? 1 : -1;
    }
    case FmuValueType::Int8:
    case FmuValueType::Int16:
    case FmuValueType::Int32:
    case FmuValueType::Int64:
    case FmuValueType::Float:
    case FmuValueType::Double: {
        bool leftOk = false;
        bool rightOk = false;
        const double leftValue = NumericValue(left->m_value, &leftOk);
        const double rightValue = NumericValue(right->m_value, &rightOk);
        if (leftOk && rightOk) {
            if (leftValue < rightValue) {
                return -1;
            }
            if (leftValue > rightValue) {
                return 1;
            }
            return 0;
        }
        if (leftOk != rightOk) {
            return leftOk ? -1 : 1;
        }
        return QString::localeAwareCompare(left->m_value.toLower(), right->m_value.toLower());
    }
    case FmuValueType::Char:
    case FmuValueType::String:
        return QString::localeAwareCompare(left->m_value.toLower(), right->m_value.toLower());
    case FmuValueType::ClassifiedArray:
    case FmuValueType::Enumeration:
    case FmuValueType::Unknown:
    default:
        return 0;
    }
}

int InternalParamModel::ValueTypeRank(FmuValueType type)
{
    switch (type) {
    case FmuValueType::Boolean:
        return 0;
    case FmuValueType::Int8:
        return 1;
    case FmuValueType::Int16:
        return 2;
    case FmuValueType::Int32:
        return 3;
    case FmuValueType::Int64:
        return 4;
    case FmuValueType::Float:
        return 5;
    case FmuValueType::Double:
        return 6;
    case FmuValueType::Char:
        return 7;
    case FmuValueType::String:
        return 8;
    case FmuValueType::ClassifiedArray:
        return 9;
    case FmuValueType::Enumeration:
        return 10;
    case FmuValueType::Unknown:
    default:
        return 11;
    }
}

bool InternalParamModel::BoolValue(const QString &value)
{
    const QString normalized = value.trimmed().toLower();
    return !(normalized.isEmpty()
             || normalized == QStringLiteral("0")
             || normalized == QStringLiteral("false")
             || normalized == QStringLiteral("no")
             || normalized == QStringLiteral("off"));
}

double InternalParamModel::NumericValue(const QString &value, bool *ok)
{
    return value.trimmed().toDouble(ok);
}
