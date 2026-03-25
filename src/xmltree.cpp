#include "xmltree.h"
#include "xmlnodedialog.h"

#include <QTreeView>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QDomElement>
#include <QDomText>
#include <QDomAttr>
#include <QMessageBox>
#include <QInputDialog>
#include <QApplication>
#include <QStyle>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QFormLayout>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDebug>

XmlTree::XmlTree(QWidget *parent) : QWidget(parent) {
    setupUI();
}

void XmlTree::setupUI() {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    treeModel = new QStandardItemModel(this);
    treeModel->setHorizontalHeaderLabels(QStringList() << tr("Тип") << tr("Имя") << tr("Атрибуты") << tr("Содержание"));

    treeView = new QTreeView();
    treeView->setModel(treeModel);
    treeView->setAlternatingRowColors(true);
    treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    treeView->setUniformRowHeights(true);
    treeView->header()->setSectionResizeMode(QHeaderView::Interactive);
    treeView->header()->setStretchLastSection(true);

    treeView->setColumnWidth(0, 80);
    treeView->setColumnWidth(1, 150);
    treeView->setColumnWidth(2, 200);
    treeView->setColumnWidth(3, 250);

    layout->addWidget(treeView);

    connect(treeView, &QTreeView::doubleClicked, this, &XmlTree::onItemDoubleClicked);
}

void XmlTree::setXmlDocument(const QDomDocument &doc) {
    xmlDocument = doc;
    refresh();
}

void XmlTree::refresh() {
    treeModel->clear();
    treeModel->setHorizontalHeaderLabels(QStringList() << tr("Тип") << tr("Имя") << tr("Атрибуты") << tr("Содержание"));

    if (!xmlDocument.isNull()) {
        populateTree(xmlDocument, nullptr);
    }
    if (treeModel->rowCount() > 0) {
        treeView->expand(treeModel->index(0, 0));
    }
}

void XmlTree::clear() {
    treeModel->clear();
    xmlDocument.clear();
    treeModel->setHorizontalHeaderLabels(QStringList() << tr("Тип") << tr("Имя") << tr("Атрибуты") << tr("Содержание"));
}

// Рекурсивно заполняет дерево в treeModel элементами, соответствующими узлам xmlDocument.
void XmlTree::populateTree(const QDomNode &node, XmlTreeItem *parentItem) {
    // Получаем список дочерних узлов текущего узла.
    QDomNodeList children = node.childNodes();

    for (int i = 0; i < children.count(); ++i) {
        QDomNode childNode = children.at(i);    // Проходимся по каждому узлу.
        // Создаем элементы строки
        QList<QStandardItem*> rowItems;
        XmlTreeItem *typeItem = new XmlTreeItem(tr(""));    // тип узла (Элемент, Текст, Узел).
        XmlTreeItem *nameItem = new XmlTreeItem(tr(""));    // имя узла (например, тег элемента).
        XmlTreeItem *attrItem = new XmlTreeItem(tr(""));    // атрибуты (если есть).
        XmlTreeItem *contentItem = new XmlTreeItem(tr("")); // содержимое узла (например, текст внутри тега).

        if (childNode.isElement()) {
            QDomElement element = childNode.toElement();
            typeItem->setText(tr("Элемент"));
            nameItem->setText(element.tagName());
            attrItem->setText(attributesToString(element.attributes()));
            QString textContent;
            QDomNodeList elementChildren = element.childNodes();
            for (int j = 0; j < elementChildren.count(); ++j) {
                if (elementChildren.at(j).isText()) {
                    if (!textContent.isEmpty()) textContent += " ";
                    textContent += elementChildren.at(j).toText().data();
                }
            }

            // Ограничиваем длину содержимого до 50 символов и добавляем ..., если длиннее.
            contentItem->setText(textContent.left(50) + (textContent.length() > 50 ? tr("...") : tr("")));
            typeItem->setNodeType(tr("Элемент"));
        } else if (childNode.isText()) {
            QDomText text = childNode.toText();
            typeItem->setText(tr("Текст"));
            nameItem->setText(tr("[текст]"));
            QString content = text.data();

            // Ограничиваем длину содержимого до 50 символов и добавляем ..., если длиннее.
            contentItem->setText(content.left(50) + (content.length() > 50 ? tr("...") : tr("")));
            typeItem->setNodeType(tr("Текст"));
        } else {
            typeItem->setText(tr("Узел"));
            nameItem->setText(childNode.nodeName());
            typeItem->setNodeType(tr("Узел"));
        }
        typeItem->setDomNode(childNode); // Связываем элемент модели с узлом DOM

        rowItems << typeItem << nameItem << attrItem << contentItem;
        if (parentItem) {
            parentItem->appendRow(rowItems);
        } else {
            treeModel->invisibleRootItem()->appendRow(rowItems);
        }
        populateTree(childNode, typeItem);
    }
}

