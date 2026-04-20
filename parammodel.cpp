#include "parammodel.h"

#include <QBrush>
#include <QColor>
#include <QFont>
#include <QJsonParseError>

#include <algorithm>

ParamModel::ParamModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    SetupModelData();
}

ParamModel::~ParamModel()
{
    Clear();
}

void ParamModel::Clear()
{
    beginResetModel();
    qDeleteAll(m_rootItems);
    m_rootItems.clear();
    endResetModel();
}

QModelIndex ParamModel::index(int row, int column, const QModelIndex &parentIndex) const
{
    if (!hasIndex(row, column, parentIndex)) {
        return QModelIndex();
    }

    // parentIndex 无效时取根节点，否则取对应父节点的 children。
    const ParamItem *parentItem = GetItem(parentIndex);
    const QVector<ParamItem*> &siblings = parentItem ? parentItem->m_children : m_rootItems;

    if (row < 0 || row >= siblings.size()) {
        return QModelIndex();
    }

    return createIndex(row, column, siblings.at(row));
}

QModelIndex ParamModel::parent(const QModelIndex &childIndex) const
{
    if (!childIndex.isValid()) {
        return QModelIndex();
    }

    const ParamItem *childItem = GetItem(childIndex);
    if (!childItem || !childItem->m_parentItem) {
        return QModelIndex();
    }

    // parentItem 指向模型里的真实节点，row 必须在爷爷节点的 children 中查找。
    ParamItem *parentItem = childItem->m_parentItem;
    ParamItem *grandParent = parentItem->m_parentItem;
    const int row = grandParent
        ? grandParent->m_children.indexOf(parentItem)
        : m_rootItems.indexOf(parentItem);

    if (row < 0) {
        return QModelIndex();
    }

    return createIndex(row, NameColumn, parentItem);
}

int ParamModel::rowCount(const QModelIndex &parentIndex) const
{
    if (parentIndex.isValid() && parentIndex.column() != NameColumn) {
        return 0;
    }

    const ParamItem *parentItem = GetItem(parentIndex);
    return parentItem ? parentItem->m_children.size() : m_rootItems.size();
}

int ParamModel::columnCount(const QModelIndex &parentIndex) const
{
    Q_UNUSED(parentIndex);
    return ColumnCount;
}

QVariant ParamModel::data(const QModelIndex &index, int role) const
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
        case NameColumn:
            return item->m_param;
        case VisibleColumn:
            return QVariant();
        case StartColumn:
            return item->m_value;
        case UnitColumn:
            return item->m_unit;
        default:
            return QVariant();
        }
    }

    if (role == Qt::CheckStateRole && index.column() == VisibleColumn) {
        return item->m_visible ? Qt::Checked : Qt::Unchecked;
    }

    if (role == Qt::BackgroundRole) {
        // 只有可编辑的开始值是白底，其它单元格按设计稿叠加半透明灰。
        if (!item->m_isSectionHeader && index.column() == StartColumn && item->m_editable) {
            return QBrush(Qt::white);
        }

        return QBrush(QColor(225, 225, 225, 102));
    }

    if (role == Qt::FontRole) {
        QFont font;
        font.setPointSize(11);
        return font;
    }

    if (role == Qt::TextAlignmentRole) {
        if (index.column() == VisibleColumn) {
            return QVariant(Qt::AlignCenter);
        }
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    }

    if (role == Qt::UserRole) {
        return item->m_isSectionHeader;
    }

    if (role == Qt::UserRole + 1) {
        return index.column() == StartColumn && item->m_editable;
    }

    if (role == Qt::UserRole + 2) {
        return item->m_visible;
    }

    if (role == Qt::UserRole + 3) {
        return static_cast<int>(item->m_valueType);
    }

    if (role == Qt::UserRole + 4) {
        return item->m_isExpanded;
    }

    return QVariant();
}

QVariant ParamModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        switch (section) {
        case NameColumn:
            return QStringLiteral("名称");
        case VisibleColumn:
            return QStringLiteral("可见性");
        case StartColumn:
            return QStringLiteral("开始");
        case UnitColumn:
            return QStringLiteral("单位");
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

bool ParamModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }

    ParamItem *item = GetItem(index);
    if (!item) {
        return false;
    }

    if (role == Qt::CheckStateRole && index.column() == VisibleColumn) {
        // 可见性列通过 CheckStateRole 驱动，m_delegate 只负责绘制和触发切换。
        item->m_visible = value.toInt() == Qt::Checked;
        emit dataChanged(index, index, {Qt::CheckStateRole, Qt::UserRole + 2});
        return true;
    }

    if (role != Qt::EditRole || index.column() != StartColumn || !item->m_editable) {
        return false;
    }

    item->m_value = value.toString();
    emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole, Qt::BackgroundRole});
    return true;
}

