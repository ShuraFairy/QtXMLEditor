#pragma once

#include <QMessageBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QFormLayout>
#include <QDialog>
#include <QDialogButtonBox>

class XmlNodeDialog : public QDialog
{
    Q_OBJECT

public:
    XmlNodeDialog(const QString &title, QWidget *parent = nullptr,
                  bool showTagName = true, bool showAttributes = true);

    void setValues(const QString &tagName, const QString &attributes,
                   const QString &textContent, bool isTextNode = false);

    QString getTagName() const { return tagNameEdit->text(); }
    QString getAttributes() const { return attributesEdit->text(); }
    QString getTextContent() const { return textContentEdit->toPlainText(); }
    bool isTextNode() const { return textNodeCheckBox->isChecked(); }

private:
    QLineEdit *tagNameEdit;
    QLineEdit *attributesEdit;
    QTextEdit *textContentEdit;
    QCheckBox *textNodeCheckBox;
    QPushButton *okButton;
    QPushButton *cancelButton;
};


