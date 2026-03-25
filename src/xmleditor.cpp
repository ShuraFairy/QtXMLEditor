#include "xmleditor.h"
#include "xmltree.h"

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QFile>
#include <QTextStream>
#include <QDockWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QSplitter>
#include <QTextEdit>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QThread>
#include <QtConcurrent/QtConcurrent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCloseEvent>
#include <QTimer>
#include <QComboBox>


XmlEditor::XmlEditor(QWidget *parent)
    : QMainWindow(parent), isModified(false) {

    validator = new XmlValidator(this);
    //createCentralWidget();
    //createXMLDeviceDescriptionWidget();
    createMenus();
    createToolBars();
    createStatusBar();

    currentFileName.clear();

    //setDarkPalette();

    setWindowTitle(tr("XML редактор"));
    resize(1200, 800);
}

void XmlEditor::createCentralWidget() {
    mainSplitter = new QSplitter(Qt::Horizontal);

    xmlTree = new XmlTree();
    xmlTree->setMinimumWidth(400);

    xmlTextEdit = new QTextEdit();
    xmlTextEdit->setLineWrapMode(QTextEdit::NoWrap);
    xmlTextEdit->setFont(QFont("Courier New", 10));
    xmlTextEdit->setMinimumWidth(400);

    mainSplitter->addWidget(xmlTree);
    mainSplitter->addWidget(xmlTextEdit);
    mainSplitter->setSizes(QList<int>() << 500 << 700);

    setCentralWidget(mainSplitter);

    //connect(xmlTree->treeView, &QTreeView::doubleClicked, this, &XmlEditor::onTreeItemDoubleClicked);
    connect(xmlTree, &XmlTree::nodeOperationFinished, this, &XmlEditor::onNodeOperationFinished);
    connect(xmlTree, &XmlTree::operationFailed, this, &XmlEditor::onOperationFailed);

    isOpenXMLDeviceDescription = false;

    editToolBar->setVisible(true);
    updateToolBar->setVisible(true);
    validationToolBar->setVisible(true);

    editMenu->menuAction()->setVisible(true);
    viewMenu->menuAction()->setVisible(true);
    validationMenu->menuAction()->setVisible(true);
}

void XmlEditor::closeCentralWidget()
{
    mainSplitter->setVisible(false);

    isOpenXML = false;
}

void XmlEditor::createXMLDeviceDescriptionWidget()
{    
    editToolBar->setVisible(false);
    updateToolBar->setVisible(false);
    validationToolBar->setVisible(false);

    editMenu->menuAction()->setVisible(false);
    viewMenu->menuAction()->setVisible(false);
    validationMenu->menuAction()->setVisible(false);

    attributesPopup = new AttributesPopup(this);
    attributesPopup->hide(); // Скрыто по умолчанию

    // Подключаем сигнал от всплывающего окна к слоту в главном окне
    connect(attributesPopup, &AttributesPopup::attributeChanged, this, &XmlEditor::onAttributeChangedFromPopup); 

    QWidget *centralWidget = new QWidget(this);    
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);    

    tabWidget = new QTabWidget();

    typesModel = new TypesModel(this);
    typesView = new QTableView();
    typesView->setModel(typesModel);    
    QHeaderView *header = typesView->horizontalHeader();
    // Установить размер по содержимому
    header->setSectionResizeMode(QHeaderView::ResizeToContents);
    header->setStretchLastSection(true);
    //header->setSectionResizeMode(QHeaderView::Stretch);
    typesView->setItemDelegate(new TextEditDelegate(this));

    parametersModel = new ParametersModel(this);
    parametersView = new QTableView();
    parametersView->setModel(parametersModel);
    parametersView->setSelectionBehavior(QAbstractItemView::SelectRows);    
    QHeaderView *headerParametersView = parametersView->horizontalHeader();
    // Установить размер по содержимому
    headerParametersView->setSectionResizeMode(QHeaderView::ResizeToContents);
    headerParametersView->setStretchLastSection(true);
    //headerParametersView->setSectionResizeMode(QHeaderView::Stretch);
    parametersView->setContextMenuPolicy(Qt::CustomContextMenu);
    //connect(parametersView, &QWidget::customContextMenuRequested, this, &XmlEditor::onParametersContextMenuRequested);
    connect(parametersView, &QWidget::customContextMenuRequested, this, &XmlEditor::onParametersContextMenuRequested2);
    parametersView->setItemDelegate(new TextEditDelegate(this));

    connectorsModel = new ConnectorsModel(this);
    connectorsView = new QTableView();
    connectorsView->setModel(connectorsModel);
    connectorsView->setSelectionBehavior(QAbstractItemView::SelectRows);
    connectorsView->setItemDelegateForColumn(2, new RoleComboBoxDelegate(this));
    connectorsView->setEditTriggers(QAbstractItemView::AllEditTriggers);    
    QHeaderView *headerConnectorsView = connectorsView->horizontalHeader();
    // Установить размер по содержимому
    headerConnectorsView->setSectionResizeMode(QHeaderView::ResizeToContents);
    headerConnectorsView->setStretchLastSection(true);
    //headerConnectorsView->setSectionResizeMode(QHeaderView::Stretch);
    connectorsView->setItemDelegate(new TextEditDelegate(this));

    deviceInfoModel = new DeviceInfoModel(this);
    deviceInfoView = new QTableView();
    deviceInfoView->setModel(deviceInfoModel);
    deviceInfoView->setSelectionBehavior(QAbstractItemView::SelectRows);    
    QHeaderView *headerDeviceInfoView = deviceInfoView->horizontalHeader();
    // Установить размер по содержимому
    headerDeviceInfoView->setSectionResizeMode(QHeaderView::ResizeToContents);
    headerDeviceInfoView->setStretchLastSection(true);
    //headerDeviceInfoView->setSectionResizeMode(QHeaderView::Stretch);
    deviceInfoView->setItemDelegate(new TextEditDelegate(this));

    filesModel = new FilesModel(this);
    filesView = new QTableView();
    filesView->setModel(filesModel);
    filesView->setSelectionBehavior(QAbstractItemView::SelectRows);    
    QHeaderView *headerFilesView = filesView->horizontalHeader();
    // Установить размер по содержимому
    headerFilesView->setSectionResizeMode(QHeaderView::ResizeToContents);
    headerFilesView->setStretchLastSection(true);
    filesView->setItemDelegate(new TextEditDelegate(this));

    deviceIdentificationModel = new DeviceIdentificationModel(this);
    deviceIdentificationView = new QTableView();
    deviceIdentificationView->setModel(deviceIdentificationModel);
    deviceIdentificationView->setSelectionBehavior(QAbstractItemView::SelectRows);    
    QHeaderView *headerDeviceIdentificationView = deviceIdentificationView->horizontalHeader();
    // Установить размер по содержимому
    headerDeviceIdentificationView->setSectionResizeMode(QHeaderView::ResizeToContents);
    headerDeviceIdentificationView->setStretchLastSection(true);
    deviceIdentificationView->setItemDelegate(new TextEditDelegate(this));

    tabWidget->addTab(typesView, "Types (Table)");
    tabWidget->addTab(parametersView, "Parameters");
    tabWidget->addTab(connectorsView, "Connectors");
    tabWidget->addTab(deviceInfoView, "Device Info");
    tabWidget->addTab(filesView, "Files");
    tabWidget->addTab(deviceIdentificationView, "Device Identification");

    mainLayout->addWidget(tabWidget);


    //QMenu menu;
    if(menu == nullptr)
        menu = new QMenu();
    if(isNew == false)
    {
        menu->removeAction(deleteParameterAction);
        deleteParameterAction = nullptr;
        menu->removeAction(addDiscreteInputParameterAction);
        addDiscreteInputParameterAction = nullptr;
        menu->removeAction(addDiscreteOutputParameterAction);
        addDiscreteOutputParameterAction = nullptr;
        menu->removeAction(addAnalogInputParameterAction);
        addAnalogInputParameterAction = nullptr;
        menu->removeAction(addAnalogOutputParameterAction);
        addAnalogOutputParameterAction = nullptr;
        menu->removeAction(addConfigParameterAction);
        addConfigParameterAction = nullptr;
    }

    if(isNew == true)
    {
    if(deleteParameterAction == nullptr)
        deleteParameterAction = menu->addAction(tr("Удалить Параметр"));
    if(addDiscreteInputParameterAction == nullptr)
        addDiscreteInputParameterAction = menu->addAction(tr("Добавить Дискретный Входной Параметр (ID 100X)"));
    if(addDiscreteOutputParameterAction == nullptr)
        addDiscreteOutputParameterAction = menu->addAction(tr("Добавить Дискретный Выходной Параметр (ID 200X)"));
    if(addAnalogInputParameterAction == nullptr)
        addAnalogInputParameterAction = menu->addAction(tr("Добавить Аналоговый Входной Параметр (ID 300X)"));
    if(addAnalogOutputParameterAction == nullptr)
        addAnalogOutputParameterAction = menu->addAction(tr("Добавить Аналоговый Выходной Параметр (ID 400X)"));
    if(addConfigParameterAction == nullptr)
        addConfigParameterAction = menu->addAction(tr("Добавить Параметр Конфигурации (ID 40000X)"));
    }
    if(editAttrs == nullptr)
        editAttrs = menu->addAction(tr("Показать/Редактировать атрибуты"));

    if(isNew == true)
    {
    connect(deleteParameterAction, &QAction::triggered, this, &XmlEditor::onDeleteParameterTriggered);
    connect(addDiscreteInputParameterAction, &QAction::triggered, this, &XmlEditor::onAddDiscreteInputChannelTriggered);
    connect(addDiscreteOutputParameterAction, &QAction::triggered, this, &XmlEditor::onAddDiscreteOutputChannelTriggered);
    connect(addAnalogInputParameterAction, &QAction::triggered, this, &XmlEditor::onAddAnalogInputChannelTriggered);
    connect(addAnalogOutputParameterAction, &QAction::triggered, this, &XmlEditor::onAddAnalogOutputChannelTriggered);
    connect(addConfigParameterAction, &QAction::triggered, this, &XmlEditor::onAddConfigParameterTriggered);
    }

    connect(&future_watcher, &QFutureWatcher<void>::finished, this, &XmlEditor::onParseFinished);

    // Подключаем dataChanged
    connect(typesModel, &QAbstractItemModel::dataChanged, this, &XmlEditor::markAsChangedForModel);
    connect(parametersModel, &QAbstractItemModel::dataChanged, this, &XmlEditor::markAsChangedForModel);
    connect(connectorsModel, &QAbstractItemModel::dataChanged, this, &XmlEditor::markAsChangedForModel);
    connect(deviceInfoModel, &QAbstractItemModel::dataChanged, this, &XmlEditor::markAsChangedForModel);
    connect(filesModel, &QAbstractItemModel::dataChanged, this, &XmlEditor::markAsChangedForModel);
    connect(deviceIdentificationModel, &QAbstractItemModel::dataChanged, this, &XmlEditor::markAsChangedForModel);
}

