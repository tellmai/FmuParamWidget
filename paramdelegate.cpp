#include "paramdelegate.h"

#include "ParamItem.h"
#include "parammodel.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFont>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QSpinBox>
#include <QStyle>
#include <QStyleOptionButton>

namespace {
// 表格单行高度，和交付要求保持一致。QTreeView 通过 sizeHint 使用这个高度。
constexpr int RowHeight = 24;
// 每深入一级参数树，名称列左侧增加的缩进距离。
constexpr int LevelIndent = 18;
// 展开/折叠箭头的绘制区域尺寸，保持小尺寸以适配 24px 行高。
constexpr int ArrowSize = 10;
// 普通单元格左右文字留白，避免文字贴近网格线。
constexpr int CellPadding = 5;
// 一级参数名称距离名称列左边框的基础留白。
constexpr int LeftPadding = 4;
// 名称列中预留给展开/折叠箭头的宽度。
constexpr int ArrowBoxWidth = 14;
// 二级及更深层级参数在缩进分隔线右侧继续保留的文字间距。
constexpr int ChildTextPadding = 8;
// 二级及更深层级参数的箭头相对缩进分隔线右侧的偏移量。
constexpr int ChildArrowPadding = 2;
// 表格网格线颜色，对应设计稿 border: 1px solid #cccccc。
const QColor GridColor(204, 204, 204);
// 表格正文文字颜色。
const QColor TextColor(86, 86, 86);
// 不可编辑单元格背景色，对应 #e1e1e1 叠加 0.4 透明度。
const QColor DisabledCellColor(225, 225, 225, 102);
// 可见性复选框选中状态的蓝色。
const QColor CheckBlue(0, 71, 157);
}

ParamDelegate::ParamDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{}

void ParamDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    // 先铺白底，再叠加模型给出的半透明背景，模拟 CSS opacity 效果。
    painter->fillRect(option.rect, Qt::white);

    QBrush backgroundBrush = index.data(Qt::BackgroundRole).value<QBrush>();
    if (backgroundBrush.style() == Qt::NoBrush) {
        backgroundBrush = QBrush(DisabledCellColor);
    }
    painter->fillRect(option.rect, backgroundBrush);

    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, QColor(218, 236, 255, 130));
    }

    const bool sectionHeader = IsSectionHeader(index);
    QFont font = opt.font;
    font.setFamily(QStringLiteral("Microsoft YaHei UI"));
    font.setPixelSize(14);
    font.setBold(false);
    painter->setFont(font);
    painter->setPen(TextColor);

    if (index.column() == ParamModel::NameColumn) {
        const int depth = DepthForIndex(index);
        const bool itemHasChildren = HasChildren(index);

        if (depth > 0) {
            // 子项左侧缩进区按设计稿保持白色，并合并成连续区域。
            const int indentWidth = depth * LevelIndent + 6;
            const QRect indentRect(option.rect.left(), option.rect.top(), indentWidth, option.rect.height());
            painter->fillRect(indentRect, Qt::white);
            painter->setPen(GridColor);
            // 多级层级时，每一层缩进都画一条竖线，避免深层节点只显示成一整块空白。
            for (int level = 1; level <= depth; ++level) {
                const int guideX = option.rect.left() + level * LevelIndent + 6;
                painter->drawLine(QPoint(guideX, option.rect.top()), QPoint(guideX, option.rect.bottom()));
            }
        }

        if (itemHasChildren) {
            // 展开箭头不使用系统图标，直接绘制成设计稿里的线框 chevron。
            const QRect indicatorRect = ArrowRect(option, index);
            const QPoint center = indicatorRect.center();
            painter->setPen(QPen(QColor(92, 92, 92), 1.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            if (index.data(Qt::UserRole + 4).toBool()) {
                painter->drawLine(QPoint(center.x() - 3, center.y() - 1), QPoint(center.x(), center.y() + 3));
                painter->drawLine(QPoint(center.x(), center.y() + 3), QPoint(center.x() + 3, center.y() - 1));
            } else {
                painter->drawLine(QPoint(center.x() - 1, center.y() - 3), QPoint(center.x() + 3, center.y()));
                painter->drawLine(QPoint(center.x() + 3, center.y()), QPoint(center.x() - 1, center.y() + 3));
            }
        }

        const int indentWidth = depth > 0 ? depth * LevelIndent + 6 : 0;
        const int contentLeft = option.rect.left()
            + (depth > 0 ? indentWidth + ChildTextPadding : LeftPadding);
        const int textLeft = contentLeft + (itemHasChildren ? ArrowBoxWidth + 6 : 0);
        const QRect textRect(textLeft, option.rect.top(), option.rect.right() - textLeft - CellPadding, option.rect.height());
        painter->setPen(sectionHeader ? QColor(74, 74, 74) : TextColor);
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter,
                          painter->fontMetrics().elidedText(index.data(Qt::DisplayRole).toString(), Qt::ElideRight, textRect.width()));
    } else if (index.column() == ParamModel::VisibleColumn) {
        // 可见性复选框完全自绘，保证选中/未选中状态和参考图一致。
        const bool checked = index.data(Qt::CheckStateRole).toInt() == Qt::Checked;
        const int checkSize = 14;
        const QRect checkRect(option.rect.left() + (option.rect.width() - checkSize) / 2,
                              option.rect.top() + (option.rect.height() - checkSize) / 2,
                              checkSize,
                              checkSize);

        painter->setPen(QPen(checked ? CheckBlue : QColor(198, 198, 198), 1));
        painter->setBrush(checked ? CheckBlue : QColor(255, 255, 255));
        painter->drawRoundedRect(checkRect.adjusted(1, 1, -1, -1), 2, 2);

        if (checked) {
            painter->setPen(QPen(Qt::white, 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter->drawLine(QPoint(checkRect.left() + 4, checkRect.center().y()),
                              QPoint(checkRect.left() + 6, checkRect.bottom() - 5));
            painter->drawLine(QPoint(checkRect.left() + 6, checkRect.bottom() - 5),
                              QPoint(checkRect.right() - 4, checkRect.top() + 4));
        }
    } else {
        const QRect textRect = option.rect.adjusted(CellPadding, 0, -CellPadding, 0);
        painter->setPen(TextColor);
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter,
                          painter->fontMetrics().elidedText(index.data(Qt::DisplayRole).toString(), Qt::ElideRight, textRect.width()));
    }

    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setPen(GridColor);

    if (index.column() == ParamModel::NameColumn && DepthForIndex(index) > 0 && !IsLastChildInParent(index)) {
        // 子项缩进块内部不画横线，形成截图里的“合并缩进”视觉。
        const int indentWidth = DepthForIndex(index) * LevelIndent + 6;
        painter->drawLine(QPoint(option.rect.left() + indentWidth, option.rect.bottom()),
                          option.rect.bottomRight());
    } else {
        painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
    }

    painter->drawLine(option.rect.topRight(), option.rect.bottomRight());

    if (index.column() == ParamModel::NameColumn) {
        painter->drawLine(option.rect.topLeft(), option.rect.bottomLeft());
    }

    painter->restore();
}

QSize ParamDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.setHeight(RowHeight);
    return size;
}

bool ParamDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                                const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (!index.isValid()) {
        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }

    if (event->type() == QEvent::MouseButtonRelease && index.column() == ParamModel::VisibleColumn) {
        // 点击可见性单元格任意位置都切换复选框，交互区域比图标本身更友好。
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton && option.rect.contains(mouseEvent->pos())) {
            const Qt::CheckState currentState = static_cast<Qt::CheckState>(index.data(Qt::CheckStateRole).toInt());
            const Qt::CheckState nextState = currentState == Qt::Checked ? Qt::Unchecked : Qt::Checked;
            model->setData(index, nextState, Qt::CheckStateRole);
            return true;
        }
    }

    if (event->type() == QEvent::MouseButtonPress && index.column() == ParamModel::NameColumn && HasChildren(index)) {
        // 只有点击箭头热区才展开/折叠，避免双击编辑或选择时误触。
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton && ArrowRect(option, index).contains(mouseEvent->pos())) {
            emit ExpandClicked(index);
            return true;
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

bool ParamDelegate::IsSectionHeader(const QModelIndex &index) const
{
    return index.data(Qt::UserRole).toBool();
}

QWidget *ParamDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    Q_UNUSED(option);

    if (index.column() != ParamModel::StartColumn) {
        return nullptr;
    }

    ParamItem *item = static_cast<ParamItem*>(index.internalPointer());
    if (!item || !item->m_editable) {
        return nullptr;
    }

    // 根据参数类型创建对应编辑器，当前只有“开始”列允许进入编辑态。
    switch (item->m_dataType) {
    case ParamDataType::CheckBox:
        return new QCheckBox(parent);
    case ParamDataType::ComboBox: {
        QComboBox *combo = new QComboBox(parent);
        combo->addItems({QStringLiteral("auto"), QStringLiteral("manual"), QStringLiteral("semi-auto")});
        return combo;
    }
    case ParamDataType::SpinBox: {
        QSpinBox *spin = new QSpinBox(parent);
        spin->setRange(-100000000, 100000000);
        return spin;
    }
    case ParamDataType::DoubleSpinBox: {
        QDoubleSpinBox *doubleSpin = new QDoubleSpinBox(parent);
        doubleSpin->setRange(-100000000.0, 100000000.0);
        doubleSpin->setDecimals(6);
        return doubleSpin;
    }
    case ParamDataType::LineEdit:
    default:
        return new QLineEdit(parent);
    }
}

void ParamDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    const QString value = index.model()->data(index, Qt::DisplayRole).toString();

    if (QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor)) {
        lineEdit->setText(value);
    } else if (QCheckBox *checkBox = qobject_cast<QCheckBox*>(editor)) {
        checkBox->setChecked(value.compare(QStringLiteral("true"), Qt::CaseInsensitive) == 0 || value == QStringLiteral("1"));
    } else if (QComboBox *combo = qobject_cast<QComboBox*>(editor)) {
        combo->setCurrentText(value);
    } else if (QSpinBox *spin = qobject_cast<QSpinBox*>(editor)) {
        spin->setValue(value.toInt());
    } else if (QDoubleSpinBox *doubleSpin = qobject_cast<QDoubleSpinBox*>(editor)) {
        doubleSpin->setValue(value.toDouble());
    }
}

void ParamDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                 const QModelIndex &index) const
{
    QString value;

    if (QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor)) {
        value = lineEdit->text();
    } else if (QCheckBox *checkBox = qobject_cast<QCheckBox*>(editor)) {
        value = checkBox->isChecked() ? QStringLiteral("true") : QStringLiteral("false");
    } else if (QComboBox *combo = qobject_cast<QComboBox*>(editor)) {
        value = combo->currentText();
    } else if (QSpinBox *spin = qobject_cast<QSpinBox*>(editor)) {
        value = QString::number(spin->value());
    } else if (QDoubleSpinBox *doubleSpin = qobject_cast<QDoubleSpinBox*>(editor)) {
        value = QString::number(doubleSpin->value(), 'g', 12);
    }

    model->setData(index, value, Qt::EditRole);
}

void ParamDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    Q_UNUSED(index);
    editor->setGeometry(option.rect.adjusted(3, 3, -3, -3));
}

int ParamDelegate::DepthForIndex(const QModelIndex &index) const
{
    // 通过 parent 链计算层级深度，用于名称列缩进。
    int depth = 0;
    QModelIndex parentIndex = index.parent();
    while (parentIndex.isValid()) {
        ++depth;
        parentIndex = parentIndex.parent();
    }
    return depth;
}

QRect ParamDelegate::ArrowRect(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const int depth = DepthForIndex(index);
    const int indentWidth = depth > 0 ? depth * LevelIndent + 6 : 0;
    const int contentLeft = option.rect.left()
        + (depth > 0 ? indentWidth + ChildArrowPadding : LeftPadding);
    const int left = contentLeft + (ArrowBoxWidth - ArrowSize) / 2;
    return QRect(left, option.rect.top() + (option.rect.height() - ArrowSize) / 2, ArrowSize, ArrowSize);
}

bool ParamDelegate::HasChildren(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return false;
    }

    const QModelIndex nameIndex = index.sibling(index.row(), ParamModel::NameColumn);
    return index.model()->rowCount(nameIndex) > 0;
}

bool ParamDelegate::IsLastChildInParent(const QModelIndex &index) const
{
    // 用于判断缩进区底部横线是否需要补齐。
    if (!index.isValid() || !index.parent().isValid()) {
        return false;
    }

    return index.row() == index.model()->rowCount(index.parent()) - 1;
}