XmlTreeItem* XmlTree::getItemFromIndex(const QModelIndex &index) const {
    if (!index.isValid()) return nullptr;
    QModelIndex firstColumnIndex = treeModel->index(index.row(), 0, index.parent());
    QStandardItem *item = treeModel->itemFromIndex(firstColumnIndex);
    return dynamic_cast<XmlTreeItem*>(item);
}

XmlTreeItem* XmlTree::getCurrentItem() const {
    return getItemFromIndex(treeView->currentIndex());
}

QString XmlTree::attributesToString(const QDomNamedNodeMap &attributes) {
    QString result;
    for (int i = 0; i < attributes.count(); ++i) {
        if (!result.isEmpty()) result += tr(", ");
        QDomAttr attr = attributes.item(i).toAttr();
        result += attr.name() + tr("=") + attr.value();
    }
    return result;
}

QDomElement XmlTree::createElementFromAttributes(const QString &tagName, const QString &attributes) {
    QDomElement element = xmlDocument.createElement(tagName);
    QStringList attrPairs = attributes.split(tr(","), Qt::SkipEmptyParts);
    for (const QString &pair : attrPairs) {
        QStringList keyValue = pair.split(tr("="));
        if (keyValue.size() == 2) {
            element.setAttribute(keyValue[0].trimmed(), keyValue[1].trimmed().remove('"'));
        }
    }
    return element;
}

// --- Методы для взаимодействия с XmlEditor ---
void XmlTree::requestAddNode(QWidget *parentWindow) {
    if (performAddNode(parentWindow)) {
        emit nodeOperationFinished();
    }
    // operationFailed испускается внутри performAddNode при ошибке
}

void XmlTree::requestEditNode(QWidget *parentWindow) {
    if (performEditNode(parentWindow)) {
        emit nodeOperationFinished();
    }
    // operationFailed испускается внутри performEditNode при ошибке
}

void XmlTree::requestRemoveNode(QWidget *parentWindow) {
    if (performRemoveNode(parentWindow)) {
        emit nodeOperationFinished();
    }
    // operationFailed испускается внутри performRemoveNode при ошибке
}