void XmlEditor::closeXMLDeviceDescriptionWidget()
{
    mainSplitter->setVisible(true);

    isOpenXMLDeviceDescription = false;

     hasUnsavedChanges = false;
     isModified = false;
}

void XmlEditor::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("Файл"));
    editMenu = menuBar()->addMenu(tr("Редактировать"));
    viewMenu = menuBar()->addMenu(tr("Вид"));
    validationMenu = menuBar()->addMenu(tr("Валидация XML"));
    helpMenu = menuBar()->addMenu(tr("&Помощь"));

    // File menu
    const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
    newAct = new QAction(newIcon, tr("&New"), this);
    newAct->setShortcut(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new XML file"));
    connect(newAct, &QAction::triggered, this, &XmlEditor::newFile);
    fileMenu->addAction(newAct);

    const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(":/images/open.png"));
    openAct = new QAction(openIcon, tr("&Открыть..."), this);
    openAct->setShortcut(QKeySequence::Open);
    openAct->setStatusTip(tr("Открыть существующий XML-файл"));
    connect(openAct, &QAction::triggered, this, &XmlEditor::openFile);
    fileMenu->addAction(openAct);

    openXMLDeviceDescriptionAct = new QAction(openIcon, tr("Открыть XML Device Description (Crevis)"), this);
    openXMLDeviceDescriptionAct->setStatusTip(tr("Открыть XML Device Description (Crevis)"));
    connect(openXMLDeviceDescriptionAct, &QAction::triggered, this, &XmlEditor::onLoadButtonClicked);
    fileMenu->addAction(openXMLDeviceDescriptionAct);

    newXMLDeviceDescriptionAct = new QAction(openIcon, tr("Создать XML Device Description"), this);
    newXMLDeviceDescriptionAct->setStatusTip(tr("Создать XML Device Description (Crevis)"));
    connect(newXMLDeviceDescriptionAct, &QAction::triggered, this, &XmlEditor::onNewClicked);
    fileMenu->addAction(newXMLDeviceDescriptionAct);

    fileMenu->addSeparator();

    const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(":/images/save.png"));
    saveAct = new QAction(saveIcon, tr("&Сохранить"), this);
    saveAct->setShortcut(QKeySequence::Save);
    saveAct->setStatusTip(tr("Сохранить текущий файл"));
    connect(saveAct, &QAction::triggered, this, &XmlEditor::saveFile_);
    fileMenu->addAction(saveAct);

    const QIcon saveAsIcon = QIcon::fromTheme("document-saveas", QIcon(":/images/saveas.png"));
    saveAsAct = new QAction(saveAsIcon, tr("Сохранить как..."), this);
    saveAsAct->setShortcut(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Сохраните файл под новым именем"));
    connect(saveAsAct, &QAction::triggered, this, &XmlEditor::saveAsFile_);
    fileMenu->addAction(saveAsAct);

    fileMenu->addSeparator();

    const QIcon closeFileIcon = QIcon::fromTheme("document-closefile", QIcon(":/images/closefile.png"));
    closeAct = new QAction(closeFileIcon, tr("&Закрыть файл"), this);
    closeAct->setShortcut(QKeySequence::Close);
    closeAct->setStatusTip(tr("&Закрыть XML файл"));
    connect(closeAct, &QAction::triggered, this, &XmlEditor::closeFile);
    fileMenu->addAction(closeAct);

    fileMenu->addSeparator();

    exitAct = new QAction(tr("&Выход"), this);
    exitAct->setShortcut(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Выход из приложения"));
    connect(exitAct, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(exitAct);

    // Edit menu
    const QIcon addElementIcon = QIcon::fromTheme("edit-add", QIcon(":/images/addelement.png"));
    addNodeAct = new QAction(addElementIcon, tr("&Добавить элемент"), this);
    addNodeAct->setShortcut(Qt::CTRL | Qt::Key_N);
    addNodeAct->setStatusTip(tr("Добавить новый элемент"));
    connect(addNodeAct, &QAction::triggered, this, &XmlEditor::addNode);
    editMenu->addAction(addNodeAct);

    const QIcon editElementIcon = QIcon::fromTheme("edit-edit", QIcon(":/images/editelement.png"));
    editNodeAct = new QAction(editElementIcon, tr("&Редактировать элемент"), this);
    editNodeAct->setShortcut(Qt::CTRL | Qt::Key_E);
    editNodeAct->setStatusTip(tr("Редактировать выбранный элемент"));
    connect(editNodeAct, &QAction::triggered, this, &XmlEditor::editNode);
    editMenu->addAction(editNodeAct);

    const QIcon deleteElementIcon = QIcon::fromTheme("edit-edit", QIcon(":/images/deleteelement.png"));
    removeNodeAct = new QAction(deleteElementIcon, tr("&Удалить элемент"), this);
    removeNodeAct->setShortcut(Qt::Key_Delete);
    removeNodeAct->setStatusTip(tr("Удалить выбранный элемент"));
    connect(removeNodeAct, &QAction::triggered, this, &XmlEditor::removeNode);
    editMenu->addAction(removeNodeAct);

    // View menu
    updateTreeAct = new QAction(tr("Обновить XML дерево"), this);
    updateTreeAct->setStatusTip(tr("Обновить вид дерева"));
    connect(updateTreeAct, &QAction::triggered, this, &XmlEditor::updateTreeView);
    viewMenu->addAction(updateTreeAct);

    updateXmlAct = new QAction(tr("Обновить XML вид"), this);
    updateXmlAct->setStatusTip(tr("Обновить вид текста XML"));
    connect(updateXmlAct, &QAction::triggered, this, &XmlEditor::updateXmlView_);
    viewMenu->addAction(updateXmlAct);

    const QIcon updateAllIcon = QIcon::fromTheme("update-updateall", QIcon(":/images/updateall.png"));
    updateAllAct = new QAction(updateAllIcon, tr("Обновить все"), this);
    updateAllAct->setStatusTip(tr("Обновить все"));
    connect(updateAllAct, &QAction::triggered, this, &XmlEditor::updateDisplay);
    viewMenu->addAction(updateAllAct);    

    const QIcon checkAllIcon = QIcon::fromTheme("update-updateall", QIcon(":/images/checkall.png"));
    fullValidationAct = new QAction(checkAllIcon, tr("&Полная проверка XML"), this);
    fullValidationAct->setShortcut(Qt::CTRL | Qt::Key_V);
    fullValidationAct->setStatusTip(tr("Выполнить полную проверку XML"));
    connect(fullValidationAct, &QAction::triggered, this, &XmlEditor::fullXmlValidation);
    validationMenu->addAction(fullValidationAct);

    validationMenu->addSeparator();

    wellFormedAct = new QAction(tr("Проверка правильности XML"), this);
    wellFormedAct->setStatusTip(tr("Проверка правильности формата XML"));
    connect(wellFormedAct, &QAction::triggered, this, &XmlEditor::validateWellFormed);
    validationMenu->addAction(wellFormedAct);

    syntaxAct = new QAction(tr("Проверка синтаксиса XML"), this);
    syntaxAct->setStatusTip(tr("Проверка синтаксиса XML"));
    connect(syntaxAct, &QAction::triggered, this, &XmlEditor::validateSyntax);
    validationMenu->addAction(syntaxAct);

    elementNamesAct = new QAction(tr("Проверка имен элементов XML"), this);
    elementNamesAct->setStatusTip(tr("Проверка имен элементов XML"));
    connect(elementNamesAct, &QAction::triggered, this, &XmlEditor::validateElementNames);
    validationMenu->addAction(elementNamesAct);

    attributesAct = new QAction(tr("Проверка атрибутов XML"), this);
    attributesAct->setStatusTip(tr("Проверка атрибутов XML"));
    connect(attributesAct, &QAction::triggered, this, &XmlEditor::validateAttributes);
    validationMenu->addAction(attributesAct);

    characterDataAct = new QAction(tr("Проверить символов"), this);
    characterDataAct->setStatusTip(tr("Проверить символов"));
    connect(characterDataAct, &QAction::triggered, this, &XmlEditor::validateCharacterData);
    validationMenu->addAction(characterDataAct);

    validationMenu->addSeparator();

    validationReportAct = new QAction(tr("Отчет о проверке"), this);
    validationReportAct->setStatusTip(tr("Показать отчет о проверке"));
    connect(validationReportAct, &QAction::triggered, this, &XmlEditor::showValidationReport);
    validationMenu->addAction(validationReportAct);

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setShortcut(Qt::Key_F1);
    connect(aboutAct, &QAction::triggered, this, &XmlEditor::about);

    aboutQtAct = new QAction(tr("About &Qt"), this);
    connect(aboutQtAct, &QAction::triggered, qApp, &QApplication::aboutQt);

    //QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}

void XmlEditor::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->setObjectName(tr("FileToolBar"));
    fileToolBar->addAction(newAct);
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);
    fileToolBar->addAction(saveAsAct);
    fileToolBar->addAction(closeAct);

    editToolBar = addToolBar(tr("Edit"));
    editToolBar->setObjectName(tr("EditToolBar"));
    editToolBar->addAction(addNodeAct);
    editToolBar->addAction(editNodeAct);
    editToolBar->addAction(removeNodeAct);

    updateToolBar = addToolBar(tr("Update"));
    updateToolBar->setObjectName(tr("UpdateToolBar"));
    //updateToolBar->addAction(updateTreeAct);
    //updateToolBar->addAction(updateXmlAct);
    updateToolBar->addAction(updateAllAct);

    validationToolBar = addToolBar(tr("Validation"));
    validationToolBar->setObjectName(tr("ValidationToolBar"));
    validationToolBar->addAction(fullValidationAct);
    //validationToolBar->addAction(wellFormedAct);
    //validationToolBar->addAction(syntaxAct);
}

void XmlEditor::createStatusBar()
{
    statusBar()->showMessage(tr("Готово"));
}

void XmlEditor::addNode() {
    if(isOpenXMLDeviceDescription == true)
    {
        QMessageBox::warning(this, tr("XML редактор"), tr("Для этого откройте как обычный XML-документ"));
        return;
    }
    xmlTree->requestAddNode(this);
}

void XmlEditor::editNode() {
    if(isOpenXMLDeviceDescription == true)
    {
        QMessageBox::warning(this, tr("XML редактор"), tr("Для этого откройте как обычный XML-документ"));
        return;
    }
    xmlTree->requestEditNode(this);    
    updateTreeView();
}

void XmlEditor::removeNode() {
    if(isOpenXMLDeviceDescription == true)
    {
        QMessageBox::warning(this, tr("XML редактор"), tr("Для этого откройте как обычный XML-документ"));
        return;
    }
    xmlTree->requestRemoveNode(this);
}

void XmlEditor::updateDisplay() {
    if(isOpenXMLDeviceDescription == true)
    {
        QMessageBox::warning(this, tr("XML редактор"), tr("Для этого откройте как обычный XML-документ"));
        return;
    }
    updateTreeView();
    updateXmlView_();
}

void XmlEditor::updateTreeView() {
    if(isOpenXMLDeviceDescription == true)
    {
        QMessageBox::warning(this, tr("XML редактор"), tr("Для этого откройте как обычный XML-документ"));
        return;
    }
    xmlTree->refresh();
    statusBar()->showMessage(tr("XML-дерево обновлено"));
}

void XmlEditor::updateXmlView_() {
    if(isOpenXMLDeviceDescription == true)
    {
        QMessageBox::warning(this, tr("XML редактор"), tr("Для этого откройте как обычный XML-документ"));
        return;
    }
    xmlTextEdit->setPlainText(xmlTree->getXmlDocument().toString(4));   // отображаем в текстовом поле XML с отступами
    statusBar()->showMessage(tr("Представление XML обновлено"));
}

void XmlEditor::updateXmlView(const QDomDocument & newDoc) {
    xmlTextEdit->setPlainText(newDoc.toString(4));   // отображаем в текстовом поле XML с отступами
    statusBar()->showMessage(tr("Представление XML обновлено"));
}


void XmlEditor::newFile() {
    //QDomDocument newDoc;
    QDomElement root = newDoc.createElement("root");
    root.setAttribute("version", "1.0");
    newDoc.appendChild(root);
    QDomElement child = newDoc.createElement("element");
    child.setAttribute("id", "1");
    QDomText text = newDoc.createTextNode("Sample text content");
    child.appendChild(text);
    root.appendChild(child);

    if(isOpenXML == false)
        createCentralWidget();

    xmlTree->setXmlDocument(newDoc);
    //xmlTextEdit->clear(); //
    updateXmlView_();

    currentFile.clear();
    isModified = false;
    setCurrentFile(QString());
    statusBar()->showMessage("New document created");

}

void XmlEditor::openFile() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Открыть XML-файл"), tr(""), tr("XML Files (*.xml);;All Files (*)"));
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("XML редактор"), QString(tr("Невозможно прочитать файл %1:\n%2.")).arg(fileName, file.errorString()));
        return;
    }

    if(isOpenXML == false)
        createCentralWidget();

    QString content = file.readAll();
    file.close();

    // Предварительная проверка WellFormed
    QString errorMsg;
    int errorLine, errorColumn;
    QXmlStreamReader reader(content);
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.hasError()) {
            errorMsg = reader.errorString();
            errorLine = reader.lineNumber();
            errorColumn = reader.columnNumber();
            QMessageBox::warning(this, tr("XML редактор"), QString(tr("Ошибка анализа в строке %1, column %2:\n%3")).arg(errorLine).arg(errorColumn).arg(errorMsg));
            return;
        }
    }

    //QDomDocument newDoc;
    if (!newDoc.setContent(content)) { // setContent тоже проверяет WellFormed
        QMessageBox::warning(this, tr("XML редактор"), tr("Неправильно сформированное содержимое XML"));
        return;
    }

    xmlTree->setXmlDocument(newDoc);
    updateXmlView_(); // XmlEditor обновляет свой редактор
    //updateXmlView(newDoc); // XmlEditor обновляет свой редактор

    currentFile = fileName;
    isModified = false;
    isOpenXML = true;
    setCurrentFile(fileName);
    statusBar()->showMessage(QString(tr("Файл загружен: %1")).arg(fileName), 2000);
}

