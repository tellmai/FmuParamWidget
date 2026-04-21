#ifndef PARAMSETWIDGET_H
#define PARAMSETWIDGET_H

#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QWidget>

class FmuParamWidget;
class QLabel;
class ParamCategoryWidget;
class QPixmap;
class QTextEdit;
class QVBoxLayout;

struct ParamCategoryContentData
{
    QString type;
    QStringList titles;
    QVariantMap properties;

    ParamCategoryContentData(const QString &typeValue = QString(),
                             const QStringList &titleValues = QStringList(),
                             const QVariantMap &propertyValues = QVariantMap());
};

class ParamSetWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ParamSetWidget(QWidget *parent = nullptr);
    explicit ParamSetWidget(const ParamCategoryContentData &categoryData, QWidget *parent = nullptr);

    /**
     * @brief SetModuleImage
     * @param pixmap 模块图片。
     */
    void SetModuleImage(const QPixmap &pixmap);

    /**
     * @brief SetModuleImage
     * @param imagePath 模块图片文件路径。
     */
    void SetModuleImage(const QString &imagePath);

    /**
     * @brief SetModuleName
     * @param name 模块名称。
     */
    void SetModuleName(const QString &name);

    /**
     * @brief SetModuleDescription
     * @param description 模块说明内容。
     */
    void SetModuleDescription(const QString &description);

    /**
     * @brief SetCategoryContentData
     * @param categoryData 分类标题和内容类型配置。
     */
    void SetCategoryContentData(const ParamCategoryContentData &categoryData);

signals:
    void HelpRequested();
    void Accepted();
    void Applied();
    void Canceled();

private slots:
    void OnHelpButtonClicked();
    void OnOkButtonClicked();
    void OnApplyButtonClicked();
    void OnCancelButtonClicked();

private:
    /**
     * @brief SetupUI
     * @param categoryData 初始化时使用的分类内容配置。
     */
    void SetupUI(const ParamCategoryContentData &categoryData);

    /**
     * @brief CreateModuleSummaryWidget
     * @return 模块图片、名称和说明区域。
     */
    QWidget *CreateModuleSummaryWidget();

    /**
     * @brief DefaultCategoryContentData
     * @return 未传入分类配置时使用的默认配置。
     */
    ParamCategoryContentData DefaultCategoryContentData() const;

    /**
     * @brief NormalizeCategoryContentData
     * @param categoryData 外部传入的分类内容配置。
     * @return 补齐默认标题后的分类内容配置。
     */
    ParamCategoryContentData NormalizeCategoryContentData(const ParamCategoryContentData &categoryData) const;

    /**
     * @brief CreateCategoryWidget
     * @param title 分类标题。
     * @param contentWidget 分类展开后显示的内容控件。
     * @param parent 父窗口对象。
     * @return 分类折叠面板控件。
     */
    ParamCategoryWidget *CreateCategoryWidget(const QString &title, QWidget *contentWidget, QWidget *parent);

    /**
     * @brief CreateTypedContentWidget
     * @param categoryData 分类内容配置。
     * @param parent 父窗口对象。
     * @return 根据 type 创建出的内容控件；未知 type 返回空控件指针。
     */
    QWidget *CreateTypedContentWidget(const ParamCategoryContentData &categoryData, QWidget *parent);

    /**
     * @brief CreateButtonBar
     * @return 底部帮助、确定、应用和取消按钮区域。
     */
    QWidget *CreateButtonBar();

    /**
     * @brief BuildCategoryWidgets
     * @param categoryData 分类标题和内容类型配置。
     */
    void BuildCategoryWidgets(const ParamCategoryContentData &categoryData);

    /**
     * @brief ClearCategoryWidgets
     */
    void ClearCategoryWidgets();

    QLabel *m_moduleImageLabel;
    QLabel *m_moduleNameLabel;
    QTextEdit *m_moduleDescriptionEdit;
    QWidget *m_contentWidget;
    QVBoxLayout *m_contentLayout;
    QVector<ParamCategoryWidget*> m_categoryWidgets;
    FmuParamWidget *m_fmuParamWidget;
};

#endif // PARAMSETWIDGET_H