bool XmlTree::performAddNode(QWidget *parentWindow) {
    QDomNode parentDomNode;
    XmlTreeItem *currentTreeItem = getCurrentItem();

    // Получает текущий выделенный элемент в QTreeView (getCurrentItem()).
    // Извлекает соответствующий ему QDomNode из QDomDocument.
    // Определяет, какой элемент в QDomDocument будет родителем для нового узла:
    // Если выделен элемент, новый узел добавляется внутрь него.
    // Если выделен не-элемент (например, текст), новый узел добавляется внутрь родительского элемента выделенного узла.
    // Если ничего не выделено или не удается определить родителя, новый узел добавляется внутрь корневого элемента документа (xmlDocument.documentElement()).

    if (currentTreeItem) {
        QDomNode currentDomNode = currentTreeItem->getDomNode();
        if (currentDomNode.isElement()) {
            parentDomNode = currentDomNode;
        } else if (!currentDomNode.parentNode().isNull() && currentDomNode.parentNode().isElement()) {
            parentDomNode = currentDomNode.parentNode();
        }
    }

    // Если корневой элемент не существует (пустой документ), parentDomNode останется isNull().
    if (parentDomNode.isNull()) {
        parentDomNode = xmlDocument.documentElement();
    }

    // Если по-прежнему не удалось определить родителя (например, документ полностью пуст), отображается сообщение об ошибке и испускается сигнал operationFailed.
    if (parentDomNode.isNull()) {
        QString error = tr("Невозможно определить родительский узел для нового узла");
        QMessageBox::warning(parentWindow, tr("Добавить элемент"/*"Add Node"*/), error);
        emit operationFailed(error);
        return false;
    }

    XmlNodeDialog dialog(tr("Добавить элемент"/*"Add Node"*/), parentWindow, true, true);
    if (dialog.exec() == QDialog::Accepted) {
        // Получение данных из диалога
        QString tagName = dialog.getTagName();
        QString attributes = dialog.getAttributes();
        QString textContent = dialog.getTextContent();
        bool isText = dialog.isTextNode();

        if (isText) {
            // Получение данных из диалога:
            // Проверяется, не пустой ли текст.
            // Создается новый QDomText узел с помощью xmlDocument.createTextNode().
            // Новый текстовый узел добавляется как дочерний к parentDomNode с помощью appendChild(). Это изменение в QDomDocument.
            // Обновление визуального представления: Вызывается addNodeToModel(...) (вспомогательный метод XmlTree), чтобы немедленно
            // добавить новый узел в QStandardItemModel, связанную с QTreeView, без полного перестроения дерева.

            if (!textContent.isEmpty()) {
                QDomText textNode = xmlDocument.createTextNode(textContent);
                parentDomNode.appendChild(textNode);
                // Обновляем визуальное представление
                addNodeToModel(currentTreeItem, tr(""), tr(""), textContent, true);
                return true;
            }
        } else {
            // Добавление элемента:
            // Проверяется, введено ли имя тега.
            // Если имя тега введено:
            //   Создается новый QDomElement с помощью вспомогательного метода createElementFromAttributes, который обрабатывает строку атрибутов.
            //   Если введен текст, создается QDomText и добавляется внутрь нового элемента.
            //   Новый элемент добавляется как дочерний к parentDomNode в QDomDocument.
            //   Обновление визуального представления: Вызывается addNodeToModel(...) для немедленного отображения нового элемента в дереве. Функция возвращает true.
            // Если имя тега не введено:
            //   Показывается предупреждение и испускается сигнал operationFailed. Функция возвращает false.

            if (!tagName.isEmpty()) {
                QDomElement newElement = createElementFromAttributes(tagName, attributes);
                if (!textContent.isEmpty()) {
                    QDomText textNode = xmlDocument.createTextNode(textContent);
                    newElement.appendChild(textNode);
                }
                parentDomNode.appendChild(newElement);
                // Обновляем визуальное представление
                addNodeToModel(currentTreeItem, tagName, attributes, textContent, false);
                return true;
            } else {
                QString error = tr("Требуется имя тега");
                QMessageBox::warning(parentWindow, tr("Добавление элемента"), error);
                emit operationFailed(error);
            }
        }
    }
    return false;
}