void XmlEditor::saveFile(const QString &fileName) {
    // if (currentFile.isEmpty()) {
    //     saveAsFile();
    // } else {
    QDomDocument docToSave;


        if(hasUnsavedChanges == true)
        {
            bool success = saveXMLFile(currentFile, originalDomDoc, parsedTypes, parsedDeviceInfo);
            if (success) {
                hasUnsavedChanges = false;
                isModified = false;
                setWindowTitle(tr("XmlEditor - %1").arg(QFileInfo(currentFile).fileName()));
            }
            docToSave = originalDomDoc;
        }
        else
            docToSave = xmlTree->getXmlDocument();

        QFile file(fileName);
        if (!file.open(QFile::WriteOnly | QFile::Text)) {
            QMessageBox::warning(this, tr("XML редактор"), QString(tr("Невозможно записать файл %1:\n%2.")).arg(currentFile, file.errorString()));
            return;
        }
        QTextStream out(&file);
        docToSave.save(out, 4);
        file.close();

        isModified = false;
        statusBar()->showMessage(QString(tr("File saved: %1")).arg(currentFile), 2000);
    //}
}

void XmlEditor::saveFile_()
{
    if (currentFile.isEmpty()) {
        saveAsFile_();
    } else {
        saveFile(currentFile);
    }
}

