#pragma once

#include <QMainWindow>
#include <QPair>
#include <QPushButton>
#include <QFutureWatcher>
#include <QDomDocument>
#include <QPalette>
#include <QStyle>
#include <QStyleFactory>
#include <QStylePainter>
#include <QTableWidgetItem>
#include <QComboBox>
#include <QScreen>
#include <QMenu>

#include "xmlvalidator.h"
#include "xmldevdesc.h"

class QTextEdit;
class QSplitter;
class XmlTree;
class XmlTreeItem;
class QTableWidget;

class XmlEditor : public QMainWindow {
    Q_OBJECT

public:
    explicit XmlEditor(QWidget *parent = nullptr);

private slots:
    // --- Управление UI и взаимодействие ---
    void newFile();
    void openFile();
    void saveFile_();
    void saveAsFile_();
    void closeFile();

    void addNode();  // Вызывает xmlTree->requestAddNode(this);
    void editNode(); // Вызывает xmlTree->requestEditNode(this);
    void removeNode(); // Вызывает xmlTree->requestRemoveNode(this);

    void updateDisplay();
    void updateTreeView();
    void updateXmlView_();

    // --- Валидация ---
    // Validation operations
    void fullXmlValidation();
    void validateWellFormed();
    void validateSyntax();
    void validateElementNames();
    void validateAttributes();
    void validateCharacterData();
    void showValidationReport();

    void about();

    // Tree events
    // --- Обработчики сигналов от XmlTree ---
    void onNodeOperationFinished();
    void onOperationFailed(const QString &error);
    //void onTreeItemDoubleClicked(XmlTreeItem *item);    

    void onAttributeChangedFromPopup(const QModelIndex &paramIndex, const QString &attrName, const QString &newValue);
    void onParametersContextMenuRequested(const QPoint &pos);
    void markAsChangedForModel(const QModelIndex &, const QModelIndex &, const QVector<int> &);

    void onParametersContextMenuRequested2(const QPoint &pos);
    void onDeleteParameterTriggered();
    void onAddDiscreteInputChannelTriggered();
    void onAddDiscreteOutputChannelTriggered();
    void onAddAnalogInputChannelTriggered();
    void onAddAnalogOutputChannelTriggered();
    void onAddConfigParameterTriggered();

    void onNewClicked();


protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void addParameterToHostParameterSet(int connectorIndex, NewParameterType paramType, const QString &baseId, const QString &baseName, const QString &type);
    QString findNextAvailableId(const QList<DeviceParameter> &paramList, const QString &baseId);

    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createCentralWidget();
    void closeCentralWidget();
    void createXMLDeviceDescriptionWidget();
    void closeXMLDeviceDescriptionWidget();
    void setDarkPalette(); //установить темную тему

    //QString domToString();
    void setCurrentFile(const QString &fileName);
    void saveFile(const QString &fileName);
    bool maybeSave();
    void updateXmlView(const QDomDocument & newDoc);

    // Validation methods
    void performValidation(XmlValidator::ValidationTypes types);
    void displayValidationResults(const XmlValidator::ValidationResult &result);

    // UI Components
    XmlTree     *xmlTree;
    QTextEdit   *xmlTextEdit;    
    QDomDocument newDoc;
    QSplitter   *mainSplitter = nullptr;
    QMenu       *fileMenu;
    QMenu       *editMenu;
    QMenu       *viewMenu;
    QMenu       *validationMenu;
    QMenu       *helpMenu;

    QToolBar    *fileToolBar;
    QToolBar    *editToolBar;
    QToolBar    *updateToolBar;
    QToolBar    *validationToolBar;

    // Actions
    QAction *newAct;
    QAction *openAct;
    QAction *openXMLDeviceDescriptionAct;
    QAction *newXMLDeviceDescriptionAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *exitAct;
    QAction *closeAct;

    QAction *addNodeAct;
    QAction *editNodeAct;
    QAction *removeNodeAct;

    QAction *updateTreeAct;
    QAction *updateXmlAct;
    QAction *updateAllAct;

    QAction *fullValidationAct;
    QAction *wellFormedAct;
    QAction *syntaxAct;
    QAction *elementNamesAct;
    QAction *attributesAct;
    QAction *characterDataAct;
    QAction *validationReportAct;

    QAction *aboutAct;
    QAction *aboutQtAct;

    QString currentFile;
    bool isModified;
    bool isOpenXML = false;
    bool isOpenXMLDeviceDescription = false;

    QString errorMessage;

    XmlValidator *validator;

    QTabWidget *tabWidget;

    QFuture< QPair<QDomDocument, QPair<QMap<QString, TypeInfo>, DeviceInfo>> > future;
    QFutureWatcher< QPair<QDomDocument, QPair<QMap<QString, TypeInfo>, DeviceInfo>> > future_watcher;
    void onLoadButtonClicked();
    void onParseFinished();
    void clearModels();

    // Хранение разобранных данных
    QDomDocument originalDomDoc; // Оригинальный DOM документ
    QMap<QString, TypeInfo> parsedTypes;
    DeviceInfo parsedDeviceInfo;

    // Состояние
    bool hasUnsavedChanges = false;
    bool saveCurrentFile();

    TypesModel *typesModel;
    ParametersModel *parametersModel;
    ConnectorsModel *connectorsModel;
    DeviceInfoModel *deviceInfoModel;
    FilesModel *filesModel;
    DeviceIdentificationModel *deviceIdentificationModel;

    QTableView *typesView;
    QTableView *parametersView;
    QTableView *connectorsView;
    QTableView *deviceInfoView;
    QTableView *filesView;
    QTableView *deviceIdentificationView;

    // Всплывающее окно атрибутов ---
    AttributesPopup *attributesPopup;

    QMenu * menu = nullptr;
    QAction *deleteParameterAction = nullptr;
    QAction *addDiscreteInputParameterAction = nullptr;
    QAction *addDiscreteOutputParameterAction = nullptr;
    QAction *addAnalogInputParameterAction = nullptr;
    QAction *addAnalogOutputParameterAction = nullptr;
    QAction *addConfigParameterAction = nullptr;
    QAction *editAttrs = nullptr;

    // Хранение индекса строки, на которой было вызвано контекстное меню
    QModelIndex currentParameterIndex;

    QString currentFileName;
    bool isNew = false;
};