// Цель функции: Изменить содержимое или свойства уже существующего узла (QDomNode) в QDomDocument, управляемом XmlTree, и немедленно обновить визуальное представление этого изменения в QTreeView.
bool XmlTree::performEditNode(QWidget *parentWindow) {

    // Получение выделенного элемента:
    // ызывает внутренний метод getCurrentItem(), который, в свою очередь, получает текущий выделенный индекс из treeView и преобразует его в XmlTreeItem.
    // Если ничего не выделено (currentTreeItem равен nullptr), метод показывает предупреждение пользователю с помощью QMessageBox::warning, испускает сигнал operationFailed с сообщением об ошибке и возвращается false.
    XmlTreeItem *currentTreeItem = getCurrentItem();
    if (!currentTreeItem) {
        QString error = tr("Выберите узел для редактирования");
        QMessageBox::warning(parentWindow, tr("Редактирование элемента"), error);
        emit operationFailed(error);
        return false;
    }

    // Получение связанного узла DOM:
    // Извлекает реальный узел DOM (QDomNode), который соответствует выделенному элементу дерева, используя метод getDomNode() класса XmlTreeItem.
    // Проверяет, является ли полученный узел DOM действительным (не null). Если он null, показывает предупреждение, испускает operationFailed и возвращается false.
    QDomNode domNode = currentTreeItem->getDomNode();
    if (domNode.isNull()) {
        QString error = tr("Невозможно редактировать этот элемент");
        QMessageBox::warning(parentWindow, tr("Редактирование элемента"), error);
        emit operationFailed(error);
        return false;
    }

    if (domNode.isText()) {
        // Редактирование текстового узла:
        // Преобразует QDomNode в QDomText с помощью domNode.toText().
        // Использует стандартный диалог Qt QInputDialog::getMultiLineText, чтобы показать пользователю поле для ввода многострочного текста. Диалог предварительно заполняется
        // текущим содержимым текстового узла (textNode.data()). parentWindow используется как родитель диалога.
        // Если пользователь нажал "OK" (ok равно true):
        // Обновляет данные в модели данных: Вызывает метод textNode.setData(newText), который непосредственно изменяет QDomDocument, управляемый XmlTree.
        //Обновляет визуальное представление: Вызывает вспомогательный метод editNodeInModel(...), передавая ему указатель на XmlTreeItem в модели и новые данные. Этот метод
        // напрямую изменяет QStandardItemModel, связанную с treeView, чтобы обновить отображение текста без полной перерисовки дерева.
        QDomText textNode = domNode.toText();
        bool ok;
        QString newText = QInputDialog::getMultiLineText(parentWindow, tr("Редактировать текстовый элемент"),
                                                         tr("Текстовое содержание:"), textNode.data(), &ok);
        if (ok) {
            textNode.setData(newText);
            // Обновляем визуальное представление
            editNodeInModel(currentTreeItem, tr(""), tr(""), newText);
            return true;
        }
    } else if (domNode.isElement()) {
        // Редактирование элемента:
        // Преобразует QDomNode в QDomElement.
        // Извлекает текущие данные элемента (имя тега, атрибуты, текстовое содержимое из дочерних текстовых узлов) для предварительного заполнения диалога.
        // Создает и показывает XmlNodeDialog: Передает parentWindow как родителя. Метод setValues заполняет поля диалога текущими данными.
        // Если пользователь нажал "OK" в диалоге (dialog.exec() == QDialog::Accepted):
        //      Получает новые данные из диалога.
        //      Обновляет данные в модели данных (QDomDocument):
        //          Имя тега: Использует element.setTagName(...).
        //          Атрибуты:
        //              Удаляет все существующие атрибуты (removeAttribute).
        //              Разбирает строку новых атрибутов и добавляет их (setAttribute).
        //          Текстовое содержимое:
        //              Удаляет все существующие дочерние текстовые узлы (removeChild).
        //              Создает новый текстовый узел (xmlDocument.createTextNode) и добавляет его внутрь редактируемого элемента (element.appendChild).
        //      Обновляет визуальное представление: Вызывает editNodeInModel(...) с новыми данными, чтобы немедленно обновить QTreeView.

        QDomElement element = domNode.toElement();
        QString tagName = element.tagName();
        QString attributes = attributesToString(element.attributes());
        QString textContent;
        QDomNodeList children = element.childNodes();
        for (int i = 0; i < children.count(); ++i) {
            if (children.at(i).isText()) {
                textContent += children.at(i).toText().data();
            }
        }

        XmlNodeDialog dialog(tr("Редактирование элемента"), parentWindow, true, true);
        dialog.setValues(tagName, attributes, textContent);

        if (dialog.exec() == QDialog::Accepted) {
            // --- Обновление QDomDocument ---
            QString newTagName = dialog.getTagName();
            QString newAttributes = dialog.getAttributes();
            QString newTextContent = dialog.getTextContent();

            if (!newTagName.isEmpty() && newTagName != tagName) {
                element.setTagName(newTagName);
            }

            // Обновление атрибутов
            QDomNamedNodeMap oldAttrs = element.attributes();
            QVector<QDomAttr> attrsToRemove;
            for (int i = 0; i < oldAttrs.count(); ++i) {
                attrsToRemove.append(oldAttrs.item(i).toAttr());
            }
            for (const QDomAttr& attr : attrsToRemove) {
                element.removeAttribute(attr.name());
            }
            if (!newAttributes.isEmpty()) {
                QStringList attrPairs = newAttributes.split(tr(","), Qt::SkipEmptyParts);
                for (const QString &pair : attrPairs) {
                    QStringList keyValue = pair.split(tr("="));
                    if (keyValue.size() == 2) {
                        element.setAttribute(keyValue[0].trimmed(), keyValue[1].trimmed());
                    }
                }
            }

            // Обновление текстового содержимого
            QDomNodeList childNodes = element.childNodes();
            for (int i = childNodes.count() - 1; i >= 0; --i) {
                if (childNodes.at(i).isText()) {
                    element.removeChild(childNodes.at(i));
                }
            }
            if (!newTextContent.isEmpty()) {
                QDomText textNode = xmlDocument.createTextNode(newTextContent);
                element.appendChild(textNode);
            }
            // --- Конец обновления QDomDocument ---

            // Обновляем визуальное представление
            editNodeInModel(currentTreeItem, newTagName, newAttributes, newTextContent);
            return true;
        }
    } else {
        QString error = tr("Редактирование этого типа узла не поддерживается");
        QMessageBox::information(parentWindow, tr("Редактирование элемента"), error);
        // Не считаем это критической ошибкой, просто информируем
    }
    return false;
}