void XmlEditor::saveAsFile_()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Сохранить XML файл"), currentFile, tr("XML Files (*.xml);;All Files (*)"));

    if (fileName.isEmpty())
        return;

    saveFile(fileName);
    setCurrentFile(fileName);
}

void XmlEditor::onNodeOperationFinished() {
    updateXmlView_(); // XmlEditor обновляет свой редактор
    isModified = true; // XmlEditor управляет флагом
    statusBar()->showMessage(tr("Операция завершена"));
}

void XmlEditor::onOperationFailed(const QString &error) {
    QMessageBox::warning(this, tr("XML редактор"), error);
    statusBar()->showMessage(tr("Операция провалилась"));
}

void XmlEditor::onAttributeChangedFromPopup(const QModelIndex &paramIndex, const QString &attrName, const QString &newValue)
{
    parametersModel->updateAttribute(paramIndex, attrName, newValue);
    hasUnsavedChanges = true;
}

void XmlEditor::onParametersContextMenuRequested(const QPoint &pos)
{
    QModelIndex index = parametersView->indexAt(pos);
    if (!index.isValid()) return;
//    QMenu menu(this);
//    QAction *editAttrs = menu.addAction("Edit Attributes");
//    QAction *selected = menu.exec(parametersView->mapToGlobal(pos));
//    if (selected == editAttrs) {
//        DeviceParameter *param = parametersModel->getParameter(index);
//        if (param) {
//            attributesPopup->showForParameter(*param, parametersView, index);
//        }
//    }

    //QMenu menu(this);
    //QAction *editAttrs = menu.addAction(tr("Показать (Редактировать) Атрибуты"));
    //QAction *selected = menu.exec(parametersView->mapToGlobal(pos));
    //if (selected == editAttrs) {
        DeviceParameter *param = parametersModel->getParameter(index);
        if (param) {
            attributesPopup->showForParameter(*param, parametersView, index);
        }
    //}
}

void XmlEditor::markAsChangedForModel(const QModelIndex &, const QModelIndex &, const QVector<int> &)
{
    hasUnsavedChanges = true;
}

void XmlEditor::onParametersContextMenuRequested2(const QPoint &pos)
{
    qDebug() << "Context menu requested at" << pos; // Отладка: проверим, вызывается ли слот

    // Получаем индекс под курсором
    QModelIndex index = parametersView->indexAt(pos);
    if (!index.isValid()) return;

    // Проверяем, принадлежит ли параметр HostParameterSet
    bool isHostParam = index.model()->data(index.siblingAtColumn(1), IsHostParameterRole).toBool(); // Используем IsHostParameterRole
    if (!isHostParam) {
        // Если это не параметр HostParameterSet, не показываем меню
        return;
    }

    // Определяем индекс коннектора
    int connectorIndex = index.model()->data(index.siblingAtColumn(1), ConnectorIndexRole).toInt();
    if (connectorIndex < 0 || connectorIndex >= parsedDeviceInfo.connectors.size()) {
        qWarning() << "Invalid connector index for parameter at row" << index.row();
        return;
    }

    // Сохраняем индекс строки для использования в действиях меню
    currentParameterIndex = index;

    QAction *selected = menu->exec(parametersView->mapToGlobal(pos));
    if (selected == editAttrs) {
        DeviceParameter *param = parametersModel->getParameter(index);
        if (param) {
            attributesPopup->showForParameter(*param, parametersView, index);
        }
    }
    ///////

    menu->exec(parametersView->mapToGlobal(pos));
}

void XmlEditor::onDeleteParameterTriggered()
{
    if (!currentParameterIndex.isValid()) return;

    bool isHostParam = currentParameterIndex.model()->data(currentParameterIndex.siblingAtColumn(1), IsHostParameterRole).toBool();
    if (!isHostParam) return; // Проверка безопасности

    int connectorIndex = currentParameterIndex.model()->data(currentParameterIndex.siblingAtColumn(1), ConnectorIndexRole).toInt();
    if (connectorIndex < 0 || connectorIndex >= parsedDeviceInfo.connectors.size()) {
        qWarning() << "Invalid connector index for parameter deletion.";
        return;
    }

    int paramRowInModel = currentParameterIndex.row();
    // Нужно найти индекс параметра внутри HostParameterSet
    // Модель ParametersModel хранит все параметры (Device и Host) в плоском списке m_allParams
    // Используем тот же подход, что и в ParametersModel::setParameter
    int paramIndexInConnector = -1;
    int cumulativeIndex = 0;
    for (int i = 0; i < parsedDeviceInfo.connectors.size(); ++i) {
        if (i == connectorIndex) {
            // Нашли нужный коннектор
            int hostParamCount = parsedDeviceInfo.connectors[i].parameters.size();
            if (paramRowInModel >= cumulativeIndex && paramRowInModel < cumulativeIndex + hostParamCount) {
                paramIndexInConnector = paramRowInModel - cumulativeIndex;
                break;
            }
        }
        // Увеличиваем кумулятивный индекс на количество параметров в этом коннекторе
        cumulativeIndex += parsedDeviceInfo.connectors[i].parameters.size();
    }

    if (paramIndexInConnector < 0 || paramIndexInConnector >= parsedDeviceInfo.connectors[connectorIndex].parameters.size()) {
        qWarning() << "Invalid parameter index in connector for deletion.";
        return;
    }

    // Удаляем параметр из структуры данных
    parsedDeviceInfo.connectors[connectorIndex].parameters.removeAt(paramIndexInConnector);
    parsedDeviceInfo.connectors[connectorIndex].isModified = true;
    parsedDeviceInfo.isModified = true; // Устройство тоже изменено

    // Перезагружаем модели, чтобы отразить изменения
    // Это может быть неэффективно для больших файлов, но надежно
    parametersModel->setData(&parsedDeviceInfo);

    // Не забываем обновить и другие модели, если HostParameterSet влияет на них напрямую
    // В данном случае, только параметры затронуты, так что перезагрузка ParametersModel достаточна для отображения
    // Однако, чтобы убедиться, что изменения сохранятся, перезагрузим все

    //typesModel->setData(&parsedDeviceInfo.connectors[connectorIndex].parameters); // Это не сработает, нужно перезагрузить все

    // Лучше перезагрузить все модели, связанные с параметрами
    // setData в ParametersModel уже вызывает reset, что обновит представление
    hasUnsavedChanges = true;
}

