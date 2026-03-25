#pragma once

#include <QString>
#include <QList>
#include <QXmlStreamReader>
#include <QDomDocument>
#include <QMap>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStyledItemDelegate>
#include <QItemDelegate>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QAbstractTableModel>
#include <QAbstractItemView>
#include <QItemSelectionModel>
#include <QStyleOptionViewItem>
#include <QPainter>
#include <QComboBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QApplication>
#include <QScreen>
#include <QListWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>

/*
1. typesTable:
    Столбец 0 ("Type Name"): Это имя типа (например, CPUSNTPConfiguration, Watchdog_Configuration). Оно служит уникальным идентификатором (name атрибут)
    для определения типа в XML (<StructType name="...">, <EnumType name="...">, и т.д.). Если изменить это имя в GUI, оно больше не будет соответствовать
    ссылкам на этот тип из других частей XML (например, в Parameter type="localTypes:CPUSNTPConfiguration"). Это нарушило бы структуру и целостность файла.
    Столбец 1 ("Component ID"): Это имя компонента внутри типа (например, SNTP_SERVICE_ENABLE_DISABLE, WD_TIME_SET). Оно служит уникальным идентификатором
    (identifier атрибут) для компонента в его StructType или BitfieldType. Изменение этого идентификатора также нарушило бы соответствие между определением и использованием.
    Столбец 2 ("Type"): Это тип данных, который сам компонент должен иметь (например, localTypes:ALLOWTYPE, std:ARRAY[0..3] OF BYTE). Это фиксированная
    часть определения структуры. Изменение типа компонента сделало бы определение несогласованным с его изначальным предназначением и могло бы привести к
    ошибкам в приложении, использующем это описание устройства.
2. parametersTable:
    Столбец 0 ("ID"): Это ParameterId (например, 500000, 500004). Это уникальный идентификатор для параметра, используемый системой (CoDeSys/3S) для внутренней
    идентификации и доступа к этому конкретному параметру. Изменение ParameterId сделало бы его несоответствующим любым ссылкам или настройкам, которые могут с
    уществовать в других частях системы или проекта, и нарушит функциональность.

Вывод: Эти столбцы содержат ключевые идентификаторы, которые определяют структуру XML-файла и связи между его элементами. Изменение этих идентификаторов может сломать
файл или привести к непредсказуемому поведению при его использовании. Поэтому они были сделаны недоступными для редактирования в GUI. Другие столбцы содержат значения
или свойства, которые можно изменить без нарушения базовой структуры (например, значения по умолчанию, описания, атрибуты доступа).
*/

/*
Столбцы "Type Name", "Component ID", "Type" в typesTable и столбец "ID" в parametersTable были помечены как только для чтения.

Цель: Эти столбцы содержат ключевые идентификаторы или определения типов, которые формируют структуру XML-файла Device Description и связи между его элементами.
Изменение этих значений может нарушить целостность файла и привести к ошибкам в системе, использующей это описание (например, CoDeSys/3S).

Разбор по таблицам:

1. typesTable (Таблица Типов):
    Столбец 0 ("Type Name"):
        Что содержит: Имя определяемого типа (например, TBit1Byte, CPUSNTPConfiguration, ALLOWTYPE).
        Где используется в XML: Это значение находится в атрибуте name элемента типа, например <StructType name="CPUSNTPConfiguration">, <EnumType name="ALLOWTYPE">.
        Почему только для чтения: Это имя уникально идентифицирует тип внутри секции <Types>. Другие части XML-файла (например, определения параметров в DeviceParameterSet)
        ссылаются на этот тип, используя это имя. Пример: <Parameter ParameterId="..." type="localTypes:CPUSNTPConfiguration">. Если изменить имя типа в определении (например,
        с CPUSNTPConfiguration на NewConfig), но не обновить все ссылки на него (в Parameter type="..."), система не сможет найти определение NewConfig по старой ссылке
        localTypes:CPUSNTPConfiguration, что приведёт к ошибке. Редактирование этого имени требует глобального обновления всех ссылок, что гораздо сложнее, чем простое
        изменение ячейки. Поэтому, для безопасности и предотвращения ошибок, это поле помечается как недоступное для редактирования в простом GUI.
    Столбец 1 ("Component ID"):
        Что содержит: Имя компонента внутри StructType или BitfieldType (например, SNTP_SERVICE_ENABLE_DISABLE, WD_TIME_SET, Bit0).
        Где используется в XML: Это значение находится в атрибуте identifier элемента Component, например <Component identifier="SNTP_SERVICE_ENABLE_DISABLE" type="...">.
        Почему только для чтения: Это имя уникально идентифицирует конкретный компонент внутри данного типа. Оно используется системой для доступа к конкретному полю структуры.
        Изменение identifier сделало бы его несоответствующим любому внутреннему использованию или ожиданиям системы, связанного с этой структурой. Это также нарушает
        определение структуры. Как и с именем типа, изменение идентификатора компонента требует сложной синхронизации и потенциально ломает структуру.
    Столбец 2 ("Type") (для Component):
        Что содержит: Тип данных, который должен иметь компонент (например, localTypes:ALLOWTYPE, std:ARRAY[0..3] OF BYTE, std:BOOL).
        Где используется в XML: Это значение находится в атрибуте type элемента Component, например <Component identifier="..." type="localTypes:ALLOWTYPE">.
        Почему только для чтения: Это определение типа компонента. Оно жёстко задаёт, какие значения может принимать этот компонент, как они хранятся и интерпретируются.
        Изменение типа (например, с std:BOOL на std:WORD) изменило бы смысл и размер данных компонента, что нарушило бы структуру StructType и привело бы к несоответствию с
        ожиданиями системы, использующей эту структуру. Это критическое изменение, влияющее на саму суть определения типа, поэтому оно не редактируется в простом интерфейсе.
2. parametersTable (Таблица Параметров):
    Столбец 0 ("ID"):
        Что содержит: Уникальный идентификатор параметра (например, 500000, 1000, 393218).
        Где используется в XML: Это значение находится в атрибуте ParameterId элемента Parameter, например <Parameter ParameterId="500000" type="...">.
        Почему только для чтения: ParameterId служит уникальным идентификатором для конкретного параметра в системе CoDeSys/3S. Система использует этот ID для внутренней
        идентификации, доступа к значению параметра, настройки его поведения, сохранения в энергонезависимой памяти и т.д. Изменение ParameterId означало бы, что система
        больше не сможет найти этот параметр по его старому ID, или что новый параметр с другим ID теперь требует отдельной настройки. Это может привести к потере конфигурации,
        ошибкам связи или неправильной работе устройства. Это критический идентификатор, изменение которого требует тщательного планирования и синхронизации, выходящих за
        рамки простого редактирования значения в таблице.
Вывод: Эти столбцы содержат ключевые элементы структуры XML-файла. Изменение их значений в GUI без соответствующего обновления всех связанных частей файла приведёт к неправильной
        интерпретации или ошибкам в системе, использующей это описание устройства. Поэтому, в интересах сохранения целостности и предотвращения случайных ошибок, они были сделаны
        недоступными для редактирования в простом табличном представлении. Другие столбцы, содержащие, например, значения по умолчанию (Default), описания (Description, VisibleName),
        атрибуты доступа (Attributes), могут быть изменены без нарушения базовой структуры, и поэтому они доступны для редактирования.
*/