Qt::ItemFlags ParamModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags itemFlags = QAbstractItemModel::flags(index);

    if (index.column() == VisibleColumn) {
        itemFlags |= Qt::ItemIsUserCheckable;
    }

    const ParamItem *item = GetItem(index);
    if (index.column() == StartColumn && item && item->m_editable) {
        itemFlags |= Qt::ItemIsEditable;
    }

    return itemFlags;
}

void ParamModel::ToggleExpand(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    ParamItem *item = GetItem(index);
    if (!item || item->m_children.isEmpty()) {
        return;
    }

    // 展开状态存回数据项，排序或刷新后可以重新恢复视图。
    item->m_isExpanded = !item->m_isExpanded;

    const QModelIndex parentIndex = index.parent();
    const QModelIndex first = this->index(index.row(), NameColumn, parentIndex);
    const QModelIndex last = this->index(index.row(), UnitColumn, parentIndex);
    emit dataChanged(first, last, {Qt::UserRole + 4});
}

void ParamModel::SortByColumn(int column, ParamSortMode mode)
{
    if (column != NameColumn) {
        return;
    }

    // 这里使用 reset，确保树中所有层级的顺序变化一次性刷新到视图。
    beginResetModel();
    SortItems(m_rootItems, column, mode);
    endResetModel();
}

void ParamModel::SortItems(QVector<ParamItem*> &items, int column, ParamSortMode mode)
{
    // 每个 children 容器单独排序，所以一级、二级参数不会被混排。
    std::stable_sort(items.begin(), items.end(), [this, column, mode](const ParamItem *left, const ParamItem *right) {
        return CompareItems(left, right, column, mode) < 0;
    });

    for (ParamItem *item : items) {
        SortItems(item->m_children, column, mode);
    }
}

int ParamModel::CompareItems(const ParamItem *left, const ParamItem *right, int column, ParamSortMode mode) const
{
    if (mode == ParamSortMode::Default) {
        // 默认排序恢复同级节点加载时的原始顺序。
        return left->m_originalOrder - right->m_originalOrder;
    }

    int comparison = 0;

    comparison = QString::localeAwareCompare(left->m_param.toLower(), right->m_param.toLower());

    if (comparison != 0 && mode == ParamSortMode::Descending) {
        comparison = -comparison;
    }

    if (comparison == 0) {
        comparison = left->m_originalOrder - right->m_originalOrder;
    }

    return comparison;
}

int ParamModel::CompareValues(const ParamItem *left, const ParamItem *right) const
{
    // 起始值排序逻辑目前未对外开放，保留在模型内供后续需求启用。
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

int ParamModel::ValueTypeRank(FmuValueType type)
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

bool ParamModel::BoolValue(const QString &value)
{
    const QString normalized = value.trimmed().toLower();
    return !(normalized.isEmpty()
             || normalized == QStringLiteral("0")
             || normalized == QStringLiteral("false")
             || normalized == QStringLiteral("no")
             || normalized == QStringLiteral("off"));
}

double ParamModel::NumericValue(const QString &value, bool *ok)
{
    return value.trimmed().toDouble(ok);
}

QJsonObject ParamModel::ToJson() const
{
    QJsonObject rootJson;
    QJsonArray itemsArray;

    for (const ParamItem *item : m_rootItems) {
        itemsArray.append(item->ToJson());
    }

    rootJson["items"] = itemsArray;
    rootJson["version"] = QStringLiteral("1.0");
    rootJson["columnCount"] = columnCount();

    return rootJson;
}

void ParamModel::FromJson(const QJsonObject &json)
{
    beginResetModel();
    qDeleteAll(m_rootItems);
    m_rootItems.clear();

    if (json.contains("items") && json["items"].isArray()) {
        const QJsonArray itemsArray = json["items"].toArray();
        for (const QJsonValue &itemValue : itemsArray) {
            if (itemValue.isObject()) {
                m_rootItems.append(ParamItem::FromJson(itemValue.toObject()));
            }
        }
    }

    // JSON 反序列化后重新补父指针和原始顺序，保证树模型可导航。
    AssignItemMetadata();
    endResetModel();
}

QString ParamModel::ToJsonString() const
{
    const QJsonDocument doc(ToJson());
    return QString::fromUtf8(doc.toJson(QJsonDocument::Indented));
}

bool ParamModel::FromJsonString(const QString &jsonString)
{
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        return false;
    }

    FromJson(doc.object());
    return true;
}