void XmlEditor::onAddDiscreteInputChannelTriggered()
{

    if (!currentParameterIndex.isValid()) return;
    // Определяем индекс коннектора из текущего индекса параметра
    int connectorIndex = currentParameterIndex.model()->data(currentParameterIndex.siblingAtColumn(1), ConnectorIndexRole).toInt();
    if (connectorIndex < 0) return; // Проверка безопасности

    // Вызываем метод модели для добавления параметра
    if (parametersModel) {

        // Вызываем метод модели, передав индекс коннектора, тип и базовые параметры
        addParameterToHostParameterSet(
                    connectorIndex,
                    NewParameterType::InputDiscreteChannel, // Используем enum из модели
                    "1000", // Базовый ID для входов
                    "IN", // Базовое имя
                    //"Input Channel", // Базовое имя
                    //"UDINT", // Базовый тип
                    "localtype:TBit1Byte" // Базовый тип
                    );
        hasUnsavedChanges = true;
    }
}

void XmlEditor::onAddDiscreteOutputChannelTriggered()
{

    if (!currentParameterIndex.isValid()) return;
    // Определяем индекс коннектора из текущего индекса параметра
    int connectorIndex = currentParameterIndex.model()->data(currentParameterIndex.siblingAtColumn(1), ConnectorIndexRole).toInt();
    if (connectorIndex < 0) return; // Проверка безопасности

    // Вызываем метод модели для добавления параметра
    if (parametersModel) {
        // Вызываем метод модели, передав индекс коннектора, тип и базовые параметры
        addParameterToHostParameterSet(
                    connectorIndex,
                    NewParameterType::OutputDiscreteChannel, // Используем enum из модели
                    "2000", // Базовый ID для входов
                    "OUT", // Базовое имя
                    //"Input Channel", // Базовое имя
                    //"UDINT", // Базовый тип
                    "localtype:TBit1Byte" // Базовый тип
                    );
        hasUnsavedChanges = true;
    }
}

void XmlEditor::onAddAnalogInputChannelTriggered()
{

    if (!currentParameterIndex.isValid()) return;
    // Определяем индекс коннектора из текущего индекса параметра
    int connectorIndex = currentParameterIndex.model()->data(currentParameterIndex.siblingAtColumn(1), ConnectorIndexRole).toInt();
    if (connectorIndex < 0) return; // Проверка безопасности

    // Вызываем метод модели для добавления параметра
    if (parametersModel) {

        // Вызываем метод модели, передав индекс коннектора, тип и базовые параметры
        addParameterToHostParameterSet(
                    connectorIndex,
                    NewParameterType::InputDiscreteChannel, // Используем enum из модели
                    "3000", // Базовый ID для входов
                    "AI", // Базовое имя
                    //"Input Channel", // Базовое имя
                    //"UDINT", // Базовый тип
                    "std:WORD" // Базовый тип
                    );
        hasUnsavedChanges = true;
    }
}

void XmlEditor::onAddAnalogOutputChannelTriggered()
{

    if (!currentParameterIndex.isValid()) return;
    // Определяем индекс коннектора из текущего индекса параметра
    int connectorIndex = currentParameterIndex.model()->data(currentParameterIndex.siblingAtColumn(1), ConnectorIndexRole).toInt();
    if (connectorIndex < 0) return; // Проверка безопасности

    // Вызываем метод модели для добавления параметра
    if (parametersModel) {
        // Вызываем метод модели, передав индекс коннектора, тип и базовые параметры
        addParameterToHostParameterSet(
                    connectorIndex,
                    NewParameterType::OutputDiscreteChannel, // Используем enum из модели
                    "4000", // Базовый ID для входов
                    "AO", // Базовое имя
                    //"Input Channel", // Базовое имя
                    //"UDINT", // Базовый тип
                    "std:WORD" // Базовый тип
                    );
        hasUnsavedChanges = true;
    }
}

void XmlEditor::onAddConfigParameterTriggered()
{

    if (!currentParameterIndex.isValid()) return;
    // Определяем индекс коннектора из текущего индекса параметра
    int connectorIndex = currentParameterIndex.model()->data(currentParameterIndex.siblingAtColumn(1), ConnectorIndexRole).toInt();
    if (connectorIndex < 0) return; // Проверка безопасности

    // Вызываем метод модели для добавления параметра
    if (parametersModel) {
        // Вызываем метод модели, передав индекс коннектора, тип и базовые параметры
        addParameterToHostParameterSet(
                    connectorIndex,
                    NewParameterType::ConfigParameter, // Используем enum из модели
                    "400000", // Базовый ID для входов
                    "Config Parameter", // Базовое имя
                    //"Input Channel", // Базовое имя
                    //"UDINT", // Базовый тип
                    "std:BYTE" // Базовый тип
                    );
        hasUnsavedChanges = true;
    }
}

void XmlEditor::onNewClicked()
{

    NewDeviceDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        DeviceType selectedType = dialog.getSelectedType();

        // Сбрасываем текущее состояние
        currentFileName.clear();
        //hasUnsavedChanges = false;
        hasUnsavedChanges = true;
        originalDomDoc = QDomDocument();
        isNew = true;

        if(isOpenXML)
        {
            closeFile();
            closeCentralWidget();
        }
        createXMLDeviceDescriptionWidget();

        isOpenXMLDeviceDescription = true;

        clearModels();

        //clearModels();

        // Создаем новый документ на основе выбранного типа
        originalDomDoc = createNewDeviceDescriptionDocumentBasedOnType(selectedType);

//            if(selectedType == DeviceType::DiscreteOutput)
//                addInputAction->setVisible(false);

        // Разбираем новый документ, чтобы заполнить структуры
        auto parseResultFromDoc = parseFromQDomDoc(originalDomDoc);

        if (!parseResultFromDoc.first.isNull()) {
            const auto &types = parseResultFromDoc.second.first;
            const auto &devInfo = parseResultFromDoc.second.second;

            parsedTypes = types;
            parsedDeviceInfo = devInfo;

            typesModel->setData(&parsedTypes);
            parametersModel->setData(&parsedDeviceInfo);
            connectorsModel->setData(&parsedDeviceInfo);
            deviceInfoModel->setData(&parsedDeviceInfo);
            filesModel->setData(&parsedDeviceInfo);
            deviceIdentificationModel->setData(&parsedDeviceInfo);

            setWindowTitle("Device Description Viewer - [New File]");

            if(selectedType == DeviceType::AnalogInput)
            {
                addDiscreteInputParameterAction->setVisible(false);
                addDiscreteOutputParameterAction->setVisible(false);
                addAnalogInputParameterAction->setVisible(true);
                addAnalogOutputParameterAction->setVisible(false);
            }
            else if(selectedType == DeviceType::AnalogOutput)
            {
                addDiscreteInputParameterAction->setVisible(false);
                addDiscreteOutputParameterAction->setVisible(false);
                addAnalogInputParameterAction->setVisible(false);
                addAnalogOutputParameterAction->setVisible(true);
            }
            else if(selectedType == DeviceType::DiscreteIO || selectedType == DeviceType::DiscreteInput || selectedType == DeviceType::DiscreteOutput)
            {
                addDiscreteInputParameterAction->setVisible(true);
                addDiscreteOutputParameterAction->setVisible(true);
                addAnalogInputParameterAction->setVisible(false);
                addAnalogOutputParameterAction->setVisible(false);
            }
            else if(selectedType == DeviceType::Power)
            {
                addDiscreteInputParameterAction->setVisible(false);
                addDiscreteOutputParameterAction->setVisible(false);
                addAnalogInputParameterAction->setVisible(false);
                addAnalogOutputParameterAction->setVisible(false);
                addConfigParameterAction->setVisible(false);
            }

        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось обработать вновь созданный файл.");
        }
    }
    // Если пользователь нажал Cancel, ничего не делаем
}