enum class DeviceType {
    DiscreteIO,
    DiscreteInput,
    DiscreteOutput,
    AnalogInput,
    AnalogOutput,
    //Specialized,
    RS232,
    RS422,
    RS485,
    SSI,
    Encoder,
    PWM,
    Drivver_Stepping_Moror,
    Power,
};

// Добавим enum для типа добавляемого параметра
enum class NewParameterType {
    InputDiscreteChannel,
    OutputDiscreteChannel,
    InputAnalogChannel,
    OutputAnalogChannel,
    ConfigParameter
};

//QDomDocument createNewDeviceDescriptionDocumentBasedOnType(DeviceType type);
//QPair<QDomDocument, QPair<QMap<QString, TypeInfo>, DeviceInfo>> parseFromQDomDoc(const QDomDocument &doc);

class NewDeviceDialog : public QDialog {
    Q_OBJECT
public:
    explicit NewDeviceDialog(QWidget *parent = nullptr) : QDialog(parent) {
        setWindowTitle(tr("Создать новый Device Description"));
        setModal(true);

        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        QLabel *label = new QLabel(tr("Выберите тип устройства:"));
        mainLayout->addWidget(label);

        listWidget = new QListWidget();
        // Заполним список элементами
        addDeviceTypeItem(DeviceType::DiscreteIO, tr("Дискретный Ввод/Вывод"), tr("Дискретные устройства ввода/вывода"));
        addDeviceTypeItem(DeviceType::DiscreteInput, tr("Дискретный Ввод"), tr("Дискретные устройства ввода"));
        addDeviceTypeItem(DeviceType::DiscreteOutput, tr("Дискретный Вывод"), tr("Дискретные устройства вывода"));
        addDeviceTypeItem(DeviceType::AnalogInput, tr("Аналоговый Ввод"), tr("Аналоговые устройства ввода"));
        addDeviceTypeItem(DeviceType::AnalogOutput, tr("Аналоговый Вывод"), tr("Аналоговые устройства вывода"));
        //addDeviceTypeItem(DeviceType::Specialized, tr("Specialized"), tr("Encoders, PWM, etc."));
        addDeviceTypeItem(DeviceType::RS232, tr("Устройства RS232"), tr("Устройства RS232"));
        addDeviceTypeItem(DeviceType::RS422, tr("Устройства RS422"), tr("Устройства RS422"));
        addDeviceTypeItem(DeviceType::RS422, tr("Устройства RS485"), tr("Устройства RS485"));
        addDeviceTypeItem(DeviceType::RS422, tr("Устройства SSI"), tr("Устройства SSI (последовательный синхронный интерфейс)"));
        addDeviceTypeItem(DeviceType::Encoder, tr("Устройства Encoder"), tr("Устройства Encoder"));
        addDeviceTypeItem(DeviceType::PWM, tr("Устройства ШИМ"), tr("Устройства ШИМ"));
        addDeviceTypeItem(DeviceType::Drivver_Stepping_Moror, tr("Устройства Драйвер Шагового двигателя"), tr("Устройства Драйвер Шагового двигателя"));


        addDeviceTypeItem(DeviceType::Power, tr("Источник питания"), tr("Устройства питания"));
        //addDeviceTypeItem(DeviceType::RS232, tr("Устройства RS232"), tr("Устройства RS232"));

        mainLayout->addWidget(listWidget);

        QHBoxLayout *buttonLayout = new QHBoxLayout();
        QPushButton *okButton = new QPushButton("OK");
        QPushButton *cancelButton = new QPushButton("Cancel");
        buttonLayout->addStretch();
        buttonLayout->addWidget(okButton);
        buttonLayout->addWidget(cancelButton);

        mainLayout->addLayout(buttonLayout);

        connect(listWidget, &QListWidget::itemDoubleClicked, this, &QDialog::accept);
        connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
        connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
        // Выбор по умолчанию
        listWidget->setCurrentRow(0);
    }

    DeviceType getSelectedType() const {
        int row = listWidget->currentRow();
        // Соответствие индекса списка и enum
        if (row >= 0 && row < static_cast<int>(DeviceType::Power) + 1) {
            return static_cast<DeviceType>(row);
        }
        return DeviceType::DiscreteIO; // Значение по умолчанию
    }

private:
    void addDeviceTypeItem(DeviceType type, const QString &title, const QString &description) {
        Q_UNUSED(type);

        QListWidgetItem *item = new QListWidgetItem();
        item->setText(title);
        item->setToolTip(description); // Подсказка при наведении
        // Здесь можно установить иконку, если есть
        // QIcon icon(":/icons/discrete_io.png"); // Путь к иконке
        // item->setIcon(icon);
        listWidget->addItem(item);
    }

    QListWidget *listWidget;
};

