#include "xmlnodedialog.h"

XmlNodeDialog::XmlNodeDialog(const QString &title, QWidget *parent,
                             bool showTagName, bool showAttributes)
    : QDialog(parent)
{
    setWindowTitle(title);
    setModal(true);
    resize(400, 300);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *formLayout = new QFormLayout();

    tagNameEdit = new QLineEdit();
    attributesEdit = new QLineEdit();
    textContentEdit = new QTextEdit();
    textContentEdit->setMaximumHeight(100);
    textNodeCheckBox = new QCheckBox(tr("Создать как текстовый узел"));

    if (showTagName) {
        formLayout->addRow(tr("Имя тега:"), tagNameEdit);
    } else {
        tagNameEdit->setVisible(false);
    }

    if (showAttributes) {
        formLayout->addRow(tr("Атрибуты:"), attributesEdit);
    } else {
        attributesEdit->setVisible(false);
    }

    formLayout->addRow(tr("Текстовое содержимое:"), textContentEdit);
    formLayout->addRow(textNodeCheckBox);

    mainLayout->addLayout(formLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    okButton = new QPushButton(tr("OK"));
    cancelButton = new QPushButton(tr("Отмена"));

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);

    connect(okButton, &QPushButton::clicked, [this, showTagName]() {
        if (!textNodeCheckBox->isChecked() && showTagName && tagNameEdit->text().isEmpty()) {
            QMessageBox::warning(this, tr("Неверный ввод"), tr("Требуется имя тега"));
            return;
        }
        accept();
    });
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(textNodeCheckBox, &QCheckBox::toggled, [this, showTagName](bool checked) {
        if (showTagName) {
            tagNameEdit->setEnabled(!checked);
        }
        attributesEdit->setEnabled(!checked);
    });

    tagNameEdit->setFocus();
}

void XmlNodeDialog::setValues(const QString &tagName, const QString &attributes,
                              const QString &textContent, bool isTextNode)
{
    tagNameEdit->setText(tagName);
    attributesEdit->setText(attributes);
    textContentEdit->setPlainText(textContent);
    textNodeCheckBox->setChecked(isTextNode);

    if (isTextNode) {
        if (tagNameEdit->isVisible()) {
            tagNameEdit->setEnabled(false);
        }
        attributesEdit->setEnabled(false);
    }
}