void XmlEditor::closeEvent(QCloseEvent *event)
{
    if (hasUnsavedChanges) {
        QMessageBox msgBox;
        msgBox.setText(tr("Документ был изменен.\nВы хотите сохранить изменения?"));

        // Добавляем пользовательские кнопки
        QPushButton* btnOk = msgBox.addButton(tr("Сохранить"), QMessageBox::YesRole);
        QPushButton* btnReset = msgBox.addButton(tr("Не Сохранять"), QMessageBox::ResetRole);
        QPushButton* btnCancel = msgBox.addButton("Отменить", QMessageBox::NoRole);
        msgBox.exec();

        if (msgBox.clickedButton() == btnOk) {
            if (!saveCurrentFile()) {
                event->ignore();
                return;
            }
        }else if (msgBox.clickedButton() == btnReset) {
            return;
        }
        else if (msgBox.clickedButton() == btnCancel) {
            event->ignore();
            return;
        }

//        QMessageBox::StandardButton ret = QMessageBox::question(this, tr("Несохраненные изменения"),
//            tr("Документ был изменен.\nХотите сохранить изменения?"),
//            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
//        if (ret == QMessageBox::Save) {
//            if (!saveCurrentFile()) {
//                event->ignore();
//                return;
//            }
//        } else if (ret == QMessageBox::Cancel) {
//            event->ignore();
//            return;
//        }
    }
    QMainWindow::closeEvent(event);
}

void XmlEditor::addParameterToHostParameterSet(int connectorIndex, NewParameterType paramType, const QString &baseId, const QString &baseName, const QString &type)
{

    QModelIndex index = currentParameterIndex;
    if (!index.isValid()) return;

    bool isHostParam = index.model()->data(index.siblingAtColumn(1), IsHostParameterRole).toBool();
    if (!isHostParam) return; // Проверка безопасности

    connectorIndex = index.model()->data(index.siblingAtColumn(1), ConnectorIndexRole).toInt();
    if (connectorIndex < 0 || connectorIndex >= parsedDeviceInfo.connectors.size()) {
        qWarning() << "Invalid connector index for parameter addition.";
        return;
    }

    // Найдем следующий свободный ID
    QString newId = findNextAvailableId(parsedDeviceInfo.connectors[connectorIndex].parameters, baseId);

    DeviceParameter newParam;
    newParam.id = newId;
    //newParam.name = baseName + " " + newId.right(3); // Например, "Input Channel 1000"
    newParam.name = baseName + newId.right(1); // Например, "Input Channel 1000"
    newParam.type = type;

    if(newId.toLongLong() >= 400000)
        newParam.description = "Description";
    else
        newParam.description = "";

    newParam.defaultValue = "0";

    // --- Добавляем атрибуты для параметра ---
    switch (paramType) {
        case NewParameterType::InputDiscreteChannel:
            newParam.attributesMap["channel"] = "input";
            newParam.attributesMap["download"] = "true";
            newParam.attributesMap["functional"] = "false";
            newParam.attributesMap["offlineaccess"] = "read";
            newParam.attributesMap["onlineaccess"] = "read";
            break;
        case NewParameterType::OutputDiscreteChannel:
            newParam.attributesMap["channel"] = "output";
            newParam.attributesMap["download"] = "true";
            newParam.attributesMap["functional"] = "false";
            newParam.attributesMap["offlineaccess"] = "readwrite";
            newParam.attributesMap["onlineaccess"] = "readwrite";
            break;
        case NewParameterType::InputAnalogChannel:
            newParam.attributesMap["channel"] = "input";
            newParam.attributesMap["download"] = "true";
            newParam.attributesMap["functional"] = "false";
            newParam.attributesMap["offlineaccess"] = "read";
            newParam.attributesMap["onlineaccess"] = "read";
            break;
        case NewParameterType::OutputAnalogChannel:
            newParam.attributesMap["channel"] = "output";
            newParam.attributesMap["download"] = "true";
            newParam.attributesMap["functional"] = "false";
            newParam.attributesMap["offlineaccess"] = "readwrite";
            newParam.attributesMap["onlineaccess"] = "readwrite";
            break;
        case NewParameterType::ConfigParameter:
            newParam.attributesMap["channel"] = "none";
            newParam.attributesMap["download"] = "true";
            newParam.attributesMap["functional"] = "false";
            newParam.attributesMap["offlineaccess"] = "readwrite";
            newParam.attributesMap["onlineaccess"] = "none";
            break;
    }

    // --- Добавляем атрибуты для Name и Description ---
    QString processedBaseName = newParam.name;//baseName; // Создаём копию
    processedBaseName.replace(" ", ""); // Изменяем копию

    if(newId.toLongLong() < 400000)
        newParam.nameAttributesMap["name"] = "local:" + processedBaseName.toLower(); // Используем изменённую копию
    else
        newParam.nameAttributesMap["name"] = "local:Id" + newParam.id; // Используем изменённую копию

    if(newId.toULongLong() >= 400000)
        newParam.descriptionAttributesMap["desc"] = "local:Id" + newParam.id + ".Desc";//"SomeDescriptionAttribute"; // Если нужно, добавьте

    newParam.isFromHostParameterSet = true;
    newParam.connectorIndex = connectorIndex;
    newParam.originalIndex = parsedDeviceInfo.connectors[connectorIndex].parameters.size(); // Индекс в списке
    newParam.isModified = true; // Новый параметр считается изменённым

    // Добавим параметр в структуру данных
    parsedDeviceInfo.connectors[connectorIndex].parameters.append(newParam);
    parsedDeviceInfo.connectors[connectorIndex].isModified = true; // Коннектор изменился
    parsedDeviceInfo.isModified = true; // Устройство изменилось

    // --- Сортировка HostParameterSet ---
    // Сортируем список параметров коннектора по ID
    //auto &params = m_deviceInfo->connectors[connectorIndex].parameters;
    auto &params = parsedDeviceInfo.connectors[connectorIndex].parameters;
    std::sort(params.begin(), params.end(), [](const DeviceParameter &a, const DeviceParameter &b) {
        return a.id < b.id;
    });
    // После сортировки нужно обновить originalIndex у каждого параметра в списке
    for (int i = 0; i < params.size(); ++i) {
        params[i].originalIndex = i;
    }
    // --- /Сортировка HostParameterSet ---

    // Перезагрузим модель, чтобы отразить изменения
    parametersModel->setData(&parsedDeviceInfo);

    // Устанавливаем флаг несохранённых изменений (если вызывается из DeviceDescriptionViewer)
    // hasUnsavedChanges = true; //
}

QString XmlEditor::findNextAvailableId(const QList<DeviceParameter> &paramList, const QString &baseId)
{
    bool ok;
    int baseNum = baseId.toInt(&ok);
    if (!ok) {
        qWarning() << "Base ID is not numeric, cannot increment:" << baseId;
        return baseId; // Вернуть как есть, если не число
    }

    int nextIdNum = baseNum;
    QSet<QString> existingIds;
    for (const auto &param : paramList) {
        bool idOk;
        int idNum = param.id.toInt(&idOk);
        if (idOk) {
            existingIds.insert(QString::number(idNum));
        }
    }

    while (existingIds.contains(QString::number(nextIdNum))) {
        nextIdNum++;
    }
    return QString::number(nextIdNum);
}

// void XmlEditor::onTreeItemDoubleClicked(XmlTreeItem *item) {
//     Q_UNUSED(item)
//     editNode(); // Запрашиваем редактирование
// }

bool XmlEditor::maybeSave()
{
    if (isModified) {
        QMessageBox msgBox;
        msgBox.setText(tr("Документ был изменен.\nВы хотите сохранить изменения?"));

        // Добавляем пользовательские кнопки
        QPushButton* btnOk = msgBox.addButton(tr("Сохранить"), QMessageBox::YesRole);
        QPushButton* btnReset = msgBox.addButton(tr("Не Сохранять"), QMessageBox::ResetRole);
        QPushButton* btnCancel = msgBox.addButton("Отменить", QMessageBox::NoRole);
        msgBox.exec();

        if (msgBox.clickedButton() == btnOk) {            
            saveFile_();
            return !isModified; // Return true if saved successfully
        }else if (msgBox.clickedButton() == btnReset) {
            return true;
        }
        else if (msgBox.clickedButton() == btnCancel) {
            return false;
        }

    }
    return true;
}