void ParamModel::SetupModelData()
{
    beginResetModel();
    qDeleteAll(m_rootItems);
    m_rootItems.clear();

    auto appendRoot = [this](ParamItem *item) {
        // 根节点没有父节点，但仍要记录原始顺序以支持默认排序。
        item->m_parentItem = nullptr;
        item->m_originalOrder = m_rootItems.size();
        m_rootItems.append(item);
    };

    ParamItem *sensc = new ParamItem(QStringLiteral("Sensc"), QString(), QString(), QString(),
                                     true, true, -1, false, ParamDataType::LineEdit, false, FmuValueType::String);
    sensc->AppendChild(new ParamItem(QStringLiteral("fuel_rdsfghh"), QStringLiteral("0"), QString(), QString(),
                                     false, false, -1, true, ParamDataType::SpinBox, false, FmuValueType::Int32));
    sensc->AppendChild(new ParamItem(QStringLiteral("ccv"), QStringLiteral("0"), QStringLiteral("g/s"), QString(),
                                     false, false, -1, true, ParamDataType::DoubleSpinBox, true, FmuValueType::Double));
    sensc->AppendChild(new ParamItem(QStringLiteral("gh"), QStringLiteral("0"), QStringLiteral("g/s"), QString(),
                                     false, false, -1, true, ParamDataType::DoubleSpinBox, true, FmuValueType::Double));
    sensc->AppendChild(new ParamItem(QStringLiteral("hhdgzvcxvcxvcxvSSSSdaS"), QStringLiteral("0"), QStringLiteral("g/s"), QString(),
                                     false, false, -1, true, ParamDataType::DoubleSpinBox, true, FmuValueType::Double));
    sensc->AppendChild(new ParamItem(QStringLiteral("vxv"), QString(), QString(), QString(),
                                     false, false, -1, false, ParamDataType::LineEdit, false, FmuValueType::String));

    // 多级测试数据：Sensc -> combustion -> injector -> injector_A -> 具体参数。
    ParamItem *combustion = new ParamItem(QStringLiteral("combustion"), QString(), QString(), QString(),
                                          true, false, -1, false, ParamDataType::LineEdit, false, FmuValueType::String);
    ParamItem *injector = new ParamItem(QStringLiteral("injector"), QString(), QString(), QString(),
                                        true, false, -1, false, ParamDataType::LineEdit, false, FmuValueType::String);
    ParamItem *injectorA = new ParamItem(QStringLiteral("injector_A"), QString(), QString(), QString(),
                                         true, false, -1, false, ParamDataType::LineEdit, false, FmuValueType::String);
    injectorA->AppendChild(new ParamItem(QStringLiteral("pulse_width"), QStringLiteral("2.5"), QStringLiteral("ms"), QString(),
                                         false, false, -1, true, ParamDataType::DoubleSpinBox, true, FmuValueType::Double));
    injectorA->AppendChild(new ParamItem(QStringLiteral("spray_angle"), QStringLiteral("18"), QStringLiteral("deg"), QString(),
                                         false, false, -1, true, ParamDataType::SpinBox, false, FmuValueType::Int32));

    ParamItem *injectorB = new ParamItem(QStringLiteral("injector_B"), QString(), QString(), QString(),
                                         true, false, -1, false, ParamDataType::LineEdit, false, FmuValueType::String);
    injectorB->AppendChild(new ParamItem(QStringLiteral("pulse_width"), QStringLiteral("2.1"), QStringLiteral("ms"), QString(),
                                         false, false, -1, true, ParamDataType::DoubleSpinBox, false, FmuValueType::Double));
    injectorB->AppendChild(new ParamItem(QStringLiteral("enabled"), QStringLiteral("true"), QString(), QString(),
                                         false, false, -1, true, ParamDataType::CheckBox, true, FmuValueType::Boolean));

    injector->AppendChild(injectorA);
    injector->AppendChild(injectorB);
    injector->AppendChild(new ParamItem(QStringLiteral("rail_pressure"), QStringLiteral("145"), QStringLiteral("bar"), QString(),
                                        false, false, -1, true, ParamDataType::DoubleSpinBox, true, FmuValueType::Double));

    ParamItem *ignition = new ParamItem(QStringLiteral("ignition"), QString(), QString(), QString(),
                                        true, false, -1, false, ParamDataType::LineEdit, false, FmuValueType::String);
    ignition->AppendChild(new ParamItem(QStringLiteral("spark_advance"), QStringLiteral("12"), QStringLiteral("deg"), QString(),
                                        false, false, -1, true, ParamDataType::SpinBox, true, FmuValueType::Int32));
    ignition->AppendChild(new ParamItem(QStringLiteral("coil_dwell"), QStringLiteral("3.4"), QStringLiteral("ms"), QString(),
                                        false, false, -1, true, ParamDataType::DoubleSpinBox, false, FmuValueType::Double));

    combustion->AppendChild(injector);
    combustion->AppendChild(ignition);
    sensc->AppendChild(combustion);
    appendRoot(sensc);

    ParamItem *secondary = new ParamItem(QStringLiteral("Senscvcxvcvcxdasdad"), QString(), QString(), QString(),
                                         true, true, -1, false, ParamDataType::LineEdit, false, FmuValueType::String);
    secondary->AppendChild(new ParamItem(QStringLiteral("enable"), QStringLiteral("false"), QString(), QString(),
                                         false, false, -1, true, ParamDataType::CheckBox, true, FmuValueType::Boolean));
    secondary->AppendChild(new ParamItem(QStringLiteral("label"), QStringLiteral("alpha"), QString(), QString(),
                                         false, false, -1, true, ParamDataType::LineEdit, false, FmuValueType::String));

    ParamItem *secondaryGroup = new ParamItem(QStringLiteral("diagnostics"), QString(), QString(), QString(),
                                              true, false, -1, false, ParamDataType::LineEdit, false, FmuValueType::String);
    ParamItem *secondaryBus = new ParamItem(QStringLiteral("bus"), QString(), QString(), QString(),
                                            true, false, -1, false, ParamDataType::LineEdit, false, FmuValueType::String);
    secondaryBus->AppendChild(new ParamItem(QStringLiteral("timeout"), QStringLiteral("30"), QStringLiteral("ms"), QString(),
                                            false, false, -1, true, ParamDataType::SpinBox, false, FmuValueType::Int32));
    secondaryBus->AppendChild(new ParamItem(QStringLiteral("retry_count"), QStringLiteral("3"), QString(), QString(),
                                            false, false, -1, true, ParamDataType::SpinBox, true, FmuValueType::Int32));
    secondaryGroup->AppendChild(secondaryBus);
    secondaryGroup->AppendChild(new ParamItem(QStringLiteral("log_level"), QStringLiteral("info"), QString(), QString(),
                                              false, false, -1, true, ParamDataType::LineEdit, false, FmuValueType::String));
    secondary->AppendChild(secondaryGroup);
    appendRoot(secondary);

    ParamItem *timing = new ParamItem(QStringLiteral("Timing"), QString(), QString(), QString(),
                                      true, true, -1, false, ParamDataType::LineEdit, false, FmuValueType::String);
    ParamItem *clock = new ParamItem(QStringLiteral("clock"), QString(), QString(), QString(),
                                     true, false, -1, false, ParamDataType::LineEdit, false, FmuValueType::String);
    ParamItem *runtime = new ParamItem(QStringLiteral("runtime"), QString(), QString(), QString(),
                                       true, false, -1, false, ParamDataType::LineEdit, false, FmuValueType::String);
    runtime->AppendChild(new ParamItem(QStringLiteral("time"), QStringLiteral("0"), QStringLiteral("g/s"), QString(),
                                       false, false, -1, true, ParamDataType::DoubleSpinBox, false, FmuValueType::Double));
    runtime->AppendChild(new ParamItem(QStringLiteral("step_size"), QStringLiteral("0.01"), QStringLiteral("s"), QString(),
                                       false, false, -1, true, ParamDataType::DoubleSpinBox, true, FmuValueType::Double));
    clock->AppendChild(runtime);
    timing->AppendChild(clock);
    appendRoot(timing);

    AssignItemMetadata();
    endResetModel();
}

ParamItem *ParamModel::GetItem(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return nullptr;
    }

    return static_cast<ParamItem*>(index.internalPointer());
}

void ParamModel::AssignItemMetadata()
{
    for (int i = 0; i < m_rootItems.size(); ++i) {
        ParamItem *item = m_rootItems.at(i);
        item->m_parentItem = nullptr;
        item->m_originalOrder = i;
        AssignItemMetadata(item);
    }
}

void ParamModel::AssignItemMetadata(ParamItem *parent)
{
    if (!parent) {
        return;
    }

    for (int i = 0; i < parent->m_children.size(); ++i) {
        ParamItem *child = parent->m_children.at(i);
        // 排序后也要保留父子关系，QAbstractItemModel::parent() 依赖这里。
        child->m_parentItem = parent;
        child->m_originalOrder = i;
        AssignItemMetadata(child);
    }
}