struct TypeInfo {
    QString name;
    QString type;
    QString baseType;
    QString minVal;
    QString maxVal;
    QString defaultVal;
    struct EnumValue {
        QString identifier;
        int value;
        QString visibleName;
        bool operator==(const EnumValue& other) const {
            return identifier == other.identifier &&
                   value == other.value &&
                   visibleName == other.visibleName;
        }
    };
    QList<EnumValue> enumValues;
    struct Component {
        QString identifier;
        QString type;
        QString defaultValue;
        QString visibleName;
        QString unit;
        int originalIndex;
        bool isModified;
        Component() : isModified(false) {}
        bool operator==(const Component& other) const {
            return identifier == other.identifier &&
                   type == other.type &&
                   defaultValue == other.defaultValue &&
                   visibleName == other.visibleName &&
                   unit == other.unit &&
                   originalIndex == other.originalIndex &&
                   isModified == other.isModified;
        }
    };
    QList<Component> components;
    int originalIndex;
    bool isModified;
    TypeInfo() : isModified(false) {}
    bool operator==(const TypeInfo& other) const {
        return name == other.name &&
               type == other.type &&
               baseType == other.baseType &&
               minVal == other.minVal &&
               maxVal == other.maxVal &&
               defaultVal == other.defaultVal &&
               enumValues == other.enumValues &&
               components == other.components &&
               originalIndex == other.originalIndex &&
               isModified == other.isModified;
    }
};

// --- В структурах данных ---
struct DeviceParameter {
    QString id;
    QString type;
    QString name;
    QString description;
    QMap<QString, QString> attributesMap; // Атрибуты параметра
    // --- Новые поля для атрибутов Name и Description ---
    QMap<QString, QString> nameAttributesMap;
    QMap<QString, QString> descriptionAttributesMap;
    // --- /Новые поля ---
    QString defaultValue;
    int originalIndex;
    bool isFromHostParameterSet;
    int connectorIndex;
    bool isModified;
    DeviceParameter() : isModified(false) {}
    bool operator==(const DeviceParameter& other) const {
        return id == other.id &&
               type == other.type &&
               name == other.name &&
               description == other.description &&
               attributesMap == other.attributesMap &&
               // --- Сравниваем новые поля ---
               nameAttributesMap == other.nameAttributesMap &&
               descriptionAttributesMap == other.descriptionAttributesMap &&
               // --- /Сравниваем новые поля ---
               defaultValue == other.defaultValue &&
               originalIndex == other.originalIndex &&
               isFromHostParameterSet == other.isFromHostParameterSet &&
               connectorIndex == other.connectorIndex &&
               isModified == other.isModified;
    }
};

struct ConnectorInfo {
    QString moduleType;
    QString interface;
    QString role;
    QString connectorId;
    QString hostpath;
    QString interfaceName;
    QList<DeviceParameter> parameters;
    int originalIndex;
    bool isModified;
    ConnectorInfo() : isModified(false) {}
    bool operator==(const ConnectorInfo& other) const {
        return moduleType == other.moduleType &&
               interface == other.interface &&
               role == other.role &&
               connectorId == other.connectorId &&
               hostpath == other.hostpath &&
               interfaceName == other.interfaceName &&
               parameters == other.parameters &&
               originalIndex == other.originalIndex &&
               isModified == other.isModified;
    }
};

struct DeviceInfo {
    QString typeName;
    QString typeDescription;
    QString vendor;
    QString orderNumber;
    QString image;
    QList<ConnectorInfo> connectors;
    QList<DeviceParameter> deviceParameters;
    struct File {
        QString fileref;
        QString identifier;
        QString localFile;
        bool isModified;
        File() : isModified(false) {}
    };
    QList<File> files;
    struct DeviceIdentification {
        QString type;
        QString id;
        QString version;
        bool isModified;
        DeviceIdentification() : isModified(false) {}
    };
    DeviceIdentification deviceIdentification;
    static const int TYPE_NAME_IDX = 0;
    static const int DESCRIPTION_IDX = 1;
    static const int VENDOR_IDX = 2;
    static const int ORDER_NUMBER_IDX = 3;
    static const int IMAGE_IDX = 4;
    bool isModified;
    DeviceInfo() : isModified(false) {}
    void updateField(int fieldIdx, const QString &newValue) {
        bool changed = false;
        switch (fieldIdx) {
            case TYPE_NAME_IDX:
                if (typeName != newValue) { typeName = newValue; changed = true; }
                break;
            case DESCRIPTION_IDX:
                if (typeDescription != newValue) { typeDescription = newValue; changed = true; }
                break;
            case VENDOR_IDX:
                if (vendor != newValue) { vendor = newValue; changed = true; }
                break;
            case ORDER_NUMBER_IDX:
                if (orderNumber != newValue) { orderNumber = newValue; changed = true; }
                break;
            case IMAGE_IDX:
                if (image != newValue) { image = newValue; changed = true; }
                break;
        }
        if (changed) {
            isModified = true;
        }
    }
};

// --- Роли ---
const int TypeInfoRole = Qt::UserRole + 1;
const int ComponentIndexRole = Qt::UserRole + 2;
const int ParameterIndexRole = Qt::UserRole + 3;
const int IsHostParameterRole = Qt::UserRole + 4;
const int ConnectorIndexRole = Qt::UserRole + 5;
const int DeviceInfoFieldRole = Qt::UserRole + 6;
const int ConnectorAttributeRole = Qt::UserRole + 7;
const int ComponentFieldRole = Qt::UserRole + 8;
const int SelectedParameterRole = Qt::UserRole + 9;
const int AttributeNameRole = Qt::UserRole + 10;
const int TypeIndexRole = Qt::UserRole + 12;


