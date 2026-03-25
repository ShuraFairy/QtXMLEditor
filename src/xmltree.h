#pragma once

#include <QWidget>
#include <QDomDocument>
#include <QDomNode>
#include <QStandardItem>
#include <QTreeView>
#include <QStandardItemModel>

class XmlNodeDialog;

class XmlTreeItem : public QStandardItem {
public:
    XmlTreeItem(const QString &text) : QStandardItem(text) {}

    void setDomNode(const QDomNode &node) { domNode = node; }
    QDomNode getDomNode() const { return domNode; }
    void setNodeType(const QString &type) { nodeType = type; }
    QString getNodeType() const { return nodeType; }

private:
    QDomNode domNode;
    QString  nodeType;
};

class XmlTree : public QWidget {
    Q_OBJECT

public:
    explicit XmlTree(QWidget *parent = nullptr);

    // Управление моделью данных
    void setXmlDocument(const QDomDocument &doc);
    QDomDocument getXmlDocument() const { return xmlDocument; }
    void clear();
    void refresh(); // Полное обновление дерева из QDomDocument

    // --- Методы для взаимодействия с XmlEditor ---
    // XmlEditor вызывает эти методы, XmlTree выполняет всю логику
    void requestAddNode(QWidget *parentWindow);
    void requestEditNode(QWidget *parentWindow);
    void requestRemoveNode(QWidget *parentWindow);

    // Вспомогательные методы для работы с QDomDocument
    QDomElement createElementFromAttributes(const QString &tagName, const QString &attributes);
    QString attributesToString(const QDomNamedNodeMap &attributes);

signals:
    // Уведомления для XmlEditor
    void nodeOperationFinished(); // Любая операция (добавить/редакт/удалить) завершена успешно
    void operationFailed(const QString &error); // Ошибка во время операции

private slots:
    void onItemDoubleClicked(const QModelIndex &index);
    // onDataChanged можно использовать, если нужно отслеживать редактирование в дереве напрямую

private:
    void setupUI();    
    void populateTree(const QDomNode &node, XmlTreeItem *parentItem);
    XmlTreeItem* getItemFromIndex(const QModelIndex &index) const;
    XmlTreeItem* getCurrentItem() const; // Внутренний метод

    // --- Внутренняя логика XmlTree ---
    // Эти методы реализуют действия над QDomDocument и обновляют QTreeView
    bool performAddNode(QWidget *parentWindow);
    bool performEditNode(QWidget *parentWindow);
    bool performRemoveNode(QWidget *parentWindow);

    // --- Вспомогательные методы для работы с моделью дерева (QStandardItemModel) ---
    // Эти методы позволяют XmlTree обновлять QTreeView напрямую, без полного refresh()
    bool addNodeToModel(XmlTreeItem *parentItemInModel, const QString &tagName, const QString &attributes, const QString &textContent, bool isText);
    bool editNodeInModel(XmlTreeItem *itemInModel, const QString &newTagName, const QString &newAttributes, const QString &newTextContent);
    bool removeNodeFromModel(XmlTreeItem *itemInModel);
    // --- Конец вспомогательных методов ---

    QTreeView           *treeView;
    QStandardItemModel  *treeModel;
    //QDomDocument         xmlDocument; // XmlTree владеет и управляет этой моделью данных
public:
    QDomDocument         xmlDocument; // XmlTree владеет и управляет этой моделью данных    
};