void XmlEditor::closeFile()
{
    if(isOpenXMLDeviceDescription == true)
    {
        clearModels();

        return;
    }

    if (maybeSave()) {
        xmlTree->xmlDocument.clear();
        xmlTree->clear();
        xmlTextEdit->clear();
        currentFile.clear();
        isModified = false;
        setCurrentFile(QString());
        statusBar()->showMessage(tr("Файл закрыт"), 2000);
    }
}

void XmlEditor::setCurrentFile(const QString &fileName)
{
    currentFile = fileName;
    QString shownName = currentFile;
    if (currentFile.isEmpty())
        shownName = "untitled.xml";

    setWindowTitle(QString(tr("%1[*] - XML редактор")).arg(shownName));
    setWindowModified(isModified);
}

// QString XmlEditor::domToString()
// {
//     QString xmlString;
//     QTextStream stream(&xmlString);
//     domDocument.save(stream, 4);
//     return xmlString;
// }
void XmlEditor::fullXmlValidation()
{
    performValidation(XmlValidator::FullValidation);
}

void XmlEditor::validateWellFormed()
{
    performValidation(XmlValidator::WellFormedCheck);
}

void XmlEditor::validateSyntax()
{
    performValidation(XmlValidator::SyntaxCheck);
}

void XmlEditor::validateElementNames()
{
    performValidation(XmlValidator::ElementNameCheck);
}

void XmlEditor::validateAttributes()
{
    performValidation(XmlValidator::AttributeCheck);
}

void XmlEditor::validateCharacterData()
{
    performValidation(XmlValidator::CharacterDataCheck);
}

void XmlEditor::showValidationReport()
{
    QString xmlContent;

    QTextEdit   *xmlTextEditTemp = new QTextEdit();
    xmlTextEditTemp->setPlainText(newDoc.toString(4));
    xmlContent = xmlTextEditTemp->toPlainText();

    if(xmlContent.isEmpty()) {
        QMessageBox::warning(this, tr("XML редактор"), QString(tr("XML файл не открыт")));
        return;
    }

    XmlValidator::ValidationResult result = validator->validateXml(xmlContent);

    QString report = QString(tr("Итог проверки:\n"));
    report += QString(tr("Ошибки: %1\n")).arg(result.errorCount);
    report += QString(tr("Предупреждения: %1\n")).arg(result.warningCount);
    report += QString(tr("Статус: %1\n")).arg(result.isValid ? tr("Валидный") : tr("Не Валидный"));

    QMessageBox::information(this, tr("Отчет о проверке"), report);
}

void XmlEditor::performValidation(XmlValidator::ValidationTypes types)
{
    QString xmlContent;

    QTextEdit   *xmlTextEditTemp = new QTextEdit();;
    xmlTextEditTemp->setPlainText(newDoc.toString(4));
    xmlContent = xmlTextEditTemp->toPlainText();

    if(xmlContent.isEmpty()) {
        QMessageBox::warning(this, tr("XML редактор"), QString(tr("XML файл не открыт")));
        return;
    }

    XmlValidator::ValidationResult result = validator->validateXml(xmlContent, types);
    displayValidationResults(result);
}

void XmlEditor::displayValidationResults(const XmlValidator::ValidationResult &result)
{
/*
    validationTable->setRowCount(0);

    for (const auto &error : result.errors) {
        int row = validationTable->rowCount();
        validationTable->insertRow(row);

        QTableWidgetItem *severityItem = new QTableWidgetItem(error.severity);
        QTableWidgetItem *typeItem = new QTableWidgetItem(error.errorType);
        QTableWidgetItem *lineItem = new QTableWidgetItem(error.line > 0 ? QString::number(error.line) : "");
        QTableWidgetItem *columnItem = new QTableWidgetItem(error.column > 0 ? QString::number(error.column) : "");
        QTableWidgetItem *messageItem = new QTableWidgetItem(error.message);
        QTableWidgetItem *detailsItem = new QTableWidgetItem(error.details);

        // Set colors based on severity
        if (error.severity == "Error") {
            QColor errorColor(255, 200, 200);
            severityItem->setBackground(errorColor);
            typeItem->setBackground(errorColor);
            lineItem->setBackground(errorColor);
            columnItem->setBackground(errorColor);
            messageItem->setBackground(errorColor);
            detailsItem->setBackground(errorColor);
        } else {
            QColor warningColor(255, 255, 200);
            severityItem->setBackground(warningColor);
            typeItem->setBackground(warningColor);
            lineItem->setBackground(warningColor);
            columnItem->setBackground(warningColor);
            messageItem->setBackground(warningColor);
            detailsItem->setBackground(warningColor);
        }

        validationTable->setItem(row, 0, severityItem);
        validationTable->setItem(row, 1, typeItem);
        validationTable->setItem(row, 2, lineItem);
        validationTable->setItem(row, 3, columnItem);
        validationTable->setItem(row, 4, messageItem);
        validationTable->setItem(row, 5, detailsItem);
    }

    validationTable->resizeColumnsToContents();
*/

    if (result.errors.isEmpty()) {
        QMessageBox::information(this, tr("XML редактор"),
                                 QString(tr("Проверка пройдена: %1")).arg(result.summary));

        statusBar()->showMessage(tr("Проверка пройдена: ") + result.summary, 3000);
    } else {
        QMessageBox::warning(this, tr("XML редактор"),
                             QString(tr("Проверка не удалась: %1 errors, %2 warnings"))
                                 .arg(result.errorCount).arg(result.warningCount));

        statusBar()->showMessage(QString(tr("Проверка не удалась: %1 errors, %2 warnings"))
                                     .arg(result.errorCount).arg(result.warningCount), 5000);
    }
}

void XmlEditor::onLoadButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Открыть Device Description File"),
                                                   QDir::homePath(),
                                                    tr("XML Files (*.xml);;Text Files (*.txt);;All Files (*)"));
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {

        if (fileName.isEmpty()) {
            return;
        }

        QMessageBox::warning(this, tr("XML редактор"), QString(tr("Невозможно прочитать файл %1:\n%2.")).arg(fileName, file.errorString()));
        return;
    }

    if (fileName.isEmpty()) {
        return;
    }

    isNew = false;

    QString content = file.readAll();

    // Предварительная проверка WellFormed
    QString errorMsg;
    int errorLine, errorColumn;
    QXmlStreamReader reader(content);
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.hasError()) {
            errorMsg = reader.errorString();
            errorLine = reader.lineNumber();
            errorColumn = reader.columnNumber();
            QMessageBox::warning(this, tr("XML редактор"), QString(tr("Ошибка анализа в строке %1, column %2:\n%3")).arg(errorLine).arg(errorColumn).arg(errorMsg));
            return;
        }
    }

    //QDomDocument newDoc;
    if (!newDoc.setContent(content)) { // setContent тоже проверяет WellFormed
        QMessageBox::warning(this, tr("XML редактор"), tr("Неправильно сформированное содержимое XML"));
        return;
    }
    xmlTree = new XmlTree();
    xmlTree->setXmlDocument(newDoc);    

    if(isOpenXML)
    {
        closeFile();
        closeCentralWidget();
    }
    createXMLDeviceDescriptionWidget();

    isOpenXMLDeviceDescription = true;

    clearModels();

    // Запускаем парсинг в отдельном потоке
    future = QtConcurrent::run(parseXMLFile, fileName);
    future_watcher.setFuture(future);    

    setCurrentFile(fileName);
}