void parseEnum(QXmlStreamReader &reader, QList<TypeInfo::EnumValue> &enumValues);
void parseComponent(QXmlStreamReader &reader, QList<TypeInfo::Component> &components);
void parseEnumType(QXmlStreamReader &reader, TypeInfo &typeInfo);
void parseRangeType(QXmlStreamReader &reader, TypeInfo &typeInfo);
void parseStructType(QXmlStreamReader &reader, TypeInfo &typeInfo);
void parseBitfieldType(QXmlStreamReader &reader, TypeInfo &typeInfo);
void parseParameter(QXmlStreamReader &reader, QList<DeviceParameter> &parameters, const QString &setName);
QPair<QDomDocument, QPair<QMap<QString, TypeInfo>, DeviceInfo>> parseXMLFile(const QString &fileName);
bool saveXMLFile(const QString &fileName, QDomDocument &doc, const QMap<QString, TypeInfo> &types, const DeviceInfo &devInfo);
QDomDocument createNewDeviceDescriptionDocumentBasedOnType(DeviceType type);
QPair<QDomDocument, QPair<QMap<QString, TypeInfo>, DeviceInfo>> parseFromQDomDoc(const QDomDocument &doc);

class TypesModel : public QAbstractTableModel {
    Q_OBJECT
    friend class TextEditDelegate;
public:
    explicit TypesModel(QObject *parent = nullptr) : QAbstractTableModel(parent) {}
    void setData(QMap<QString, TypeInfo> *types) {
        beginResetModel();
        m_typesMap = types;
        m_flatList.clear();
        if (m_typesMap) {
            for (auto it = m_typesMap->constBegin(); it != m_typesMap->constEnd(); ++it) {
                const TypeInfo &typeInfo = it.value();
                m_flatList.append({it.key(), "", typeInfo.type, "", ""});
                if (typeInfo.type == "StructType" || typeInfo.type == "BitfieldType") {
                    for (const auto &comp : typeInfo.components) {
                        QString visibleAndUnit = comp.visibleName;
                        if (!comp.unit.isEmpty()) {
                            if (!comp.visibleName.isEmpty()) visibleAndUnit += " / ";
                            visibleAndUnit += comp.unit;
                        }
                        m_flatList.append({it.key(), comp.identifier, comp.type, comp.defaultValue, visibleAndUnit});
                    }
                }
                if (typeInfo.type == "EnumType") {
                    for (const auto &enumVal : typeInfo.enumValues) {
                        m_flatList.append({it.key(), enumVal.identifier, QString::number(enumVal.value), "", ""});
                    }
                }
            }
        }
        endResetModel();
    }
    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid()) return 0;
        return m_flatList.size();
    }
    int columnCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid()) return 0;
        return 5;
    }
    QVariant data(const QModelIndex &index, int role) const override {
        if (!index.isValid() || index.row() >= m_flatList.size() || index.column() >= 5 || !m_typesMap)
            return QVariant();
        const auto &row = m_flatList[index.row()];
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            switch (index.column()) {
                case 0: return row.typeName;
                case 1: return row.componentOrEnumId;
                case 2: return row.typeOrValue;
                case 3: return row.defaultValue;
                case 4: return row.visibleNameOrUnit;
                default: return QVariant();
            }
        }
        if (role == TypeInfoRole && index.column() == 0) {
            return row.typeName;
        }
        return QVariant();
    }
    // setData теперь НЕ обрабатывает изменение, только делегат
    bool setData(const QModelIndex &index, const QVariant &value, int role) override {
        Q_UNUSED(index);
        Q_UNUSED(value);
        Q_UNUSED(role);
        return false; // Всё делегат
    }
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            QStringList headers = {"Type Name", "Component/Enum ID", "Type/Value", "Default", "Visible Name/Unit"};
            if (section < headers.size()) return headers[section];
        }
        return QAbstractTableModel::headerData(section, orientation, role);
    }
    Qt::ItemFlags flags(const QModelIndex &index) const override {
        Qt::ItemFlags flags = QAbstractTableModel::flags(index);
        if (index.isValid()) {
            // Сделаем редактируемыми только нужные столбцы
            if (index.column() >= 3 && index.column() <= 4 && !m_flatList[index.row()].componentOrEnumId.isEmpty()) {
                flags |= Qt::ItemIsEditable;
            }
        }
        return flags;
    }
private:
    struct FlatRow {
        QString typeName;
        QString componentOrEnumId;
        QString typeOrValue;
        QString defaultValue;
        QString visibleNameOrUnit;
    };
    QList<FlatRow> m_flatList;
    QMap<QString, TypeInfo> *m_typesMap = nullptr;
};

