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

    void SetExpanded(bool expanded);
    bool IsExpanded() const;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void SetupUI(const QString &title);
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

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void SetupUI();
    void SetupConnections(QAction *browseAction);
    void CreateSections(QVBoxLayout *sectionsLayout);
    QWidget *CreateBlankContent(int height) const;
    QIcon CreateFolderIcon() const;
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