//
bool XmlTree::performRemoveNode(QWidget *parentWindow) {
    // Получение выделенного элемента:
    // getCurrentItem() — это внутренний метод XmlTree, который получает текущий выделенный элемент из treeView.
    // Если ничего не выделено (currentTreeItem равен nullptr), метод показывает предупреждение пользователю через модальное окно (QMessageBox::warning), используя parentWindow как родителя окна.
    // Он испускает сигнал operationFailed, передавая сообщение об ошибке. Это позволяет XmlEditor (который подключен к этому сигналу) узнать, что операция не удалась.
    XmlTreeItem *currentTreeItem = getCurrentItem();
    if (!currentTreeItem) {
        QString error = tr("Выберите элемент для удаления.");
        QMessageBox::warning(parentWindow, tr("Удаление элемента"), error);
        emit operationFailed(error);
        return false;
    }

    // Получение связанного узла DOM:
    // Извлекает реальный узел QDomNode из XmlTreeItem. Это узел в xmlDocument, который соответствует выделенному элементу в дереве.
    // Если QDomNode не действителен (isNull()), это означает внутреннюю ошибку или проблему с синхронизацией. Показывается ошибка, испускается сигнал operationFailed, и метод возвращает false.
    QDomNode domNode = currentTreeItem->getDomNode();
    if (domNode.isNull()) {
        QString error = tr("Невозможно удалить этот элемент");
        QMessageBox::warning(parentWindow, tr("Удаление элемента"), error);
        emit operationFailed(error);
        return false;
    }

    QString nodeType = domNode.isText() ? tr("текстовый узел") : domNode.isElement() ? tr("элемент") : tr("узел");
    int ret = QMessageBox::question(parentWindow, tr("Удаление элемента"),
                                    QString(tr("Вы уверены, что хотите удалить %1?")).arg(nodeType),
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        // Фактическое удаление из QDomDocument:
        // Получает родительский узел удаляемого узла (domNode.parentNode()). Удаление в DOM происходит через вызов removeChild у родителя.
        // Проверяет, существует ли родитель (!parent.isNull()). Попытка удалить корневой узел документа или узел без родителя приведет к ошибке.
        // Если родитель существует:
        //      parent.removeChild(domNode); — Это ключевой момент. Эта строка изменяет xmlDocument, удаляя domNode и всех его потомков из структуры данных.
        //      removeNodeFromModel(currentTreeItem); — Обновление UI. Этот вызов просит XmlTree немедленно удалить соответствующую строку из QStandardItemModel,
        //      не перестраивая всё дерево. Это делает UI отзывчивым.
        //      return true; — Метод возвращает true, сигнализируя, что операция успешно завершена. requestRemoveNode затем испустит сигнал nodeOperationFinished.
        // Если родителя нет:
        //      Показывается сообщение об ошибке.
        //      Испускается сигнал operationFailed.
        QDomNode parent = domNode.parentNode();
        if (!parent.isNull()) {
            parent.removeChild(domNode);
            // Обновляем визуальное представление
            removeNodeFromModel(currentTreeItem);
            return true;
        } else {
            QString error = tr("Невозможно удалить корневой или потерянный элемент");
            QMessageBox::warning(parentWindow, tr("Удаление узла"), error);
            emit operationFailed(error);
        }
    }
    return false;
}

void XmlTree::onItemDoubleClicked(const QModelIndex &index) {
    Q_UNUSED(index)
    // Двойной клик может запускать редактирование, но это решает XmlEditor
    // XmlEditor подключится к сигналу itemDoubleClicked от treeView, если нужно
}