class ParametersModel : public QAbstractTableModel {
    Q_OBJECT
    friend class TextEditDelegate; // <-- Разрешаем делегату доступ к приватным членам
public:
    explicit ParametersModel(QObject *parent = nullptr) : QAbstractTableModel(parent) {}
    void setData(DeviceInfo *devInfo) {
        beginResetModel();
        m_deviceInfo = devInfo;
        m_allParams.clear();
        if (m_deviceInfo) {
            for (int i = 0; i < m_deviceInfo->deviceParameters.size(); ++i) {
                m_allParams.push_back({&m_deviceInfo->deviceParameters[i], false, i, -1});
            }
            for (int j = 0; j < m_deviceInfo->connectors.size(); ++j) {
                auto& connParams = m_deviceInfo->connectors[j].parameters;
                for (int k = 0; k < connParams.size(); ++k) {
                    m_allParams.push_back({&connParams[k], true, k, j});
                }
            }
        }
        endResetModel();
    }
    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid()) return 0;
        return m_allParams.size();
    }
    int columnCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid()) return 0;
        return 5;
    }
    QVariant data(const QModelIndex &index, int role) const override {
        if (!index.isValid() || index.row() >= static_cast<int>(m_allParams.size()) || index.column() >= 5 || !m_deviceInfo)
            return QVariant();
        const auto &paramRef = m_allParams[index.row()];
        const DeviceParameter *param = paramRef.param;
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            switch (index.column()) {
                case 0: return param->id;
                case 1: return param->name;
                case 2: return param->type;
                case 3: return param->defaultValue;
                case 4: return param->description;
                default: return QVariant();
            }
        }
        if (role == ParameterIndexRole) return paramRef.originalIndex;
        if (role == IsHostParameterRole) return paramRef.isFromHostSet;
        if (role == ConnectorIndexRole) return paramRef.connectorIndex;
        return QVariant();
    }
    bool setData(const QModelIndex &index, const QVariant &value, int role) override {
        if (!index.isValid() || role != Qt::EditRole || index.column() == 0 || !m_deviceInfo)
            return false;
        auto &paramRef = m_allParams[index.row()];
        DeviceParameter *param = paramRef.param;
        bool fieldChanged = false;
        switch (index.column()) {
            case 1: if (param->name != value.toString()) { param->name = value.toString(); fieldChanged = true; } break;
            case 2: if (param->type != value.toString()) { param->type = value.toString(); fieldChanged = true; } break;
            case 3: if (param->defaultValue != value.toString()) { param->defaultValue = value.toString(); fieldChanged = true; } break;
            case 4: if (param->description != value.toString()) { param->description = value.toString(); fieldChanged = true; } break;
        }
        if (fieldChanged) {
            param->isModified = true;
            if (paramRef.isFromHostSet) {
                m_deviceInfo->connectors[paramRef.connectorIndex].isModified = true;
            }
            emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
            return true;
        }
        return false;
    }
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            QStringList headers = {"ID", "Name", "Type", "Default Value", "Description"};
            if (section < headers.size()) return headers[section];
        }
        return QAbstractTableModel::headerData(section, orientation, role);
    }
    Qt::ItemFlags flags(const QModelIndex &index) const override {
        Qt::ItemFlags flags = QAbstractTableModel::flags(index);
        if (index.isValid() && index.column() != 0) {
            flags |= Qt::ItemIsEditable;
        }
        return flags;
    }
    DeviceParameter* getParameter(const QModelIndex &index) const {
        if (index.isValid() && index.row() < static_cast<int>(m_allParams.size())) {
            return m_allParams[index.row()].param;
        }
        return nullptr;
    }
    void updateAttribute(const QModelIndex &paramIndex, const QString &attrName, const QString &newValue) {
        if (paramIndex.isValid() && paramIndex.row() < static_cast<int>(m_allParams.size()) && m_deviceInfo) {
            DeviceParameter *param = m_allParams[paramIndex.row()].param;
            if (param->attributesMap[attrName] != newValue) {
                param->attributesMap[attrName] = newValue;
                param->isModified = true;
                if (m_allParams[paramIndex.row()].isFromHostSet) {
                    m_deviceInfo->connectors[m_allParams[paramIndex.row()].connectorIndex].isModified = true;
                }
            }
        }
    }
private:
    DeviceInfo *m_deviceInfo = nullptr;
    struct ParamRef {
        DeviceParameter* param;
        bool isFromHostSet;
        int originalIndex;
        int connectorIndex;
    };
    std::vector<ParamRef> m_allParams;
};


class ConnectorsModel : public QAbstractTableModel {
    Q_OBJECT
    friend class TextEditDelegate;
public:
    explicit ConnectorsModel(QObject *parent = nullptr) : QAbstractTableModel(parent) {}
    void setData(DeviceInfo *devInfo) {
        beginResetModel();
        m_deviceInfo = devInfo;
        endResetModel();
    }
    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid() || !m_deviceInfo) return 0;
        return m_deviceInfo->connectors.size();
    }
    int columnCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid()) return 0;
        return 6;
    }
    QVariant data(const QModelIndex &index, int role) const override {
        if (!index.isValid() || index.row() >= m_deviceInfo->connectors.size() || index.column() >= 6 || !m_deviceInfo)
            return QVariant();
        const auto &conn = m_deviceInfo->connectors[index.row()];
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            switch (index.column()) {
                case 0: return conn.moduleType;
                case 1: return conn.interface;
                case 2: return conn.role;
                case 3: return conn.connectorId;
                case 4: return conn.hostpath;
                case 5: return conn.interfaceName;
                default: return QVariant();
            }
        }
        if (role == ConnectorIndexRole) return index.row();
        return QVariant();
    }
    // setData теперь НЕ обрабатывает изменение, только делегат
    bool setData(const QModelIndex &index, const QVariant &value, int role) override {
        Q_UNUSED(index);
        Q_UNUSED(value);
        Q_UNUSED(role);
        return false; // Всё делегат
    }
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            QStringList headers = {"Module Type", "Interface", "Role", "Connector ID", "Host Path", "Interface Name"};
            if (section < headers.size()) return headers[section];
        }
        return QAbstractTableModel::headerData(section, orientation, role);
    }
    Qt::ItemFlags flags(const QModelIndex &index) const override {
        Qt::ItemFlags flags = QAbstractTableModel::flags(index);
        if (index.isValid()) {
            flags |= Qt::ItemIsEditable;
        }
        return flags;
    }
    void updateRole(int row, const QString &newRole) {
        if (row >= 0 && row < m_deviceInfo->connectors.size()) {
            ConnectorInfo &conn = m_deviceInfo->connectors[row];
            if (conn.role != newRole) {
                conn.role = newRole;
                conn.isModified = true;
                QModelIndex index = createIndex(row, 2);
                emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
            }
        }
    }
    void updateField(int row, int col, const QString &newValue) {
        if (row >= 0 && row < m_deviceInfo->connectors.size()) {
            ConnectorInfo &conn = m_deviceInfo->connectors[row];
            bool fieldChanged = false;
            switch (col) {
                case 0: if (conn.moduleType != newValue) { conn.moduleType = newValue; fieldChanged = true; } break;
                case 1: if (conn.interface != newValue) { conn.interface = newValue; fieldChanged = true; } break;
                case 3: if (conn.connectorId != newValue) { conn.connectorId = newValue; fieldChanged = true; } break;
                case 4: if (conn.hostpath != newValue) { conn.hostpath = newValue; fieldChanged = true; } break;
                case 5: if (conn.interfaceName != newValue) { conn.interfaceName = newValue; fieldChanged = true; } break;
            }
            if (fieldChanged) {
                conn.isModified = true;
                QModelIndex index = createIndex(row, col);
                emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
            }
        }
    }
private:
    DeviceInfo *m_deviceInfo = nullptr;
};

