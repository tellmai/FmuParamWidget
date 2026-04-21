#ifndef FMUPARAMWIDGET_H
#define FMUPARAMWIDGET_H

#include <QIcon>
#include <QString>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QAction;
class QEvent;
class QLabel;
class QLineEdit;
class QRadioButton;
class QResizeEvent;
class QToolButton;
class QVBoxLayout;
QT_END_NAMESPACE

class InputParamEditorWidget;
class InternalParamEditorWidget;

class FmuSectionWidget : public QWidget
{
public:
    explicit FmuSectionWidget(const QString &title, QWidget *contentWidget, QWidget *parent = nullptr);

    /**
     * @brief SetExpanded
     * @param expanded 是否展开当前分组。
     */
    void SetExpanded(bool expanded);

    /**
     * @brief IsExpanded
     * @return 当前分组是否处于展开状态。
     */
    bool IsExpanded() const;

protected:
    /**
     * @brief eventFilter
     * @param watched 被监听的对象。
     * @param event 事件对象。
     * @return 事件是否已被当前控件处理。
     */
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    /**
     * @brief SetupUI
     * @param title 分组标题。
     */
    void SetupUI(const QString &title);

    /**
     * @brief ToggleExpanded
     * 根据当前展开状态取反，并通过 SetExpanded 更新箭头和内容区域显示状态。
     */
    void ToggleExpanded();

    QToolButton *m_arrowButton;
    QLabel *m_titleLabel;
    QWidget *m_headerWidget;
    QWidget *m_contentContainer;
    QWidget *m_contentWidget;
    bool m_expanded;
};

class FmuParamWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FmuParamWidget(QWidget *parent = nullptr);

    /**
     * @brief SetCoSimulationRadioVisible
     * @param visible 是否显示联合仿真选项。
     */
    void SetCoSimulationRadioVisible(bool visible);

    /**
     * @brief SetModelExchangeRadioVisible
     * @param visible 是否显示模型交换选项。
     */
    void SetModelExchangeRadioVisible(bool visible);

    /**
     * @brief SetSimulationModeRadiosVisible
     * @param visible 是否显示全部仿真模式选项。
     */
    void SetSimulationModeRadiosVisible(bool visible);

protected:
    /**
     * @brief resizeEvent
     * @param event 尺寸变化事件。
     */
    void resizeEvent(QResizeEvent *event) override;

private:
    /**
     * @brief SetupUI
     */
    void SetupUI();

    /**
     * @brief SetupConnections
     * @param browseAction 模型路径浏览按钮绑定的动作。
     */
    void SetupConnections(QAction *browseAction);

    /**
     * @brief CreateSections
     * @param sectionsLayout 用于放置分组控件的布局。
     */
    void CreateSections(QVBoxLayout *sectionsLayout);

    /**
     * @brief CreateBlankContent
     * @param height 空白占位区域高度。
     * @return 空白占位控件。
     */
    QWidget *CreateBlankContent(int height) const;

    /**
     * @brief CreateFolderIcon
     * @return 模型路径输入框右侧使用的文件夹图标。
     */
    QIcon CreateFolderIcon() const;

    /**
     * @brief UpdateResponsiveMargins
     */
    void UpdateResponsiveMargins();

    void OnBrowseModelPathTriggered();

    QLabel *m_modelPathLabel;
    QLabel *m_helpLabel;
    QLineEdit *m_modelPathEdit;
    QRadioButton *m_coSimulationRadio;
    QRadioButton *m_modelExchangeRadio;

    FmuSectionWidget *m_internalSection;
    FmuSectionWidget *m_simulationSection;
    FmuSectionWidget *m_inputSection;
    FmuSectionWidget *m_outputSection;
    InternalParamEditorWidget *m_internalParamEditor;
    InputParamEditorWidget *m_inputParamEditor;
};

#endif // FMUPARAMWIDGET_H