// Функция addNodeToModel — это вспомогательный метод класса XmlTree, который напрямую изменяет визуальное представление XML-документа
// (то есть QStandardItemModel, лежащую в основе QTreeView) без изменения основной модели данных (QDomDocument).
// Её задача — немедленно отобразить в дереве новый узел, который уже был добавлен в QDomDocument другим кодом (например, в XmlEditor или в другом методе XmlTree).
// Это делает интерфейс более отзывчивым, чем если бы мы каждый раз перестраивали всё дерево заново через refresh().

// Цель йункции: Добавить новую строку (представляющую XML-узел) в модель QTreeView (QStandardItemModel) в указанное место.
bool XmlTree::addNodeToModel(XmlTreeItem *parentItemInModel, const QString &tagName, const QString &attributes, const QString &textContent, bool isText) {
    if (!treeModel) {
        qWarning() << tr("XmlTree::addNodeToModel: Модель имеет значение null");
        return false;
    }

    QStandardItem *actualParentItem = nullptr;
    // Если parentItemInModel передан, код пытается получить соответствующий QStandardItem из treeModel (берётся первый столбец строки, соответствующей parentItemInModel).
    // Это необходимо, потому что appendRow должен вызываться у QStandardItem, представляющего узел.
    if (parentItemInModel) {
        QModelIndex parentIndex = treeModel->indexFromItem(parentItemInModel);
        if (parentIndex.isValid()) {
            actualParentItem = treeModel->itemFromIndex(treeModel->index(parentIndex.row(), 0, parentIndex.parent()));
        }
    }
    // Если parentItemInModel не передан или не найден, новая строка будет добавлена в корень дерева (treeModel->invisibleRootItem()).
    if (!actualParentItem) {
        actualParentItem = treeModel->invisibleRootItem();
    }

    // Создаётся список (QList) из четырёх QStandardItem (или его наследника XmlTreeItem), которые будут представлять один узел в четырёх столбцах дерева (Тип, Имя, Атрибуты, Содержимое).
    // Для текстового узла:
    //      typeItem = "Text"
    //      nameItem = "[text]"
    //      attrItem = "" (пусто)
    //      contentItem = textContent (обрезанный до 50 символов)
    // Для элемента:
    //      typeItem = "Element"
    //      nameItem = tagName
    //      attrItem = attributes
    //      contentItem = textContent (обрезанный до 50 символов)
    // Важно: typeItem создаётся как XmlTreeItem, но ему не устанавливается setDomNode, потому что эта функция работает только с моделью, а не с QDomDocument.
    QList<QStandardItem*> rowItems;
    XmlTreeItem *typeItem, *nameItem, *attrItem, *contentItem;

    if (isText) {
        typeItem = new XmlTreeItem(tr("Текст"));
        nameItem = new XmlTreeItem(tr("[текст]"));
        attrItem = new XmlTreeItem(tr(""));
        contentItem = new XmlTreeItem(textContent.left(50) + (textContent.length() > 50 ? tr("...") : tr("")));
        typeItem->setNodeType(tr("Текст"));
    } else {
        typeItem = new XmlTreeItem(tr("Элемент"));
        nameItem = new XmlTreeItem(tagName);
        attrItem = new XmlTreeItem(attributes);
        contentItem = new XmlTreeItem(textContent.left(50) + (textContent.length() > 50 ? tr("...") : tr("")));
        typeItem->setNodeType(tr("Элемент"));
    }

    // Добавление строки в модель:
    // Список созданных элементов (rowItems) добавляется как новая дочерняя строка к actualParentItem в treeModel. Именно это вызывает появление нового узла в QTreeView.
    rowItems << typeItem << nameItem << attrItem << contentItem;    
    actualParentItem->appendRow(rowItems);

    // Раскрытие родителя:
    // Чтобы пользователь сразу увидел добавленный узел, его родитель в QTreeView раскрывается.
    if (parentItemInModel) {
        QModelIndex parentModelIndex = treeModel->indexFromItem(actualParentItem);
        treeView->expand(parentModelIndex);
    }
    return true;
}