class DeviceInfoModel : public QAbstractTableModel {
    Q_OBJECT
    friend class TextEditDelegate;
public:
    explicit DeviceInfoModel(QObject *parent = nullptr) : QAbstractTableModel(parent) {}
    void setData(DeviceInfo *devInfo) {
        beginResetModel();
        m_deviceInfo = devInfo;
        m_fields.clear();
        m_fields << "Type Name" << "Description" << "Vendor" << "Order Number" << "Image";
        m_values.clear();
        if (m_deviceInfo) {
            m_values << m_deviceInfo->typeName << m_deviceInfo->typeDescription << m_deviceInfo->vendor
                     << m_deviceInfo->orderNumber << m_deviceInfo->image;
        }
        endResetModel();
    }
    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid()) return 0;
        return m_fields.size();
    }
    int columnCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid()) return 0;
        return 2;
    }
    QVariant data(const QModelIndex &index, int role) const override {
        if (!index.isValid() || index.row() >= m_fields.size() || index.column() >= 2 || !m_deviceInfo)
            return QVariant();
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            if (index.column() == 0) return m_fields[index.row()];
            if (index.column() == 1) return m_values[index.row()];
        }
        if (role == DeviceInfoFieldRole && index.column() == 1) {
            return index.row();
        }
        return QVariant();
    }
    // setData теперь НЕ обрабатывает изменение, только делегат
    bool setData(const QModelIndex &index, const QVariant &value, int role) override {
        Q_UNUSED(index);
        Q_UNUSED(value);
        Q_UNUSED(role);
        return false; // Всё делегат
    }
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            if (section == 0) return "Field";
            if (section == 1) return "Value";
        }
        return QAbstractTableModel::headerData(section, orientation, role);
    }
    Qt::ItemFlags flags(const QModelIndex &index) const override {
        Qt::ItemFlags flags = QAbstractTableModel::flags(index);
        if (index.isValid() && index.column() == 1) {
            flags |= Qt::ItemIsEditable;
        }
        return flags;
    }
    void updateField(int fieldIdx, const QString &newValue) {
        if (fieldIdx >= 0 && fieldIdx < m_fields.size()) {
            DeviceInfo tempInfo = *m_deviceInfo;
            tempInfo.updateField(fieldIdx, newValue);
            if (tempInfo.isModified) {
                *m_deviceInfo = tempInfo;
                m_values[fieldIdx] = newValue;
                QModelIndex index = createIndex(fieldIdx, 1);
                emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
            }
        }
    }
private:
    DeviceInfo *m_deviceInfo = nullptr;
    QStringList m_fields;
    QStringList m_values;
};

class FilesModel : public QAbstractTableModel {
    Q_OBJECT
    friend class TextEditDelegate;
public:
    explicit FilesModel(QObject *parent = nullptr) : QAbstractTableModel(parent) {}
    void setData(DeviceInfo *devInfo) {
        beginResetModel();
        m_deviceInfo = devInfo;
        endResetModel();
    }
    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid() || !m_deviceInfo) return 0;
        return m_deviceInfo->files.size();
    }
    int columnCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid()) return 0;
        return 3;
    }
    QVariant data(const QModelIndex &index, int role) const override {
        if (!index.isValid() || index.row() >= m_deviceInfo->files.size() || index.column() >= 3 || !m_deviceInfo)
            return QVariant();
        const auto &file = m_deviceInfo->files[index.row()];
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            switch (index.column()) {
                case 0: return file.fileref;
                case 1: return file.identifier;
                case 2: return file.localFile;
                default: return QVariant();
            }
        }
        if (role == Qt::UserRole) return index.row();
        return QVariant();
    }
    // setData теперь НЕ обрабатывает изменение, только делегат
    bool setData(const QModelIndex &index, const QVariant &value, int role) override {
        Q_UNUSED(index);
        Q_UNUSED(value);
        Q_UNUSED(role);
        return false; // Всё делегат
    }
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            QStringList headers = {"Fileref", "Identifier", "LocalFile"};
            if (section < headers.size()) return headers[section];
        }
        return QAbstractTableModel::headerData(section, orientation, role);
    }
    Qt::ItemFlags flags(const QModelIndex &index) const override {
        Qt::ItemFlags flags = QAbstractTableModel::flags(index);
        if (index.isValid()) {
            flags |= Qt::ItemIsEditable;
        }
        return flags;
    }
    void updateField(int row, int col, const QString &newValue) {
        if (row >= 0 && row < m_deviceInfo->files.size()) {
            auto &file = m_deviceInfo->files[row];
            bool fieldChanged = false;
            switch (col) {
                case 0: if (file.fileref != newValue) { file.fileref = newValue; fieldChanged = true; } break;
                case 1: if (file.identifier != newValue) { file.identifier = newValue; fieldChanged = true; } break;
                case 2: if (file.localFile != newValue) { file.localFile = newValue; fieldChanged = true; } break;
            }
            if (fieldChanged) {
                file.isModified = true;
                m_deviceInfo->isModified = true;
                QModelIndex index = createIndex(row, col);
                emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
            }
        }
    }
private:
    DeviceInfo *m_deviceInfo = nullptr;
};