void XmlEditor::onParseFinished()
{
    if (future.isCanceled()) {
        QMessageBox::warning(this, tr("Предупреждение"), tr("Загрузка отменена."));
        return;
    }

    auto result = future.result();
    QDomDocument doc = result.first;
    const QMap<QString, TypeInfo> &types = result.second.first;
    const DeviceInfo &devInfo = result.second.second;

    if (doc.isNull() || (types.isEmpty() && devInfo.connectors.isEmpty() && devInfo.deviceParameters.isEmpty() && devInfo.typeName.isEmpty())) {
         QMessageBox::critical(this, tr("Ошибка"), tr("Не удалось проанализировать файл или файл пуст."));
         return;
    }

    // Сохраняем разобранные данные и оригинальный DOM
    originalDomDoc = doc;
    parsedTypes = types;
    parsedDeviceInfo = devInfo;

    typesModel->setData(&parsedTypes);
    parametersModel->setData(&parsedDeviceInfo);
    connectorsModel->setData(&parsedDeviceInfo);
    deviceInfoModel->setData(&parsedDeviceInfo);
    filesModel->setData(&parsedDeviceInfo);
    deviceIdentificationModel->setData(&parsedDeviceInfo);

    hasUnsavedChanges = false; // Сброс после загрузки
    isModified = false;
}

void XmlEditor::clearModels()
{
    typesModel->setData(nullptr);
    parametersModel->setData(nullptr);
    connectorsModel->setData(nullptr);
    deviceInfoModel->setData(nullptr);
    filesModel->setData(nullptr);
    deviceIdentificationModel->setData(nullptr);
    attributesPopup->hide();
}

bool XmlEditor::saveCurrentFile()
{
    if (currentFile.isEmpty() || originalDomDoc.isNull()) {
        return false;
    }
    bool success = saveXMLFile(currentFile, originalDomDoc, parsedTypes, parsedDeviceInfo);
    if (success) {
        hasUnsavedChanges = false;
        isModified = false;
        setWindowTitle(tr("XmlEditor - %1").arg(QFileInfo(currentFile).fileName()));
    }
    return success;
}

void XmlEditor::about()
{
    QMessageBox::information(this, tr("О XML редакторе %1").arg(tr("1.0")),
                             tr("XML редактор %1.\n\n"
                                "Авторские права 2025 г.").arg(tr("1.0")));
}

void XmlEditor::setDarkPalette()
{
    // Создать палитру для тёмной темы оформления
     QPalette darkPalette;

     QColor base_color = QColor(0, 23, 25); //QColor(0, 38, 43);//QColor(25, 25, 25);
     QColor background_color = QColor(0, 44, 50);//QColor(0, 68, 77); //QColor(53, 53, 53);

     //QColor background_color_1 = QColor(26, 89, 97);//QColor(0, 68, 77); //QColor(53, 53, 53);
     QColor text_color = Qt::white;
     //QColor text_color_1 = QColor(186, 186, 186);

     QColor highlighted_text_color = Qt::black;//Qt::white;//QColor(69, 157, 131); //
     //QColor highlighted_color = QColor(26, 89, 97); //Qt::black;

     //QColor link_color = QColor(99, 181, 188); //QColor(99, 151, 158); //69, 157, 131);
     QColor link_color = QColor(82, 199, 155);//99, 181, 188); //QColor(99, 151, 158); //69, 157, 131); 82 199 155
     QColor light_color = QColor(16, 79, 87);

     QColor placeholder_text_color = QColor(99, 181, 188);
     //QPalette::PlaceholderText
    // Настраиваем палитру для цветовых ролей элементов интерфейса
     darkPalette.setColor(QPalette::PlaceholderText,  placeholder_text_color); //цвет фона - не виден

     darkPalette.setColor(QPalette::Window, background_color); //цвет фона - не виден
     darkPalette.setColor(QPalette::WindowText, Qt::white); //цвер текста - не виден

     darkPalette.setColor(QPalette::Base, base_color); //базовый цвет - цвет выпадающих меню
     darkPalette.setColor(QPalette::AlternateBase, background_color);//альтернативный базовый цвет - не виден

     darkPalette.setColor(QPalette::ToolTipBase, text_color); //цвет всплювающей подсказки - не виден
     darkPalette.setColor(QPalette::ToolTipText, text_color);//альтернативный цвет всплювающей подсказки - не виден

 //    darkPalette.setColor(QPalette::Foreground, text_color);//цвет текста меток (QLabel)

 //    darkPalette.setColor(QPalette::Background, background_color);//цвет фона главного окна и вызываемых окон приложения и меню
     darkPalette.setColor(QPalette::Text, text_color); //цвет текста выпадающих меню

     darkPalette.setColor(QPalette::Button, background_color); //цвет фона кнопок, виджетов, вкладок (с разными оттенками)
     darkPalette.setColor(QPalette::ButtonText, text_color); //цвет текста кнопок, главного меню и маркеров выпадающих меню

     darkPalette.setColor(QPalette::BrightText, Qt::green); //не выден

     darkPalette.setColor(QPalette::Link, link_color); //не виден
     darkPalette.setColor(QPalette::LinkVisited, Qt::red);//не виден

     darkPalette.setColor(QPalette::Highlight, link_color);//base_color); //highlighted_color); //цвет отводки контура кнопок, выделенного текста и текста выбранных пунктов меню
     darkPalette.setColor(QPalette::HighlightedText, highlighted_text_color); //цвет обводки выделенного текста и текста выбранных пунктов меню

     darkPalette.setColor(QPalette::Light, light_color);// link_color); //светлый цвет обводки фрейма (если утоплен или приподнят)
     darkPalette.setColor(QPalette::Dark,  base_color); //темный цвет обводки фрейма (если утоплен или приподнят)
     darkPalette.setColor(QPalette::Shadow, link_color); //Qt::black);//не виден

     QBrush brush = darkPalette.window();
     brush.setColor(brush.color().darker());

     darkPalette.setBrush(QPalette::Disabled, QPalette::WindowText, brush);
     darkPalette.setBrush(QPalette::Disabled, QPalette::Text, brush);
     darkPalette.setBrush(QPalette::Disabled, QPalette::ButtonText, brush);
     darkPalette.setBrush(QPalette::Disabled, QPalette::Base, brush);
     darkPalette.setBrush(QPalette::Disabled, QPalette::Button, brush);
     darkPalette.setBrush(QPalette::Disabled, QPalette::Mid, brush);

     // Установить палитру
     this->setPalette(darkPalette);

     fileMenu->setPalette(darkPalette);
     editMenu->setPalette(darkPalette);
     viewMenu->setPalette(darkPalette);
     validationMenu->setPalette(darkPalette);
     helpMenu->setPalette(darkPalette);

 //    devices_widget->deviceTree_widget->poup_menu->setPalette(darkPalette);
 //    devices_widget->deviceTree_widget->addDeviceSubMenu->setPalette(darkPalette);
 //    devices_widget->deviceTree_widget->addPortSubMenu->setPalette(darkPalette);
 //    devices_widget->deviceTree_widget->addInterfaceSubMenu->setPalette(darkPalette);

 //    devices_widget->dmx_widget->poup_menu->setPalette(darkPalette);
 //    devices_widget->dmx_widget->addRgbSubMenu->setPalette(darkPalette);

      //******** 2021.06.17
 //    devices_widget->mdi_widget->mdi_num_comboBox->setPalette(darkPalette);
 //    devices_widget->mdi_widget->mdi_num_comboBox->view()->setPalette(darkPalette);
 //    devices_widget->mdo_widget->mdo_num_comboBox->setPalette(darkPalette);
 //    devices_widget->mdo_widget->mdo_num_comboBox->view()->setPalette(darkPalette);
 //    devices_widget->dmx_widget->dmx_num_comboBox->setPalette(darkPalette);
 //    devices_widget->dmx_widget->dmx_num_comboBox->view()->setPalette(darkPalette);
     //******
 //    devices_widget->setPalette(darkPalette);
 //    devices_widget->plc_widget->setPalette(darkPalette);
 //    devices_widget->plc_widget->setDarkPalette(true);
 //    devices_widget->plc_widget->bridge_widget->bridgeNum_comboBox->setPalette(darkPalette);
 //    devices_widget->plc_widget->bridge_widget->bridgeNum_comboBox->view()->setPalette(darkPalette);
 //    devices_widget->plc_widget->indicator_widget->num_comboBox->setPalette(darkPalette);
 //    devices_widget->plc_widget->indicator_widget->num_comboBox->view()->setPalette(darkPalette);

 //    time_label->setPalette(darkPalette);

 //    this->devices_widget->setStyleSheet("QToolTip{background-color: rgb(0, 23, 25); color: white; border: 1px solid rgb(26, 89, 97);}");
}
