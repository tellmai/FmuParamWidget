#ifndef PARAMSETWIDGET_H
#define PARAMSETWIDGET_H

#include <QWidget>

class FmuParamWidget;
class QLabel;
class QPixmap;
class QString;
class QTextEdit;

class ParamSetWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ParamSetWidget(QWidget *parent = nullptr);

    void SetModuleImage(const QPixmap &pixmap);
    void SetModuleImage(const QString &imagePath);
    void SetModuleName(const QString &name);
    void SetModuleDescription(const QString &description);

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
    void SetupUI();
    QWidget *CreateModuleSummaryWidget();
    QWidget *CreateSecondCategoryPlaceholder() const;
    QWidget *CreateButtonBar();

    QLabel *m_moduleImageLabel;
    QLabel *m_moduleNameLabel;
    QTextEdit *m_moduleDescriptionEdit;
    FmuParamWidget *m_fmuParamWidget;
    QWidget *m_secondCategoryPlaceholder;
};

#endif // PARAMSETWIDGET_H