// Цель функции :
// Обновить информацию, отображаемую в строке дерева (в QTreeView), соответствующей определенному узлу XML, чтобы она отражала изменения, внесенные в QDomDocument.
// Это делается напрямую в модели QStandardItemModel, а не через xmlTree->setXmlDocument(domDocument) и populateTree, что было бы менее эффективно.
bool XmlTree::editNodeInModel(XmlTreeItem *itemInModel, const QString &newTagName, const QString &newAttributes, const QString &newTextContent) {
    if (!itemInModel || !treeModel) {
        qWarning() << tr("XmlTree::editNodeInModel: Элемент или модель имеют значение null");
        return false;
    }

    // Поиск строки в модели:
    // Получает модельный индекс (QModelIndex) для переданного itemInModel. Этот индекс указывает на конкретную строку и столбец в QStandardItemModel. Проверяется его корректность.
    QModelIndex itemIndex = treeModel->indexFromItem(itemInModel);
    if (!itemIndex.isValid()) {
        qWarning() << tr("XmlTree::editNodeInModel: Недопустимый индекс элемента");
        return false;
    }

    // Определение позиции строки:
    // Извлекает номер строки (row) и индекс родителя (parentIndex) из itemIndex. Это нужно, чтобы найти все элементы (ячейки) в этой строке, так как каждый XmlTreeItem
    // в строке отвечает за отдельный столбец (Тип, Имя, Атрибуты, Содержимое).
    int row = itemIndex.row();
    QModelIndex parentIndex = itemIndex.parent();

    QStandardItem *baseItem = treeModel->itemFromIndex(treeModel->index(row, 0, parentIndex));
    QStandardItem *nameItem = treeModel->itemFromIndex(treeModel->index(row, 1, parentIndex));
    QStandardItem *attrItem = treeModel->itemFromIndex(treeModel->index(row, 2, parentIndex));
    QStandardItem *contentItem = treeModel->itemFromIndex(treeModel->index(row, 3, parentIndex));

    // Проверка доступности элементов строки
    if (!baseItem || !nameItem || !attrItem || !contentItem) {
        qWarning() << tr("XmlTree::editNodeInModel: Не удалось найти все элементы в строке");
        return false;
    }

    QString nodeType = itemInModel->getNodeType();

    if (nodeType == tr("Текст")) {
        // Для текстового узла:
        // Обновляется только содержимое ячейки Содержимое (contentItem).
        // Текст обрезается до 50 символов с добавлением "...", если он длиннее.
        // Имя ("[text]") и атрибуты (пусто) обычно не меняются для текстовых узлов.
        contentItem->setText(newTextContent.left(50) + (newTextContent.length() > 50 ? tr("...") : tr("")));
        return true;
    } else if (nodeType == tr("Элемент")) {
        // Для элемента:
        // Если newTagName не пусто, обновляется ячейка Имя (nameItem).
        // Если newAttributes не null (что позволяет передать пустую строку), обновляется ячейка Атрибуты (attrItem).
        // Если newTextContent не null, обновляется ячейка Содержимое (contentItem).

        if (!newTagName.isEmpty()) {
            nameItem->setText(newTagName);
        }
        if (!newAttributes.isNull()) {
            attrItem->setText(newAttributes);
        }
        if (!newTextContent.isNull()) {
            contentItem->setText(newTextContent.left(50) + (newTextContent.length() > 50 ? tr("...") : tr("")));
        }
        return true;
    }
    return false;
}

bool XmlTree::removeNodeFromModel(XmlTreeItem *itemInModel) {
    if (!itemInModel || !treeModel) {
        qWarning() << tr("XmlTree::removeNodeFromModel: Элемент или модель имеют значение null");
        return false;
    }

    QModelIndex itemIndex = treeModel->indexFromItem(itemInModel);
    if (!itemIndex.isValid()) {
        qWarning() << tr("XmlTree::removeNodeFromModel: Недопустимый индекс элемента");
        return false;
    }

    // Удаление строки из модели:
    // itemIndex.row(): Получает номер строки (начиная с 0), которую представляет itemInModel.
    // itemIndex.parent(): Получает QModelIndex родительского элемента для itemInModel. Если itemInModel находится на верхнем уровне дерева, itemIndex.parent() будет недействительным (isValid() будет false), что QStandardItemModel интерпретирует как корень.
    // treeModel->removeRow(...): Этот метод QStandardItemModel удаляет строку с указанным номером из указанного родительского элемента (или из корня, если родитель недействителен).
    // Это приводит к немедленному удалению строки из treeModel, и QTreeView, связанный с этой моделью, автоматически обновляется, убирая соответствующий узел из визуального представления.
    treeModel->removeRow(itemIndex.row(), itemIndex.parent());
    return true;
}