class DeviceIdentificationModel : public QAbstractTableModel {
    Q_OBJECT
    friend class TextEditDelegate;
public:
    explicit DeviceIdentificationModel(QObject *parent = nullptr) : QAbstractTableModel(parent) {}
    void setData(DeviceInfo *devInfo) {
        beginResetModel();
        m_deviceInfo = devInfo;
        m_fields.clear();
        m_fields << "Type" << "Id" << "Version";
        m_values.clear();
        if (m_deviceInfo) {
            m_values << m_deviceInfo->deviceIdentification.type << m_deviceInfo->deviceIdentification.id
                     << m_deviceInfo->deviceIdentification.version;
        }
        endResetModel();
    }
    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid()) return 0;
        return m_fields.size();
    }
    int columnCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid()) return 0;
        return 2;
    }
    QVariant data(const QModelIndex &index, int role) const override {
        if (!index.isValid() || index.row() >= m_fields.size() || index.column() >= 2 || !m_deviceInfo)
            return QVariant();
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            if (index.column() == 0) return m_fields[index.row()];
            if (index.column() == 1) return m_values[index.row()];
        }
        if (role == Qt::UserRole && index.column() == 1) return m_fields[index.row()];
        return QVariant();
    }
    // setData теперь НЕ обрабатывает изменение, только делегат
    bool setData(const QModelIndex &index, const QVariant &value, int role) override {
        Q_UNUSED(index);
        Q_UNUSED(value);
        Q_UNUSED(role);
        return false; // Всё делегат
    }
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            if (section == 0) return "Field";
            if (section == 1) return "Value";
        }
        return QAbstractTableModel::headerData(section, orientation, role);
    }
    Qt::ItemFlags flags(const QModelIndex &index) const override {
        Qt::ItemFlags flags = QAbstractTableModel::flags(index);
        if (index.isValid() && index.column() == 1) {
            flags |= Qt::ItemIsEditable;
        }
        return flags;
    }
    void updateField(int fieldIdx, const QString &newValue) {
        bool fieldChanged = false;
        QString fieldName = m_fields[fieldIdx];
        if (fieldName == "Type") {
            if (m_deviceInfo->deviceIdentification.type != newValue) {
                m_deviceInfo->deviceIdentification.type = newValue;
                fieldChanged = true;
            }
        } else if (fieldName == "Id") {
            if (m_deviceInfo->deviceIdentification.id != newValue) {
                m_deviceInfo->deviceIdentification.id = newValue;
                fieldChanged = true;
            }
        } else if (fieldName == "Version") {
            if (m_deviceInfo->deviceIdentification.version != newValue) {
                m_deviceInfo->deviceIdentification.version = newValue;
                fieldChanged = true;
            }
        }
        if (fieldChanged) {
            m_deviceInfo->deviceIdentification.isModified = true;
            m_deviceInfo->isModified = true;
            m_values[fieldIdx] = newValue;
            QModelIndex index = createIndex(fieldIdx, 1);
            emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
        }
    }
private:
    DeviceInfo *m_deviceInfo = nullptr;
    QStringList m_fields;
    QStringList m_values;
};

class MyComboBox : public QComboBox
{
    Q_OBJECT

public:
    explicit MyComboBox(QWidget *parent = nullptr) : QComboBox(parent)
    {  }

protected:
//    void showPopup() override
//    {
//        QComboBox::showPopup(); // Базовая реализация

//        QWidget *popup = this->findChild<QFrame*>();

//        if (popup)
//            popup->move(popup->x(), popup->y()+popup->height());

//    }

//    void showPopup() override {
//        QComboBox::showPopup();
//        QWidget* popup = this->findChild<QFrame*>();
//        if (popup) {
//            QPoint globalPos = this->mapToGlobal(this->rect().bottomLeft());
////            // Сдвигаем popup вверх, если нужно
//            popup->move(globalPos.x(), globalPos.y() - popup->height());
//        }
//    }

    void showPopup() override
    {
        // Сначала вызываем базовую реализацию, чтобы создать и показать всплывающий виджет
        QComboBox::showPopup();

        // Получаем указатель на сам всплывающий виджет (это QFrame, который является родителем QListView)
        // Примечание: это обходной путь, так как внутренние компоненты могут меняться в разных версиях Qt
        QWidget *popup = this->findChild<QFrame*>();
        if (popup)
        {
            // Определяем глобальную позицию, где должен появиться список:
            // в той же глобальной координате X, что и комбобокс,
            // но по координате Y, смещенной вниз на высоту комбобокса.
            QPoint globalPos = this->mapToGlobal(QPoint(0, this->height()));

            // Перемещаем всплывающий виджет в нужную позицию
            popup->move(globalPos.x(), globalPos.y());
        }
    }
};

class RoleComboBoxDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit RoleComboBoxDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        if (index.column() == 2) { // Role column

            MyComboBox* editor = new MyComboBox(parent);            

            //QComboBox *editor = new QComboBox(parent);
            editor->addItems({"child", "parent"});
            return editor;
        }
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
    void setEditorData(QWidget *editor, const QModelIndex &index) const override {
        if (index.column() == 2) {
            QComboBox *comboBox = qobject_cast<QComboBox*>(editor);
            QString value = index.model()->data(index, Qt::EditRole).toString();
            comboBox->setCurrentText(value);
        } else {
            QStyledItemDelegate::setEditorData(editor, index);
        }
    }
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
        if (index.column() == 2) {
            QComboBox *comboBox = qobject_cast<QComboBox*>(editor);
            QString value = comboBox->currentText();
            ConnectorsModel *connModel = qobject_cast<ConnectorsModel*>(model);
            if (connModel) {
                connModel->updateRole(index.row(), value);
            }
        } else {
            QStyledItemDelegate::setModelData(editor, model, index);
        }
    }
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        if (index.column() == 2) {
            editor->setGeometry(option.rect);
        } else {
            QStyledItemDelegate::updateEditorGeometry(editor, option, index);
        }
    }
};

class TextEditDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit TextEditDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
        QString value = qobject_cast<QLineEdit*>(editor)->text();
        // Проверим, к какой модели мы обращаемся
        ConnectorsModel *connModel = qobject_cast<ConnectorsModel*>(model);
        if (connModel && index.column() != 2) { // Не Role
            connModel->updateField(index.row(), index.column(), value);
            return;
        }
        DeviceInfoModel *devInfoModel = qobject_cast<DeviceInfoModel*>(model);
        if (devInfoModel && index.column() == 1) { // Только Value столбец
            devInfoModel->updateField(index.row(), value);
            return;
        }
        FilesModel *filesModel = qobject_cast<FilesModel*>(model);
        if (filesModel) {
            filesModel->updateField(index.row(), index.column(), value);
            return;
        }
        DeviceIdentificationModel *devIdModel = qobject_cast<DeviceIdentificationModel*>(model);
        if (devIdModel && index.column() == 1) { // Только Value столбец
            devIdModel->updateField(index.row(), value);
            return;
        }
        ParametersModel *paramsModel = qobject_cast<ParametersModel*>(model);
        if (paramsModel && index.column() != 0) { // Не ID
            DeviceParameter *param = paramsModel->getParameter(index);
            if (param) {
                bool fieldChanged = false;
                switch (index.column()) {
                    case 1: if (param->name != value) { param->name = value; fieldChanged = true; } break;
                    case 2: if (param->type != value) { param->type = value; fieldChanged = true; } break;
                    case 3: if (param->defaultValue != value) { param->defaultValue = value; fieldChanged = true; } break;
                    case 4: if (param->description != value) { param->description = value; fieldChanged = true; } break;
                }
                if (fieldChanged) {
                    param->isModified = true;
                    if (paramsModel->m_allParams[index.row()].isFromHostSet) {
                        paramsModel->m_deviceInfo->connectors[paramsModel->m_allParams[index.row()].connectorIndex].isModified = true;
                    }
                    emit model->dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
                }
            }
            return;
        }
        TypesModel *typesModel = qobject_cast<TypesModel*>(model);
        if (typesModel && index.column() >= 3 && index.column() <= 4) {
            auto &row = typesModel->m_flatList[index.row()];
            if (!row.componentOrEnumId.isEmpty()) {
                auto typeIt = typesModel->m_typesMap->find(row.typeName);
                if (typeIt != typesModel->m_typesMap->end()) {
                    TypeInfo &typeInfo = const_cast<TypeInfo&>(typeIt.value());
                    if (typeInfo.type == "StructType" || typeInfo.type == "BitfieldType") {
                        for (auto &comp : typeInfo.components) {
                            if (comp.identifier == row.componentOrEnumId) {
                                bool fieldChanged = false;
                                if (index.column() == 3) {
                                    if (comp.defaultValue != value) {
                                        comp.defaultValue = value;
                                        comp.isModified = true;
                                        typeInfo.isModified = true;
                                        row.defaultValue = value;
                                        fieldChanged = true;
                                    }
                                } else if (index.column() == 4) {
                                    QString val = value;
                                    int sepIndex = val.indexOf(" / ");
                                    QString visName = (sepIndex != -1) ? val.left(sepIndex) : val.trimmed();
                                    QString unit = (sepIndex != -1) ? val.mid(sepIndex + 3) : "";
                                    bool nameChanged = false, unitChanged = false;
                                    if (comp.visibleName != visName) { comp.visibleName = visName; nameChanged = true; }
                                    if (comp.unit != unit) { comp.unit = unit; unitChanged = true; }
                                    if (nameChanged || unitChanged) {
                                        comp.isModified = true;
                                        typeInfo.isModified = true;
                                        row.visibleNameOrUnit = val;
                                        fieldChanged = true;
                                    }
                                }
                                if (fieldChanged) {
                                    emit model->dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
                                }
                                break;
                            }
                        }
                    }
                }
            }
            return;
        }
        // Если модель неизвестна или столбец не редактируемый, вызвать базовый метод
        QStyledItemDelegate::setModelData(editor, model, index);
    }
};

class AttributesPopup : public QWidget {
    Q_OBJECT
public:
    explicit AttributesPopup(QWidget *parent = nullptr) : QWidget(parent) {

        setWindowFlags(Qt::Popup);
        layout = new QVBoxLayout(this);

        titleLabel = new QLabel("Атрибуты", this);
        QFont titleFont = titleLabel->font();
        titleFont.setBold(true);
        titleLabel->setFont(titleFont);
        titleLabel->setAlignment(Qt::AlignCenter);

        formLayout = new QFormLayout();
        buttonLayout = new QHBoxLayout();
        closeButton = new QPushButton(tr("Закрыть"));
        buttonLayout->addStretch();
        buttonLayout->addWidget(closeButton);

        layout->addWidget(titleLabel);
        layout->addLayout(formLayout);
        layout->addLayout(buttonLayout);
        setLayout(layout);
        connect(closeButton, &QPushButton::clicked, this, &QWidget::hide);
    }
    void showForParameter(const DeviceParameter &param, QAbstractItemView *view, const QModelIndex &index) {

        parametersView = view;
        currentParamIndex = index;
        QLayoutItem *child;
        while ((child = formLayout->takeAt(0)) != nullptr) {
            delete child->widget();
            delete child;
        }
        if (param.attributesMap.isEmpty()) {
            hide();
            return;
        }
        for (auto it = param.attributesMap.constBegin(); it != param.attributesMap.constEnd(); ++it) {
            QLabel *nameLabel = new QLabel(it.key());
            QComboBox *valueComboBox = new QComboBox();
            QString attrName = it.key();
            QString attrValue = it.value();
            if (attrName == "channel") {
                valueComboBox->addItems({"none", "input", "output"});
            } else if (attrName == "download" || attrName == "functional") {
                valueComboBox->addItems({"true", "false"});
            } else if (attrName == "offlineaccess" || attrName == "onlineaccess") {
                valueComboBox->addItems({"read", "readwrite"});
            } else {
                valueComboBox->addItems({attrValue});
                valueComboBox->setEditable(true);
            }
            valueComboBox->setCurrentText(attrValue);
            valueComboBox->setProperty("attr_name", attrName);
            connect(valueComboBox, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
                    this, &AttributesPopup::onAttributeValueComboBoxChanged);
            formLayout->addRow(nameLabel, valueComboBox);
        }
        adjustSize();
        if (parametersView) {
            QPoint globalPos = parametersView->mapToGlobal(parametersView->visualRect(index).bottomLeft());
            move(globalPos);
        } else {
            move(QCursor::pos() + QPoint(10, 10));
        }
        show();
    }
signals:
    void attributeChanged(const QModelIndex &paramIndex, const QString &attrName, const QString &newValue);
private slots:
    void onAttributeValueComboBoxChanged(const QString &newText) {
        QComboBox *senderComboBox = qobject_cast<QComboBox*>(sender());
        if (!senderComboBox) return;
        QString attrName = senderComboBox->property("attr_name").toString();
        if (attrName.isEmpty()) return;
        emit attributeChanged(currentParamIndex, attrName, newText);
    }
private:
    QVBoxLayout *layout;
    QLabel *titleLabel;
    QFormLayout *formLayout;
    QHBoxLayout *buttonLayout;
    QPushButton *closeButton;
    QAbstractItemView *parametersView = nullptr;
    QModelIndex currentParamIndex;
};
