#include "xmldevdesc.h"
#include <QDebug>
#include <QFile>
#include <QRegularExpressionMatchIterator>

void parseEnum(QXmlStreamReader &reader, QList<TypeInfo::EnumValue> &enumValues) {
    Q_ASSERT(reader.name() == "Enum");
    TypeInfo::EnumValue enumVal;
    enumVal.identifier = reader.attributes().value("identifier").toString();
    while (!reader.atEnd()) {
        reader.readNextStartElement();
        if (reader.tokenType() == QXmlStreamReader::StartElement) {
            if (reader.name() == "Value") {
                QString valStr = reader.readElementText();
                bool ok;
                enumVal.value = valStr.toInt(&ok);
                if (!ok) { qWarning() << "Could not parse Enum Value:" << valStr; }
            } else if (reader.name() == "VisibleName") {
                enumVal.visibleName = reader.attributes().value("name").toString();
                reader.readElementText();
            } else {
                reader.skipCurrentElement();
            }
        } else if (reader.tokenType() == QXmlStreamReader::EndElement && reader.name() == "Enum") {
            break;
        }
    }
    enumValues.append(enumVal);
}
void parseComponent(QXmlStreamReader &reader, QList<TypeInfo::Component> &components, int currentComponentIndex) {
    Q_ASSERT(reader.name() == "Component");
    TypeInfo::Component comp;
    comp.identifier = reader.attributes().value("identifier").toString();
    comp.type = reader.attributes().value("type").toString();
    comp.originalIndex = currentComponentIndex;
    while (!reader.atEnd()) {
        reader.readNextStartElement();
        if (reader.tokenType() == QXmlStreamReader::StartElement) {
            if (reader.name() == "Default") {
                comp.defaultValue = reader.readElementText();
            } else if (reader.name() == "VisibleName") {
                comp.visibleName = reader.attributes().value("name").toString();
                reader.readElementText();
            } else if (reader.name() == "Unit") {
                comp.unit = reader.attributes().value("name").toString();
                reader.readElementText();
            } else {
                reader.skipCurrentElement();
            }
        } else if (reader.tokenType() == QXmlStreamReader::EndElement && reader.name() == "Component") {
            break;
        }
    }
    components.append(comp);
}
void parseEnumType(QXmlStreamReader &reader, TypeInfo &typeInfo) {
    Q_ASSERT(reader.name() == "EnumType");
    typeInfo.baseType = reader.attributes().value("basetype").toString();
    typeInfo.name = reader.attributes().value("name").toString();
    typeInfo.type = "EnumType";
    while (!reader.atEnd()) {
        reader.readNextStartElement();
        if (reader.tokenType() == QXmlStreamReader::StartElement) {
            if (reader.name() == "Enum") {
                parseEnum(reader, typeInfo.enumValues);
            } else {
                reader.skipCurrentElement();
            }
        } else if (reader.tokenType() == QXmlStreamReader::EndElement && reader.name() == "EnumType") {
            break;
        }
    }
}
void parseRangeType(QXmlStreamReader &reader, TypeInfo &typeInfo) {
    Q_ASSERT(reader.name() == "RangeType");
    typeInfo.baseType = reader.attributes().value("basetype").toString();
    typeInfo.name = reader.attributes().value("name").toString();
    typeInfo.type = "RangeType";
    while (!reader.atEnd()) {
        reader.readNextStartElement();
        if (reader.tokenType() == QXmlStreamReader::StartElement) {
            if (reader.name() == "Min") {
                typeInfo.minVal = reader.readElementText();
            } else if (reader.name() == "Max") {
                typeInfo.maxVal = reader.readElementText();
            } else if (reader.name() == "Default") {
                typeInfo.defaultVal = reader.readElementText();
            } else {
                reader.skipCurrentElement();
            }
        } else if (reader.tokenType() == QXmlStreamReader::EndElement && reader.name() == "RangeType") {
            break;
        }
    }
}
void parseStructType(QXmlStreamReader &reader, TypeInfo &typeInfo) {
    Q_ASSERT(reader.name() == "StructType");
    typeInfo.name = reader.attributes().value("name").toString();
    typeInfo.type = "StructType";
    int componentIndex = 0;
    while (!reader.atEnd()) {
        reader.readNextStartElement();
        if (reader.tokenType() == QXmlStreamReader::StartElement) {
            if (reader.name() == "Component") {
                parseComponent(reader, typeInfo.components, componentIndex);
                componentIndex++;
            } else {
                reader.skipCurrentElement();
            }
        } else if (reader.tokenType() == QXmlStreamReader::EndElement && reader.name() == "StructType") {
            break;
        }
    }
}
void parseBitfieldType(QXmlStreamReader &reader, TypeInfo &typeInfo) {
    Q_ASSERT(reader.name() == "BitfieldType");
    typeInfo.baseType = reader.attributes().value("basetype").toString();
    typeInfo.name = reader.attributes().value("name").toString();
    typeInfo.type = "BitfieldType";
    int componentIndex = 0;
    while (!reader.atEnd()) {
        reader.readNextStartElement();
        if (reader.tokenType() == QXmlStreamReader::StartElement) {
            if (reader.name() == "Component") {
                parseComponent(reader, typeInfo.components, componentIndex);
                componentIndex++;
            } else {
                reader.skipCurrentElement();
            }
        } else if (reader.tokenType() == QXmlStreamReader::EndElement && reader.name() == "BitfieldType") {
            break;
        }
    }
}
QMap<QString, QString> parseAttributesString(const QString &attributesString) {
    QMap<QString, QString> attrMap;
    if (!attributesString.isEmpty()) {
        QRegularExpression re(R"((\w+)\s*=\s*["']([^"']*)["'])");
        QRegularExpressionMatchIterator it = re.globalMatch(attributesString);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            QString attrName = match.captured(1);
            QString attrValue = match.captured(2);
            attrMap[attrName] = attrValue;
        }
    }
    return attrMap;
}

void parseParameter(QXmlStreamReader &reader, QList<DeviceParameter> &parameters, const QString &setName, int currentParamIndex) {
    Q_ASSERT(reader.name() == "Parameter");
    DeviceParameter param;
    param.id = reader.attributes().value("ParameterId").toString();
    param.type = reader.attributes().value("type").toString();
    param.originalIndex = currentParamIndex;
    param.isFromHostParameterSet = (setName == "HostParameterSet");
    QXmlStreamAttributes paramAttrs = reader.attributes();
    for (int i = 0; i < paramAttrs.size(); ++i) {
        QXmlStreamAttribute attr = paramAttrs.at(i);
        QString attrName = attr.name().toString();
        QString attrValue = attr.value().toString();
        if (attrName == "channel" || attrName == "download" || attrName == "functional" ||
            attrName == "offlineaccess" || attrName == "onlineaccess") {
            param.attributesMap[attrName] = attrValue;
        }
    }
    while (reader.readNextStartElement()) {
        if (reader.name() == "Attributes") {
            QXmlStreamAttributes attrs = reader.attributes();
            if (attrs.size() > 0) {
                for (int i = 0; i < attrs.size(); ++i) {
                    QXmlStreamAttribute attr = attrs.at(i);
                    QString attrName = attr.name().toString();
                    QString attrValue = attr.value().toString();
                    param.attributesMap[attrName] = attrValue;
                }
                reader.skipCurrentElement();
            } else {
                reader.skipCurrentElement();
            }
        } else if (reader.name() == "Name") {
            // --- Читаем атрибуты Name ---
            QXmlStreamAttributes nameAttrs = reader.attributes();
            for (int i = 0; i < nameAttrs.size(); ++i) {
                QXmlStreamAttribute attr = nameAttrs.at(i);
                QString attrName = attr.name().toString();
                QString attrValue = attr.value().toString();
                param.nameAttributesMap[attrName] = attrValue;
            }
            // --- /Читаем атрибуты Name ---
            param.name = reader.readElementText();
        } else if (reader.name() == "Description") {
            // --- Читаем атрибуты Description ---
            QXmlStreamAttributes descAttrs = reader.attributes();
            for (int i = 0; i < descAttrs.size(); ++i) {
                QXmlStreamAttribute attr = descAttrs.at(i);
                QString attrName = attr.name().toString();
                QString attrValue = attr.value().toString();
                param.descriptionAttributesMap[attrName] = attrValue;
            }
            // --- /Читаем атрибуты Description ---
            param.description = reader.readElementText();
        } else if (reader.name() == "Default") {
            param.defaultValue = reader.readElementText();
        } else {
            reader.skipCurrentElement();
        }
    }
    parameters.append(param);
}

// --- Вспомогательная функция для парсинга из существующего QDomDocument ---
QPair<QDomDocument, QPair<QMap<QString, TypeInfo>, DeviceInfo>>
parseFromQDomDoc(const QDomDocument &doc) {
    // Работаем с копией, чтобы не изменять оригинал
    QDomDocument domDoc = doc;
    QDomElement root = domDoc.documentElement();
    if (root.tagName() != "DeviceDescription") {
        qCritical() << "Not a DeviceDescription document.";
        return qMakePair(QDomDocument(), qMakePair(QMap<QString, TypeInfo>(), DeviceInfo()));
    }

    QMap<QString, TypeInfo> typesMap;
    DeviceInfo devInfo;

    QDomNodeList typesList = domDoc.elementsByTagName("Types");
    if (!typesList.isEmpty()) {
        QDomElement typesElement = typesList.at(0).toElement();
        QDomNodeList typeChildren = typesElement.childNodes();
        for (int i = 0; i < typeChildren.size(); ++i) {
            QDomNode node = typeChildren.at(i);
            if (node.isElement()) {
                QDomElement typeEl = node.toElement();
                QString typeName = typeEl.attribute("name");
                TypeInfo typeInfo;
                typeInfo.name = typeName;
                typeInfo.originalIndex = typesMap.size();
                if (typeEl.tagName() == "EnumType") {
                    typeInfo.type = "EnumType";
                    typeInfo.baseType = typeEl.attribute("basetype");
                    QDomNodeList enumList = typeEl.elementsByTagName("Enum");
                    for (int j = 0; j < enumList.size(); ++j) {
                        TypeInfo::EnumValue enumVal;
                        QDomElement enumEl = enumList.at(j).toElement();
                        enumVal.identifier = enumEl.attribute("identifier");
                        QDomNodeList valueList = enumEl.elementsByTagName("Value");
                        if (!valueList.isEmpty()) {
                             QString valStr = valueList.at(0).toElement().text();
                             bool ok; int val = valStr.toInt(&ok);
                             if (ok) enumVal.value = val;
                        }
                        QDomNodeList visibleNameList = enumEl.elementsByTagName("VisibleName");
                        if (!visibleNameList.isEmpty()) {
                            enumVal.visibleName = visibleNameList.at(0).toElement().attribute("name");
                        }
                        typeInfo.enumValues.append(enumVal);
                    }
                } else if (typeEl.tagName() == "RangeType") {
                    typeInfo.type = "RangeType";
                    typeInfo.baseType = typeEl.attribute("basetype");
                    QDomNodeList minList = typeEl.elementsByTagName("Min");
                    QDomNodeList maxList = typeEl.elementsByTagName("Max");
                    QDomNodeList defaultList = typeEl.elementsByTagName("Default");
                    if (!minList.isEmpty()) typeInfo.minVal = minList.at(0).toElement().text();
                    if (!maxList.isEmpty()) typeInfo.maxVal = maxList.at(0).toElement().text();
                    if (!defaultList.isEmpty()) typeInfo.defaultVal = defaultList.at(0).toElement().text();
                } else if (typeEl.tagName() == "StructType") {
                    typeInfo.type = "StructType";
                    QDomNodeList compList = typeEl.elementsByTagName("Component");
                    for (int j = 0; j < compList.size(); ++j) {
                        TypeInfo::Component comp;
                        QDomElement compEl = compList.at(j).toElement();
                        comp.identifier = compEl.attribute("identifier");
                        comp.type = compEl.attribute("type");
                        comp.originalIndex = j;
                        QDomNodeList defaultList = compEl.elementsByTagName("Default");
                        QDomNodeList visibleNameList = compEl.elementsByTagName("VisibleName");
                        QDomNodeList unitList = compEl.elementsByTagName("Unit");
                        if (!defaultList.isEmpty()) comp.defaultValue = defaultList.at(0).toElement().text();
                        if (!visibleNameList.isEmpty()) comp.visibleName = visibleNameList.at(0).toElement().attribute("name");
                        if (!unitList.isEmpty()) comp.unit = unitList.at(0).toElement().attribute("name");
                        typeInfo.components.append(comp);
                    }
                } else if (typeEl.tagName() == "BitfieldType") {
                    typeInfo.type = "BitfieldType";
                    typeInfo.baseType = typeEl.attribute("basetype");
                    QDomNodeList compList = typeEl.elementsByTagName("Component");
                    for (int j = 0; j < compList.size(); ++j) {
                        TypeInfo::Component comp;
                        QDomElement compEl = compList.at(j).toElement();
                        comp.identifier = compEl.attribute("identifier");
                        comp.type = compEl.attribute("type");
                        comp.originalIndex = j;
                        QDomNodeList defaultList = compEl.elementsByTagName("Default");
                        QDomNodeList visibleNameList = compEl.elementsByTagName("VisibleName");
                        QDomNodeList unitList = compEl.elementsByTagName("Unit");
                        if (!defaultList.isEmpty()) comp.defaultValue = defaultList.at(0).toElement().text();
                        if (!visibleNameList.isEmpty()) comp.visibleName = visibleNameList.at(0).toElement().attribute("name");
                        if (!unitList.isEmpty()) comp.unit = unitList.at(0).toElement().attribute("name");
                        typeInfo.components.append(comp);
                    }
                }
                typesMap[typeName] = typeInfo;
            }
        }
    }

    QDomNodeList deviceInfoList = domDoc.elementsByTagName("DeviceInfo");
    if (!deviceInfoList.isEmpty()) {
        QDomElement deviceInfoEl = deviceInfoList.at(0).toElement();
        QDomNodeList nameList = deviceInfoEl.elementsByTagName("Name");
        QDomNodeList descList = deviceInfoEl.elementsByTagName("Description");
        QDomNodeList vendorList = deviceInfoEl.elementsByTagName("Vendor");
        QDomNodeList orderNumList = deviceInfoEl.elementsByTagName("OrderNumber");
        QDomNodeList imageList = deviceInfoEl.elementsByTagName("Image");
        if (!nameList.isEmpty()) devInfo.typeName = nameList.at(0).toElement().text();
        if (!descList.isEmpty()) devInfo.typeDescription = descList.at(0).toElement().text();
        if (!vendorList.isEmpty()) devInfo.vendor = vendorList.at(0).toElement().text();
        if (!orderNumList.isEmpty()) devInfo.orderNumber = orderNumList.at(0).toElement().text();
        if (!imageList.isEmpty()) devInfo.image = imageList.at(0).toElement().text();
    }

    QDomNodeList deviceIdentList = domDoc.elementsByTagName("DeviceIdentification");
    if (!deviceIdentList.isEmpty()) {
        QDomElement deviceIdentEl = deviceIdentList.at(0).toElement();
        QDomNodeList typeList = deviceIdentEl.elementsByTagName("Type");
        QDomNodeList idList = deviceIdentEl.elementsByTagName("Id");
        QDomNodeList versionList = deviceIdentEl.elementsByTagName("Version");
        if (!typeList.isEmpty()) devInfo.deviceIdentification.type = typeList.at(0).toElement().text();
        if (!idList.isEmpty()) devInfo.deviceIdentification.id = idList.at(0).toElement().text();
        if (!versionList.isEmpty()) devInfo.deviceIdentification.version = versionList.at(0).toElement().text();
    }

    QDomNodeList fileLists = domDoc.elementsByTagName("File");
    for (int i = 0; i < fileLists.size(); ++i) {
        QDomElement fileEl = fileLists.at(i).toElement();
        DeviceInfo::File fileEntry;
        fileEntry.fileref = fileEl.attribute("fileref");
        fileEntry.identifier = fileEl.attribute("identifier");
        QDomNodeList localFileList = fileEl.elementsByTagName("LocalFile");
        if (!localFileList.isEmpty()) {
            fileEntry.localFile = localFileList.at(0).toElement().text();
        }
        devInfo.files.append(fileEntry);
    }

    QDomNodeList connectorLists = domDoc.elementsByTagName("Connector");
    for (int i = 0; i < connectorLists.size(); ++i) {
        QDomElement connectorEl = connectorLists.at(i).toElement();
        ConnectorInfo connInfo;
        connInfo.moduleType = connectorEl.attribute("moduleType");
        connInfo.interface = connectorEl.attribute("interface");
        connInfo.role = connectorEl.attribute("role");
        connInfo.connectorId = connectorEl.attribute("connectorId");
        connInfo.hostpath = connectorEl.attribute("hostpath");
        QDomNodeList interfaceNameList = connectorEl.elementsByTagName("InterfaceName");
        if (!interfaceNameList.isEmpty()) {
            connInfo.interfaceName = interfaceNameList.at(0).toElement().text();
        }
        QDomNodeList hostParamSetList = connectorEl.elementsByTagName("HostParameterSet");
        if (!hostParamSetList.isEmpty()) {
            QDomElement hostParamSetEl = hostParamSetList.at(0).toElement();
            QDomNodeList paramList = hostParamSetEl.elementsByTagName("Parameter");
            for (int j = 0; j < paramList.size(); ++j) {
                QDomElement paramEl = paramList.at(j).toElement();
                DeviceParameter param;
                param.id = paramEl.attribute("ParameterId");
                param.type = paramEl.attribute("type");
                param.connectorIndex = i;
                param.isFromHostParameterSet = true;
                param.originalIndex = j;

                QDomNodeList children = paramEl.childNodes();
                for (int k = 0; k < children.size(); ++k) {
                    QDomNode childNode = children.at(k);
                    if (childNode.isElement()) {
                        QDomElement childEl = childNode.toElement();
                        if (childEl.tagName() == "Name") {
                            // --- Читаем атрибуты Name ---
                            QDomNamedNodeMap attrMap = childEl.attributes();
                            for (int l = 0; l < attrMap.count(); ++l) {
                                QDomNode attrNode = attrMap.item(l);
                                if (attrNode.isAttr()) {
                                    QDomAttr attr = attrNode.toAttr();
                                    param.nameAttributesMap[attr.name()] = attr.value();
                                }
                            }
                            // --- /Читаем атрибуты Name ---
                            param.name = childEl.text();
                        } else if (childEl.tagName() == "Description") {
                            // --- Читаем атрибуты Description ---
                            QDomNamedNodeMap attrMap = childEl.attributes();
                            for (int l = 0; l < attrMap.count(); ++l) {
                                QDomNode attrNode = attrMap.item(l);
                                if (attrNode.isAttr()) {
                                    QDomAttr attr = attrNode.toAttr();
                                    param.descriptionAttributesMap[attr.name()] = attr.value();
                                }
                            }
                            // --- /Читаем атрибуты Description ---
                            param.description = childEl.text();
                        } else if (childEl.tagName() == "Default") {
                            param.defaultValue = childEl.text();
                        } else if (childEl.tagName() == "Attributes") {
                            QDomNamedNodeMap attrMap = childEl.attributes();
                            for (int l = 0; l < attrMap.count(); ++l) {
                                QDomNode attrNode = attrMap.item(l);
                                if (attrNode.isAttr()) {
                                    QDomAttr attr = attrNode.toAttr();
                                    param.attributesMap[attr.name()] = attr.value();
                                }
                            }
                        }
                    }
                }
                connInfo.parameters.append(param);
            }
        }
        connInfo.originalIndex = i;
        devInfo.connectors.append(connInfo);
    }

    QDomNodeList deviceParamSetList = domDoc.elementsByTagName("DeviceParameterSet");
    if (!deviceParamSetList.isEmpty()) {
        QDomElement deviceParamSetEl = deviceParamSetList.at(0).toElement();
        QDomNodeList paramList = deviceParamSetEl.elementsByTagName("Parameter");
        for (int i = 0; i < paramList.size(); ++i) {
            QDomElement paramEl = paramList.at(i).toElement();
            DeviceParameter param;
            param.id = paramEl.attribute("ParameterId");
            param.type = paramEl.attribute("type");
            param.isFromHostParameterSet = false;
            param.originalIndex = i;

            QDomNodeList children = paramEl.childNodes();
            for (int k = 0; k < children.size(); ++k) {
                QDomNode childNode = children.at(k);
                if (childNode.isElement()) {
                    QDomElement childEl = childNode.toElement();
                    if (childEl.tagName() == "Name") {
                        // --- Читаем атрибуты Name ---
                        QDomNamedNodeMap attrMap = childEl.attributes();
                        for (int l = 0; l < attrMap.count(); ++l) {
                            QDomNode attrNode = attrMap.item(l);
                            if (attrNode.isAttr()) {
                                QDomAttr attr = attrNode.toAttr();
                                param.nameAttributesMap[attr.name()] = attr.value();
                            }
                        }
                        // --- /Читаем атрибуты Name ---
                        param.name = childEl.text();
                    } else if (childEl.tagName() == "Description") {
                        // --- Читаем атрибуты Description ---
                        QDomNamedNodeMap attrMap = childEl.attributes();
                        for (int l = 0; l < attrMap.count(); ++l) {
                            QDomNode attrNode = attrMap.item(l);
                            if (attrNode.isAttr()) {
                                QDomAttr attr = attrNode.toAttr();
                                param.descriptionAttributesMap[attr.name()] = attr.value();
                            }
                        }
                        // --- /Читаем атрибуты Description ---
                        param.description = childEl.text();
                    } else if (childEl.tagName() == "Default") {
                        param.defaultValue = childEl.text();
                    } else if (childEl.tagName() == "Attributes") {
                        QDomNamedNodeMap attrMap = childEl.attributes();
                        for (int l = 0; l < attrMap.count(); ++l) {
                            QDomNode attrNode = attrMap.item(l);
                            if (attrNode.isAttr()) {
                                QDomAttr attr = attrNode.toAttr();
                                param.attributesMap[attr.name()] = attr.value();
                            }
                        }
                    }
                }
            }
            devInfo.deviceParameters.append(param);
        }
    }

    return qMakePair(domDoc, qMakePair(typesMap, devInfo));
}


QPair<QDomDocument, QPair<QMap<QString, TypeInfo>, DeviceInfo>>
parseXMLFile(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "Cannot open file for parsing:" << fileName << file.errorString();
        return qMakePair(QDomDocument(), qMakePair(QMap<QString, TypeInfo>(), DeviceInfo()));
    }
    QDomDocument doc;
    QString errorMessage;
    int errorLine, errorColumn;
    if (!doc.setContent(&file, false, &errorMessage, &errorLine, &errorColumn)) {
        qCritical() << "Error parsing XML (DOM):" << errorMessage << "at line" << errorLine << ", column" << errorColumn;
        return qMakePair(QDomDocument(), qMakePair(QMap<QString, TypeInfo>(), DeviceInfo()));
    }
    file.seek(0);
    QXmlStreamReader reader(&file);
    QMap<QString, TypeInfo> typesMap;
    DeviceInfo devInfo;
    if (reader.readNextStartElement()) {
        if (reader.name() != "DeviceDescription") {
            qCritical() << "Not a DeviceDescription file.";
            return qMakePair(doc, qMakePair(QMap<QString, TypeInfo>(), DeviceInfo()));
        }
    } else {
        qCritical() << "Could not read root element.";
        return qMakePair(doc, qMakePair(QMap<QString, TypeInfo>(), DeviceInfo()));
    }
    int typeIndex = 0;
    int paramIndex = 0;
    int connectorIndex = 0;
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.tokenType() == QXmlStreamReader::StartElement) {
            if (reader.name() == "Types") {
                while (reader.readNextStartElement()) {
                    TypeInfo typeInfo;
                    typeInfo.originalIndex = typeIndex;
                    if (reader.name() == "EnumType") {
                        parseEnumType(reader, typeInfo);
                        typesMap[typeInfo.name] = typeInfo;
                        typeIndex++;
                    } else if (reader.name() == "RangeType") {
                        parseRangeType(reader, typeInfo);
                        typesMap[typeInfo.name] = typeInfo;
                        typeIndex++;
                    } else if (reader.name() == "StructType") {
                        parseStructType(reader, typeInfo);
                        typesMap[typeInfo.name] = typeInfo;
                        typeIndex++;
                    } else if (reader.name() == "BitfieldType") {
                        parseBitfieldType(reader, typeInfo);
                        typesMap[typeInfo.name] = typeInfo;
                        typeIndex++;
                        qDebug() << "Parsed BitfieldType:" << typeInfo.name;
                    } else {
                        reader.skipCurrentElement();
                    }
                }
            } else if (reader.name() == "File") {
                DeviceInfo::File fileEntry;
                fileEntry.fileref = reader.attributes().value("fileref").toString();
                fileEntry.identifier = reader.attributes().value("identifier").toString();
                while (reader.readNextStartElement()) {
                    if (reader.name() == "LocalFile") {
                        fileEntry.localFile = reader.readElementText();
                    } else {
                        reader.skipCurrentElement();
                    }
                }
                devInfo.files.append(fileEntry);
            } else if (reader.name() == "Device") {
                while (reader.readNextStartElement()) {
                    if (reader.name() == "DeviceIdentification") {
                        while (reader.readNextStartElement()) {
                            if (reader.name() == "Type") {
                                devInfo.deviceIdentification.type = reader.readElementText();
                            } else if (reader.name() == "Id") {
                                devInfo.deviceIdentification.id = reader.readElementText();
                            } else if (reader.name() == "Version") {
                                devInfo.deviceIdentification.version = reader.readElementText();
                            } else {
                                reader.skipCurrentElement();
                            }
                        }
                    } else if (reader.name() == "DeviceInfo") {
                        while (reader.readNextStartElement()) {
                            if (reader.name() == "Name") {
                                devInfo.typeName = reader.readElementText();
                            } else if (reader.name() == "Description") {
                                devInfo.typeDescription = reader.readElementText();
                            } else if (reader.name() == "Vendor") {
                                devInfo.vendor = reader.readElementText();
                            } else if (reader.name() == "OrderNumber") {
                                devInfo.orderNumber = reader.readElementText();
                            } else if (reader.name() == "Image") {
                                devInfo.image = reader.readElementText();
                            } else {
                                reader.skipCurrentElement();
                            }
                        }
                    } else if (reader.name() == "Connector") {
                        ConnectorInfo connInfo;
                        QXmlStreamAttributes attrs = reader.attributes();
                        for (int i = 0; i < attrs.size(); ++i) {
                            QXmlStreamAttribute attr = attrs.at(i);
                            QString attrName = attr.name().toString();
                            QString attrValue = attr.value().toString();
                            if (attrName == "moduleType") {
                                connInfo.moduleType = attrValue;
                            } else if (attrName == "interface") {
                                connInfo.interface = attrValue;
                            } else if (attrName == "role") {
                                connInfo.role = attrValue;
                            } else if (attrName == "connectorId") {
                                connInfo.connectorId = attrValue;
                            } else if (attrName == "hostpath") {
                                connInfo.hostpath = attrValue;
                            }
                        }
                        connInfo.originalIndex = connectorIndex;
                        while (reader.readNextStartElement()) {
                            if (reader.name() == "InterfaceName") {
                                connInfo.interfaceName = reader.readElementText();
                            } else if (reader.name() == "HostParameterSet") {
                                int hostParamIndex = 0;
                                while (reader.readNextStartElement()) {
                                    if (reader.name() == "Parameter") {
                                        DeviceParameter param;
                                        param.connectorIndex = connectorIndex;
                                        parseParameter(reader, connInfo.parameters, "HostParameterSet", hostParamIndex);
                                        hostParamIndex++;
                                    } else {
                                        reader.skipCurrentElement();
                                    }
                                }
                            } else {
                                reader.skipCurrentElement();
                            }
                        }
                        devInfo.connectors.append(connInfo);
                        connectorIndex++;
                    } else if (reader.name() == "DeviceParameterSet") {
                        while (reader.readNextStartElement()) {
                            if (reader.name() == "Parameter") {
                                parseParameter(reader, devInfo.deviceParameters, "DeviceParameterSet", paramIndex);
                                paramIndex++;
                            } else {
                                reader.skipCurrentElement();
                            }
                        }
                    } else {
                        reader.skipCurrentElement();
                    }
                }
            } else {
                reader.skipCurrentElement();
            }
        } else if (reader.tokenType() == QXmlStreamReader::Invalid) {
            qCritical() << "XML Error:" << reader.errorString();
            return qMakePair(doc, qMakePair(QMap<QString, TypeInfo>(), DeviceInfo()));
        }
    }
    if (reader.hasError()) {
        qCritical() << "Error occurred while parsing (Stream):" << reader.errorString();
        return qMakePair(doc, qMakePair(QMap<QString, TypeInfo>(), DeviceInfo()));
    }
    return qMakePair(doc, qMakePair(typesMap, devInfo));
}

void updateAttributesElement(QDomElement &attributesEl, const QMap<QString, QString> &attributesMap) {
    if (attributesEl.isNull()) return;
    QStringList attributeNames;
    QDomNamedNodeMap attrMap = attributesEl.attributes();
    for (int i = 0; i < attrMap.count(); ++i) {
        QDomNode attrNode = attrMap.item(i);
        if (attrNode.isAttr()) {
            attributeNames << attrNode.nodeName();
        }
    }
    for (const QString &name : attributeNames) {
        attributesEl.removeAttribute(name);
    }
    for (auto it = attributesMap.constBegin(); it != attributesMap.constEnd(); ++it) {
        attributesEl.setAttribute(it.key(), it.value());
    }
}


bool saveXMLFile(const QString &fileName, QDomDocument &doc, const QMap<QString, TypeInfo> &types, const DeviceInfo &devInfo) {
    QDomElement root = doc.documentElement();
    if (root.tagName() != "DeviceDescription") {
        qCritical() << "Root element is not DeviceDescription, cannot save.";
        return false;
    }
    QDomNodeList typeLists = root.elementsByTagName("Types");
    if (!typeLists.isEmpty()) {
        QDomElement typesElement = typeLists.at(0).toElement();
        if (!typesElement.isNull()) {
            QDomNodeList typeChildren = typesElement.childNodes();
            for (int i = 0; i < typeChildren.size(); ++i) {
                QDomNode node = typeChildren.at(i);
                if (node.isElement()) {
                    QDomElement typeEl = node.toElement();
                    QString typeName = typeEl.attribute("name");
                    if (types.contains(typeName)) {
                        const TypeInfo &typeInfo = types[typeName];
                        // --- Добавлен RangeType ---
                        if (typeInfo.isModified && (typeInfo.type == "StructType" || typeInfo.type == "BitfieldType" || typeInfo.type == "RangeType")) {
                            if (typeInfo.type == "StructType" || typeInfo.type == "BitfieldType") {
                                QDomNodeList compList = typeEl.elementsByTagName("Component");
                                for (int j = 0; j < compList.size() && j < typeInfo.components.size(); ++j) {
                                    QDomElement compEl = compList.at(j).toElement();
                                    const TypeInfo::Component &comp = typeInfo.components[j];
                                    if (comp.isModified) {
                                        QDomNodeList defaultList = compEl.elementsByTagName("Default");
                                        if (!defaultList.isEmpty()) {
                                            QDomElement defaultEl = defaultList.at(0).toElement();
                                            QDomNode firstChild = defaultEl.firstChild();
                                            if (firstChild.isText()) {
                                                firstChild.setNodeValue(comp.defaultValue);
                                            } else {
                                                QDomText newText = doc.createTextNode(comp.defaultValue);
                                                defaultEl.appendChild(newText);
                                            }
                                        } else {
                                            QDomElement defaultEl = doc.createElement("Default");
                                            QDomText defaultText = doc.createTextNode(comp.defaultValue);
                                            defaultEl.appendChild(defaultText);
                                            compEl.appendChild(defaultEl);
                                        }
                                        QDomNodeList visibleNameList = compEl.elementsByTagName("VisibleName");
                                        if (!visibleNameList.isEmpty()) {
                                            QDomElement visibleNameEl = visibleNameList.at(0).toElement();
                                            if (visibleNameEl.attribute("name") != comp.visibleName) {
                                                visibleNameEl.setAttribute("name", comp.visibleName);
                                            }
                                        }
                                        QDomNodeList unitList = compEl.elementsByTagName("Unit");
                                        if (!unitList.isEmpty()) {
                                            QDomElement unitEl = unitList.at(0).toElement();
                                            if (unitEl.attribute("name") != comp.unit) {
                                                unitEl.setAttribute("name", comp.unit);
                                            }
                                        }
                                    }
                                }
                            } else if (typeInfo.type == "RangeType") { // --- Обработка RangeType ---
                                // Обновляем Min, Max, Default
                                QDomNodeList minList = typeEl.elementsByTagName("Min");
                                QDomNodeList maxList = typeEl.elementsByTagName("Max");
                                QDomNodeList defaultList = typeEl.elementsByTagName("Default");

                                if (!minList.isEmpty() && minList.at(0).firstChild().nodeValue() != typeInfo.minVal) {
                                    minList.at(0).firstChild().setNodeValue(typeInfo.minVal);
                                }
                                if (!maxList.isEmpty() && maxList.at(0).firstChild().nodeValue() != typeInfo.maxVal) {
                                    maxList.at(0).firstChild().setNodeValue(typeInfo.maxVal);
                                }
                                if (!defaultList.isEmpty() && defaultList.at(0).firstChild().nodeValue() != typeInfo.defaultVal) {
                                    defaultList.at(0).firstChild().setNodeValue(typeInfo.defaultVal);
                                }
                            }
                            // --- /Обработка RangeType ---
                        }
                    }
                }
            }
        }
    }
    // ... (остальная часть функции saveXMLFile без изменений) ...
    QDomNodeList deviceInfoLists = root.elementsByTagName("DeviceInfo");
    if (!deviceInfoLists.isEmpty()) {
        QDomElement deviceInfoElement = deviceInfoLists.at(0).toElement();
        if (!deviceInfoElement.isNull() && devInfo.isModified) {
            QDomNodeList nameList = deviceInfoElement.elementsByTagName("Name");
            QDomNodeList descList = deviceInfoElement.elementsByTagName("Description");
            QDomNodeList vendorList = deviceInfoElement.elementsByTagName("Vendor");
            QDomNodeList orderNumList = deviceInfoElement.elementsByTagName("OrderNumber");
            QDomNodeList imageList = deviceInfoElement.elementsByTagName("Image");
            if (!nameList.isEmpty()) nameList.at(0).firstChild().setNodeValue(devInfo.typeName);
            if (!descList.isEmpty()) descList.at(0).firstChild().setNodeValue(devInfo.typeDescription);
            if (!vendorList.isEmpty()) vendorList.at(0).firstChild().setNodeValue(devInfo.vendor);
            if (!orderNumList.isEmpty()) orderNumList.at(0).firstChild().setNodeValue(devInfo.orderNumber);
            if (!imageList.isEmpty()) imageList.at(0).firstChild().setNodeValue(devInfo.image);
        }
    }
    QDomNodeList fileLists = root.elementsByTagName("File");
    if (fileLists.size() == devInfo.files.size()) {
        for (int i = 0; i < fileLists.size() && i < devInfo.files.size(); ++i) {
            QDomElement fileEl = fileLists.at(i).toElement();
            const auto &fileEntry = devInfo.files[i];
            if (fileEntry.isModified) {
                if (fileEl.attribute("fileref") != fileEntry.fileref) {
                    fileEl.setAttribute("fileref", fileEntry.fileref);
                }
                if (fileEl.attribute("identifier") != fileEntry.identifier) {
                    fileEl.setAttribute("identifier", fileEntry.identifier);
                }
                QDomNodeList localFileLists = fileEl.elementsByTagName("LocalFile");
                if (!localFileLists.isEmpty()) {
                    localFileLists.at(0).firstChild().setNodeValue(fileEntry.localFile);
                }
            }
        }
    }
    QDomNodeList deviceIdentLists = root.elementsByTagName("DeviceIdentification");
    if (!deviceIdentLists.isEmpty() && devInfo.deviceIdentification.isModified) {
        QDomElement deviceIdentEl = deviceIdentLists.at(0).toElement();
        QDomNodeList typeLists = deviceIdentEl.elementsByTagName("Type");
        QDomNodeList idLists = deviceIdentEl.elementsByTagName("Id");
        QDomNodeList versionLists = deviceIdentEl.elementsByTagName("Version");
        if (!typeLists.isEmpty()) typeLists.at(0).firstChild().setNodeValue(devInfo.deviceIdentification.type);
        if (!idLists.isEmpty()) idLists.at(0).firstChild().setNodeValue(devInfo.deviceIdentification.id);
        if (!versionLists.isEmpty()) versionLists.at(0).firstChild().setNodeValue(devInfo.deviceIdentification.version);
    }
    QDomNodeList deviceParamSetLists = root.elementsByTagName("DeviceParameterSet");
    if (!deviceParamSetLists.isEmpty()) {
        QDomElement deviceParamSetElement = deviceParamSetLists.at(0).toElement();
        if (!deviceParamSetElement.isNull()) {
            QDomNodeList paramList = deviceParamSetElement.elementsByTagName("Parameter");
            // --- Обновление существующих параметров DeviceParameterSet ---
            for (int i = 0; i < paramList.size() && i < devInfo.deviceParameters.size(); ++i) {
                QDomElement paramEl = paramList.at(i).toElement();
                const DeviceParameter &param = devInfo.deviceParameters[i];
                if (param.isModified) {
                    QDomNodeList nameList = paramEl.elementsByTagName("Name");
                    QDomNodeList descList = paramEl.elementsByTagName("Description");
                    QDomNodeList defaultList = paramEl.elementsByTagName("Default");

                    // --- Обновление Name ---
                    if (!nameList.isEmpty()) {
                        QDomElement nameEl = nameList.at(0).toElement();
                        // --- Установка атрибутов Name ---
                        QDomNamedNodeMap existingNameAttrs = nameEl.attributes();
                        QStringList existingNameAttrNames;
                        for (int k = 0; k < existingNameAttrs.count(); ++k) {
                            existingNameAttrNames << existingNameAttrs.item(k).nodeName();
                        }
                        for (const QString &name : existingNameAttrNames) {
                            nameEl.removeAttribute(name);
                        }
                        for (auto it = param.nameAttributesMap.constBegin(); it != param.nameAttributesMap.constEnd(); ++it) {
                            nameEl.setAttribute(it.key(), it.value());
                        }
                        // --- /Установка атрибутов Name ---
                        nameEl.firstChild().setNodeValue(param.name);
                    }
                    // --- /Обновление Name ---

                    // --- Обновление Description ---
                    if (!descList.isEmpty()) {
                        QDomElement descEl = descList.at(0).toElement();
                        // --- Установка атрибутов Description ---
                        QDomNamedNodeMap existingDescAttrs = descEl.attributes();
                        QStringList existingDescAttrNames;
                        for (int k = 0; k < existingDescAttrs.count(); ++k) {
                            existingDescAttrNames << existingDescAttrs.item(k).nodeName();
                        }
                        for (const QString &name : existingDescAttrNames) {
                            descEl.removeAttribute(name);
                        }
                        for (auto it = param.descriptionAttributesMap.constBegin(); it != param.descriptionAttributesMap.constEnd(); ++it) {
                            descEl.setAttribute(it.key(), it.value());
                        }
                        // --- /Установка атрибутов Description ---
                        descEl.firstChild().setNodeValue(param.description);
                    }
                    // --- /Обновление Description ---

                    if (!defaultList.isEmpty()) {
                        QDomElement defaultEl = defaultList.at(0).toElement();
                        if (defaultEl.firstChildElement("Element").isNull()) {
                            defaultEl.firstChild().setNodeValue(param.defaultValue);
                        } else {
                            qWarning() << "Skipping update of complex Default value for Parameter" << param.id;
                        }
                    }
                    if (paramEl.attribute("type") != param.type) {
                        paramEl.setAttribute("type", param.type);
                    }
                    QDomNodeList attributesLists = paramEl.elementsByTagName("Attributes");
                    if (!attributesLists.isEmpty()) {
                        QDomElement attributesEl = attributesLists.at(0).toElement();
                        updateAttributesElement(attributesEl, param.attributesMap);
                    }
                }
            }
            // --- /Обновление существующих параметров DeviceParameterSet ---

            // --- Добавление новых параметров DeviceParameterSet ---
            if (paramList.size() < devInfo.deviceParameters.size()) {
                for (int i = paramList.size(); i < devInfo.deviceParameters.size(); ++i) {
                    const DeviceParameter &param = devInfo.deviceParameters[i];
                    QDomElement newParamEl = doc.createElement("Parameter");
                    newParamEl.setAttribute("ParameterId", param.id);
                    newParamEl.setAttribute("type", param.type);

                    QDomElement nameEl = doc.createElement("Name");
                    // --- Установка атрибутов Name для нового параметра ---
                    for (auto it = param.nameAttributesMap.constBegin(); it != param.nameAttributesMap.constEnd(); ++it) {
                        nameEl.setAttribute(it.key(), it.value());
                    }
                    // --- /Установка атрибутов Name ---
                    nameEl.appendChild(doc.createTextNode(param.name));
                    newParamEl.appendChild(nameEl);

                    QDomElement descEl = doc.createElement("Description");
                    // --- Установка атрибутов Description для нового параметра ---
                    for (auto it = param.descriptionAttributesMap.constBegin(); it != param.descriptionAttributesMap.constEnd(); ++it) {
                        descEl.setAttribute(it.key(), it.value());
                    }
                    // --- /Установка атрибутов Description ---
                    descEl.appendChild(doc.createTextNode(param.description));
                    newParamEl.appendChild(descEl);

                    QDomElement defaultEl = doc.createElement("Default");
                    defaultEl.appendChild(doc.createTextNode(param.defaultValue));
                    newParamEl.appendChild(defaultEl);

                    if (!param.attributesMap.isEmpty()) {
                        QDomElement attributesEl = doc.createElement("Attributes");
                        updateAttributesElement(attributesEl, param.attributesMap);
                        newParamEl.appendChild(attributesEl);
                    }

                    deviceParamSetElement.appendChild(newParamEl);
                }
            }
            // --- /Добавление новых параметров DeviceParameterSet ---
        }
    }
    QDomNodeList connectorLists = root.elementsByTagName("Connector");
    for (int i = 0; i < connectorLists.size() && i < devInfo.connectors.size(); ++i) {
        QDomElement connectorEl = connectorLists.at(i).toElement();
        if (!connectorEl.isNull()) {
            const ConnectorInfo &connInfo = devInfo.connectors[i];
            if (connInfo.isModified) {
                if (connInfo.moduleType != connectorEl.attribute("moduleType")) {
                    connectorEl.setAttribute("moduleType", connInfo.moduleType);
                }
                if (connInfo.interface != connectorEl.attribute("interface")) {
                    connectorEl.setAttribute("interface", connInfo.interface);
                }
                if (connInfo.role != connectorEl.attribute("role")) {
                    connectorEl.setAttribute("role", connInfo.role);
                }
                if (connectorEl.hasAttribute("connectorId")) {
                    if (connInfo.connectorId != connectorEl.attribute("connectorId")) {
                        connectorEl.setAttribute("connectorId", connInfo.connectorId);
                    }
                } else if (!connInfo.connectorId.isEmpty()) {
                    connectorEl.setAttribute("connectorId", connInfo.connectorId);
                }
                if (connectorEl.hasAttribute("hostpath")) {
                    if (connInfo.hostpath != connectorEl.attribute("hostpath")) {
                        connectorEl.setAttribute("hostpath", connInfo.hostpath);
                    }
                } else if (!connInfo.hostpath.isEmpty()) {
                    connectorEl.setAttribute("hostpath", connInfo.hostpath);
                }
                QDomNodeList interfaceNameLists = connectorEl.elementsByTagName("InterfaceName");
                if (!interfaceNameLists.isEmpty()) {
                    QDomElement interfaceNameEl = interfaceNameLists.at(0).toElement();
                    if (interfaceNameEl.firstChild().nodeValue() != connInfo.interfaceName) {
                        interfaceNameEl.firstChild().setNodeValue(connInfo.interfaceName);
                    }
                }

                // --- Обработка HostParameterSet ---
                QDomNodeList hostParamSetLists = connectorEl.elementsByTagName("HostParameterSet");
                if (!hostParamSetLists.isEmpty()) {
                    QDomElement hostParamSetEl = hostParamSetLists.at(0).toElement();
                    if (!hostParamSetEl.isNull()) {
                        // --- НОВАЯ ЛОГИКА: Полностью пересобираем HostParameterSet ---
                        // 1. Удаляем все существующие узлы <Parameter>
                        QDomNodeList paramList = hostParamSetEl.elementsByTagName("Parameter");
                        // Удаляем в обратном порядке, чтобы индексы оставались валидными
                        for (int j = paramList.size() - 1; j >= 0; --j) {
                            QDomElement paramEl = paramList.at(j).toElement();
                            if (!paramEl.isNull()) {
                                hostParamSetEl.removeChild(paramEl);
                            }
                        }

                        // 2. Создаём новые узлы <Parameter> из DeviceInfo
                        for (const DeviceParameter &param : connInfo.parameters) {
                            QDomElement paramEl = doc.createElement("Parameter");
                            paramEl.setAttribute("ParameterId", param.id);
                            paramEl.setAttribute("type", param.type);

                            QDomElement nameEl = doc.createElement("Name");
                            // --- Установка атрибутов Name ---
                            for (auto it = param.nameAttributesMap.constBegin(); it != param.nameAttributesMap.constEnd(); ++it) {
                                nameEl.setAttribute(it.key(), it.value());
                            }
                            // --- /Установка атрибутов Name ---
                            nameEl.appendChild(doc.createTextNode(param.name));
                            paramEl.appendChild(nameEl);

                            QDomElement descEl = doc.createElement("Description");
                            // --- Установка атрибутов Description ---
                            for (auto it = param.descriptionAttributesMap.constBegin(); it != param.descriptionAttributesMap.constEnd(); ++it) {
                                descEl.setAttribute(it.key(), it.value());
                            }
                            // --- /Установка атрибутов Description ---
                            descEl.appendChild(doc.createTextNode(param.description));
                            paramEl.appendChild(descEl);

                            QDomElement defaultEl = doc.createElement("Default");
                            defaultEl.appendChild(doc.createTextNode(param.defaultValue));
                            paramEl.appendChild(defaultEl);

                            if (!param.attributesMap.isEmpty()) {
                                QDomElement attributesEl = doc.createElement("Attributes");
                                updateAttributesElement(attributesEl, param.attributesMap);
                                paramEl.appendChild(attributesEl);
                            }

                            hostParamSetEl.appendChild(paramEl);
                        }
                    }
                }
                // --- /Обработка HostParameterSet ---
            }
        }
    }
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCritical() << "Cannot open file for writing:" << fileName << file.errorString();
        return false;
    }
    QTextStream outStream(&file);
    doc.save(outStream, 4);
    if (outStream.status() != QTextStream::Ok) {
        qCritical() << "Error writing XML:" << outStream.status();
        return false;
    }
    return true;
}

QDomDocument createTemplateForDiscreteIO() {

    QDomDocument doc;
    // Добавляем декларацию XML
    QDomProcessingInstruction xmlDeclaration = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(xmlDeclaration);

    // Создаем корневой элемент DeviceDescription с пространствами имен
    QDomElement root = doc.createElementNS("http://www.3s-software.com/schemas/DeviceDescription-1.0.xsd", "DeviceDescription");
    //root.setAttribute("xmlns", "http://www.3s-software.com/schemas/DeviceDescription-1.0.xsd");
    root.setAttribute("xmlns:ts", "http://www.3s-software.com/schemas/TargetSettings-0.1.xsd");
    root.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    doc.appendChild(root);

    QDomComment typesComment = doc.createComment("Types namespace=\"std\"></Types");
    root.appendChild(typesComment);

    // --- Добавляем Types ---
    QDomElement typesElement = doc.createElement("Types");
    typesElement.setAttribute("namespace", "localtype");
    root.appendChild(typesElement);

    // Добавляем BitfieldType TBit1Byte
    QDomElement bitfieldTypeElement = doc.createElement("BitfieldType");
    bitfieldTypeElement.setAttribute("basetype", "std:BYTE");
    bitfieldTypeElement.setAttribute("name", "TBit1Byte");
    typesElement.appendChild(bitfieldTypeElement);

    // Добавляем компоненты Bit0 - Bit7
    for (int i = 0; i < 8; ++i) {
        QDomElement componentElement = doc.createElement("Component");
        componentElement.setAttribute("identifier", QString("Bit%1").arg(i));
        componentElement.setAttribute("type", "std:BOOL");

        QDomElement defaultElement = doc.createElement("Default");
        componentElement.appendChild(defaultElement);

        QDomElement visibleNameElement = doc.createElement("VisibleName");
        visibleNameElement.setAttribute("name", QString("localtype:TBit1Byte.Bit%1").arg(i));
        visibleNameElement.appendChild(doc.createTextNode(QString("BIT%1").arg(i)));
        componentElement.appendChild(visibleNameElement);

        bitfieldTypeElement.appendChild(componentElement);
    }
    // --- /Добавляем Types ---

    // --- Добавляем File ---
    QDomElement fileElement = doc.createElement("File");
    fileElement.setAttribute("fileref", "local");
    fileElement.setAttribute("identifier", "image_io");

    QDomElement localFileElement = doc.createElement("LocalFile");
    localFileElement.appendChild(doc.createTextNode("MyDiscreteIO.gif"));
    fileElement.appendChild(localFileElement);

    root.appendChild(fileElement);
    // --- /Добавляем File ---

    // --- Добавляем Device ---
    QDomElement deviceElement = doc.createElement("Device");
    deviceElement.setAttribute("hideInCatalogue", "false");
    root.appendChild(deviceElement);

    // DeviceIdentification
    QDomElement deviceIdentElement = doc.createElement("DeviceIdentification");
    deviceElement.appendChild(deviceIdentElement);

    QDomElement typeElement = doc.createElement("Type");
    typeElement.appendChild(doc.createTextNode("40305"));
    deviceIdentElement.appendChild(typeElement);

    QDomElement idElement = doc.createElement("Id");
    //idElement.appendChild(doc.createTextNode("10ad 1b7f"));
    idElement.appendChild(doc.createTextNode("10ad mydiscreteio"));
    deviceIdentElement.appendChild(idElement);

    QDomElement versionElement = doc.createElement("Version");
    versionElement.appendChild(doc.createTextNode("2.0.0.0"));
    deviceIdentElement.appendChild(versionElement);

    // DeviceInfo
    QDomElement deviceInfoElement = doc.createElement("DeviceInfo");
    deviceElement.appendChild(deviceInfoElement);

    QDomElement nameElement = doc.createElement("Name");
    nameElement.setAttribute("name", "local:ModelName");
    //nameElement.appendChild(doc.createTextNode("GT-1B7F"));
    nameElement.appendChild(doc.createTextNode("MyDiscreteIO"));
    deviceInfoElement.appendChild(nameElement);

    QDomElement descElement = doc.createElement("Description");
    descElement.setAttribute("name", "local:DeviceDescription");
    descElement.appendChild(doc.createTextNode("Selectable DI/DO 16CH, Sink Input/Source Output, 24Vdc"));
    deviceInfoElement.appendChild(descElement);

    QDomElement vendorElement = doc.createElement("Vendor");
    vendorElement.setAttribute("name", "local:VendorName");
    vendorElement.appendChild(doc.createTextNode("CREVIS CO.,LTD"));
    deviceInfoElement.appendChild(vendorElement);

    QDomElement orderNumElement = doc.createElement("OrderNumber");
    //orderNumElement.appendChild(doc.createTextNode("GT-1B7F"));
    orderNumElement.appendChild(doc.createTextNode("MyDiscreteIO"));
    deviceInfoElement.appendChild(orderNumElement);

    QDomElement imageElement = doc.createElement("Image");
    imageElement.setAttribute("name", "local:image_io");
    //imageElement.appendChild(doc.createTextNode("GT-1B7F.gif"));
    imageElement.appendChild(doc.createTextNode("MyDiscreteIO.gif"));
    deviceInfoElement.appendChild(imageElement);

    // --- Добавляем первый Connector (child) ---
    QDomComment connectorComment = doc.createComment("Connector moduleType=\"257\" interface=\"Common.PCI\" role=\"child\" explicit=\"false\" connectorId=\"1\" hostpath=\"-1\">\n      <InterfaceName name=\"local:PCI\">PCI-Bus</InterfaceName>\n      <Slot count=\"1\" allowEmpty=\"false\"></Slot>\n    </Connector");
    deviceElement.appendChild(connectorComment);

    QDomElement connector1Element = doc.createElement("Connector");
    connector1Element.setAttribute("moduleType", "47000");
    connector1Element.setAttribute("interface", "CVS.OptionG");
    connector1Element.setAttribute("role", "child");
    connector1Element.setAttribute("explicit", "false");
    connector1Element.setAttribute("connectorId", "1");
    connector1Element.setAttribute("hostpath", "-1");

    QDomElement interfaceName1Element = doc.createElement("InterfaceName");
    interfaceName1Element.setAttribute("name", "local:PCI");
    interfaceName1Element.appendChild(doc.createTextNode("CVS Option"));
    connector1Element.appendChild(interfaceName1Element);

    QDomElement slotElement = doc.createElement("Slot");
    slotElement.setAttribute("count", "1");
    slotElement.setAttribute("allowEmpty", "false");
    connector1Element.appendChild(slotElement);

    deviceElement.appendChild(connector1Element);
    // --- /Добавляем первый Connector (child) ---

    // --- Добавляем второй Connector (parent) ---
    QDomElement connector2Element = doc.createElement("Connector");
    connector2Element.setAttribute("moduleType", "40305");
    connector2Element.setAttribute("interface", "MyCompany:Internal");
    connector2Element.setAttribute("role", "parent");
    connector2Element.setAttribute("explicit", "false");
    connector2Element.setAttribute("connectorId", "2");
    connector2Element.setAttribute("hostpath", "1");
    deviceElement.appendChild(connector2Element);

    QDomElement interfaceName2Element = doc.createElement("InterfaceName");
    interfaceName2Element.setAttribute("name", "local:DP");
    interfaceName2Element.appendChild(doc.createTextNode("Digital IOs"));
    connector2Element.appendChild(interfaceName2Element);

    QDomElement varElement = doc.createElement("Var");
    varElement.setAttribute("max", "8");
    connector2Element.appendChild(varElement);

    QDomElement driverInfoElement = doc.createElement("DriverInfo");
    driverInfoElement.setAttribute("needsBusCycle", "true");
    connector2Element.appendChild(driverInfoElement);

    // HostParameterSet
    QDomElement hostParamSetElement = doc.createElement("HostParameterSet");
    connector2Element.appendChild(hostParamSetElement);

    struct ParamDef {
        QString id;
        QString type;
        QString channel;
        QString download;
        QString functional;
        QString offlineAccess;
        QString onlineAccess;
        QString defaultValue;
        QString nameKey;
        QString nameValue;
        QString descKey;
        QString descValue;
    };

    std::vector<ParamDef> params = {
        {"1000", "localtype:TBit1Byte", "input", "true", "false", "read", "read", "0", "local:in0", "IN0", "", ""},
        {"1001", "localtype:TBit1Byte", "input", "true", "false", "read", "read", "0", "local:in1", "IN1", "", ""},
        {"2000", "localtype:TBit1Byte", "output", "true", "false", "readwrite", "readwrite", "0", "local:out0", "OUT0", "", ""},
        {"2001", "localtype:TBit1Byte", "output", "true", "false", "readwrite", "readwrite", "0", "local:out0", "OUT1", "", ""},
        {"393218", "std:STRING", "none", "true", "false", "read", "read", "'CREVIS'", "local:Id393218", "Vendor", "local:Id393218.Desc", "Vendor of the device"},
        {"393219", "std:STRING", "none", "true", "false", "read", "read", "'MyDiscreteIO'", "local:Id393219", "Module ID", "local:Id393219.Desc", "Module ID of the device"},
        {"400000", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400000", "Operation Selection(CH0~CH7)", "local:Id400000.Desc", "0:Input, 1:Output"},
        {"400001", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400001", "Operation Selection(CH8~CH15)", "local:Id400001.Desc", "0:Input, 1:Output"},
        {"400002", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400002", "Fault Action(CH0~CH7)", "local:Id400002.Desc", "0:Falut value, 1:Hold last state"},
        {"400003", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400003", "Fault Action(CH8~CH15)", "local:Id400003.Desc", "0:Falut value, 1:Hold last state"},
        {"400004", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400004", "Fault value(CH0~CH7)", "local:Id400004.Desc", "0:OFF, 1:ON"},
        {"400005", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400005", "Fault value(CH8~CH15)", "local:Id400005.Desc", "0:OFF, 1:ON"},
        {"400006", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400006", "Input Filter value", "local:Id400006.Desc", "0~10(unit:ms)"},
        {"400007", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400007", "Reserved", "local:Id400007.Desc", " "},
    };

    for (const auto& pDef : params) {
        QDomElement paramElement = doc.createElement("Parameter");
        paramElement.setAttribute("ParameterId", pDef.id);
        paramElement.setAttribute("type", pDef.type);
        hostParamSetElement.appendChild(paramElement);

        QDomElement attributesElement = doc.createElement("Attributes");
        attributesElement.setAttribute("channel", pDef.channel);
        attributesElement.setAttribute("download", pDef.download);
        attributesElement.setAttribute("functional", pDef.functional);
        attributesElement.setAttribute("offlineaccess", pDef.offlineAccess);
        attributesElement.setAttribute("onlineaccess", pDef.onlineAccess);
        paramElement.appendChild(attributesElement);

        QDomElement defaultElement = doc.createElement("Default");
        defaultElement.appendChild(doc.createTextNode(pDef.defaultValue));
        paramElement.appendChild(defaultElement);

        QDomElement nameElement = doc.createElement("Name");
        nameElement.setAttribute("name", pDef.nameKey);
        nameElement.appendChild(doc.createTextNode(pDef.nameValue));
        paramElement.appendChild(nameElement);

        if (!pDef.descKey.isEmpty() && !pDef.descValue.isEmpty()) {
            QDomElement descElement = doc.createElement("Description");
            descElement.setAttribute("name", pDef.descKey);
            descElement.appendChild(doc.createTextNode(pDef.descValue));
            paramElement.appendChild(descElement);
        }
    }
    // --- /Добавляем второй Connector (parent) ---

    return doc;
}

QDomDocument createTemplateForDiscreteInput() {

    QDomDocument doc;
    // Добавляем декларацию XML
    QDomProcessingInstruction xmlDeclaration = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(xmlDeclaration);

    // Создаем корневой элемент DeviceDescription с пространствами имен
    QDomElement root = doc.createElementNS("http://www.3s-software.com/schemas/DeviceDescription-1.0.xsd", "DeviceDescription");
    //root.setAttribute("xmlns", "http://www.3s-software.com/schemas/DeviceDescription-1.0.xsd");
    root.setAttribute("xmlns:ts", "http://www.3s-software.com/schemas/TargetSettings-0.1.xsd");
    root.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    doc.appendChild(root);

    QDomComment typesComment = doc.createComment("Types namespace=\"std\"></Types");
    root.appendChild(typesComment);

    // --- Добавляем Types ---
    QDomElement typesElement = doc.createElement("Types");
    typesElement.setAttribute("namespace", "localtype");
    root.appendChild(typesElement);

    // Добавляем BitfieldType TBit1Byte
    QDomElement bitfieldTypeElement = doc.createElement("BitfieldType");
    bitfieldTypeElement.setAttribute("basetype", "std:BYTE");
    bitfieldTypeElement.setAttribute("name", "TBit1Byte");
    typesElement.appendChild(bitfieldTypeElement);

    // Добавляем компоненты Bit0 - Bit7
    for (int i = 0; i < 8; ++i) {
        QDomElement componentElement = doc.createElement("Component");
        componentElement.setAttribute("identifier", QString("Bit%1").arg(i));
        componentElement.setAttribute("type", "std:BOOL");

        QDomElement defaultElement = doc.createElement("Default");
        componentElement.appendChild(defaultElement);

        QDomElement visibleNameElement = doc.createElement("VisibleName");
        visibleNameElement.setAttribute("name", QString("localtype:TBit1Byte.Bit%1").arg(i));
        visibleNameElement.appendChild(doc.createTextNode(QString("BIT%1").arg(i)));
        componentElement.appendChild(visibleNameElement);

        bitfieldTypeElement.appendChild(componentElement);
    }
    // --- /Добавляем Types ---

    // --- Добавляем File ---
    QDomElement fileElement = doc.createElement("File");
    fileElement.setAttribute("fileref", "local");
    fileElement.setAttribute("identifier", "image_input");

    QDomElement localFileElement = doc.createElement("LocalFile");
    localFileElement.appendChild(doc.createTextNode("MyDiscreteInput.gif"));
    fileElement.appendChild(localFileElement);

    root.appendChild(fileElement);
    // --- /Добавляем File ---

    // --- Добавляем Device ---
    QDomElement deviceElement = doc.createElement("Device");
    deviceElement.setAttribute("hideInCatalogue", "false");
    root.appendChild(deviceElement);

    // DeviceIdentification
    QDomElement deviceIdentElement = doc.createElement("DeviceIdentification");
    deviceElement.appendChild(deviceIdentElement);

    QDomElement typeElement = doc.createElement("Type");
    typeElement.appendChild(doc.createTextNode("40303"));
    deviceIdentElement.appendChild(typeElement);

    QDomElement idElement = doc.createElement("Id");
    //idElement.appendChild(doc.createTextNode("10ad 1b7f"));
    idElement.appendChild(doc.createTextNode("10ad mydiscreteinput"));
    deviceIdentElement.appendChild(idElement);

    QDomElement versionElement = doc.createElement("Version");
    versionElement.appendChild(doc.createTextNode("2.0.0.0"));
    deviceIdentElement.appendChild(versionElement);

    // DeviceInfo
    QDomElement deviceInfoElement = doc.createElement("DeviceInfo");
    deviceElement.appendChild(deviceInfoElement);

    QDomElement nameElement = doc.createElement("Name");
    nameElement.setAttribute("name", "local:ModelName");
    //nameElement.appendChild(doc.createTextNode("GT-1B7F"));
    nameElement.appendChild(doc.createTextNode("MyDiscreteInput"));
    deviceInfoElement.appendChild(nameElement);

    QDomElement descElement = doc.createElement("Description");
    descElement.setAttribute("name", "local:DeviceDescription");
    descElement.appendChild(doc.createTextNode("Universal Input Terminal 32CH, 24Vdc"));
    deviceInfoElement.appendChild(descElement);

    QDomElement vendorElement = doc.createElement("Vendor");
    vendorElement.setAttribute("name", "local:VendorName");
    vendorElement.appendChild(doc.createTextNode("CREVIS CO.,LTD"));
    deviceInfoElement.appendChild(vendorElement);

    QDomElement orderNumElement = doc.createElement("OrderNumber");
    //orderNumElement.appendChild(doc.createTextNode("GT-1B7F"));
    orderNumElement.appendChild(doc.createTextNode("MyDiscreteInput"));
    deviceInfoElement.appendChild(orderNumElement);

    QDomElement imageElement = doc.createElement("Image");
    imageElement.setAttribute("name", "local:image_io");
    //imageElement.appendChild(doc.createTextNode("GT-1B7F.gif"));
    imageElement.appendChild(doc.createTextNode("MyDiscreteInput.gif"));
    deviceInfoElement.appendChild(imageElement);

    QDomComment connectorComment = doc.createComment("Connector moduleType=\"257\" interface=\"Common.PCI\" role=\"child\" explicit=\"false\" connectorId=\"1\" hostpath=\"-1\">\n      <InterfaceName name=\"local:PCI\">PCI-Bus</InterfaceName>\n      <Slot count=\"1\" allowEmpty=\"false\"></Slot>\n    </Connector");
    deviceElement.appendChild(connectorComment);

    QDomElement connector1Element = doc.createElement("Connector");
    connector1Element.setAttribute("moduleType", "47000");
    connector1Element.setAttribute("interface", "CVS.OptionG");
    connector1Element.setAttribute("role", "child");
    connector1Element.setAttribute("explicit", "false");
    connector1Element.setAttribute("connectorId", "1");
    connector1Element.setAttribute("hostpath", "-1");

    QDomElement interfaceName1Element = doc.createElement("InterfaceName");
    interfaceName1Element.setAttribute("name", "local:PCI");
    interfaceName1Element.appendChild(doc.createTextNode("CVS Option"));
    connector1Element.appendChild(interfaceName1Element);

    QDomElement slotElement = doc.createElement("Slot");
    slotElement.setAttribute("count", "1");
    slotElement.setAttribute("allowEmpty", "false");
    connector1Element.appendChild(slotElement);

    deviceElement.appendChild(connector1Element);

    QDomElement connector2Element = doc.createElement("Connector");
    connector2Element.setAttribute("moduleType", "40303");
    connector2Element.setAttribute("interface", "MyCompany:Internal");
    connector2Element.setAttribute("role", "parent");
    connector2Element.setAttribute("explicit", "false");
    connector2Element.setAttribute("connectorId", "2");
    connector2Element.setAttribute("hostpath", "1");
    deviceElement.appendChild(connector2Element);

    QDomElement interfaceName2Element = doc.createElement("InterfaceName");
    interfaceName2Element.setAttribute("name", "local:DP");
    interfaceName2Element.appendChild(doc.createTextNode("Digital IOs"));
    connector2Element.appendChild(interfaceName2Element);

    QDomElement varElement = doc.createElement("Var");
    varElement.setAttribute("max", "8");
    connector2Element.appendChild(varElement);

    QDomElement driverInfoElement = doc.createElement("DriverInfo");
    driverInfoElement.setAttribute("needsBusCycle", "true");
    connector2Element.appendChild(driverInfoElement);

    // HostParameterSet
    QDomElement hostParamSetElement = doc.createElement("HostParameterSet");
    connector2Element.appendChild(hostParamSetElement);

    struct ParamDef {
        QString id;
        QString type;
        QString channel;
        QString download;
        QString functional;
        QString offlineAccess;
        QString onlineAccess;
        QString defaultValue;
        QString nameKey;
        QString nameValue;
        QString descKey;
        QString descValue;
    };

    std::vector<ParamDef> params = {
        {"1000", "localtype:TBit1Byte", "input", "true", "false", "read", "read", "0", "local:in0", "IN0", "", ""},
        {"1001", "localtype:TBit1Byte", "input", "true", "false", "read", "read", "0", "local:in1", "IN1", "", ""},
        {"1002", "localtype:TBit1Byte", "input", "true", "false", "read", "read", "0", "local:in2", "IN2", "", ""},
        {"1003", "localtype:TBit1Byte", "input", "true", "false", "read", "read", "0", "local:in3", "IN3", "", ""},
        {"393218", "std:STRING", "none", "true", "false", "read", "read", "'CREVIS'", "local:Id393218", "Vendor", "local:Id393218.Desc", "Vendor of the device"},
        {"393219", "std:STRING", "none", "true", "false", "read", "read", "'MyDiscreteInput'", "local:Id393219", "Module ID", "local:Id393219.Desc", "Module ID of the device"},
        {"400000", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400000", "Input Filter value", "local:Id400000.Desc", "0~10(unit:ms)"},
        {"400001", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400001", "Reserved", "local:Id400001.Desc", " "},
    };

    for (const auto& pDef : params) {
        QDomElement paramElement = doc.createElement("Parameter");
        paramElement.setAttribute("ParameterId", pDef.id);
        paramElement.setAttribute("type", pDef.type);
        hostParamSetElement.appendChild(paramElement);

        QDomElement attributesElement = doc.createElement("Attributes");
        attributesElement.setAttribute("channel", pDef.channel);
        attributesElement.setAttribute("download", pDef.download);
        attributesElement.setAttribute("functional", pDef.functional);
        attributesElement.setAttribute("offlineaccess", pDef.offlineAccess);
        attributesElement.setAttribute("onlineaccess", pDef.onlineAccess);
        paramElement.appendChild(attributesElement);

        QDomElement defaultElement = doc.createElement("Default");
        defaultElement.appendChild(doc.createTextNode(pDef.defaultValue));
        paramElement.appendChild(defaultElement);

        QDomElement nameElement = doc.createElement("Name");
        nameElement.setAttribute("name", pDef.nameKey);
        nameElement.appendChild(doc.createTextNode(pDef.nameValue));
        paramElement.appendChild(nameElement);

        if (!pDef.descKey.isEmpty() && !pDef.descValue.isEmpty()) {
            QDomElement descElement = doc.createElement("Description");
            descElement.setAttribute("name", pDef.descKey);
            descElement.appendChild(doc.createTextNode(pDef.descValue));
            paramElement.appendChild(descElement);
        }
    }

    return doc;
}

QDomDocument createTemplateForDiscreteOutput() {

    QDomDocument doc;
    // Добавляем декларацию XML
    QDomProcessingInstruction xmlDeclaration = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(xmlDeclaration);

    // Создаем корневой элемент DeviceDescription с пространствами имен
    QDomElement root = doc.createElementNS("http://www.3s-software.com/schemas/DeviceDescription-1.0.xsd", "DeviceDescription");
    //root.setAttribute("xmlns", "http://www.3s-software.com/schemas/DeviceDescription-1.0.xsd");
    root.setAttribute("xmlns:ts", "http://www.3s-software.com/schemas/TargetSettings-0.1.xsd");
    root.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    doc.appendChild(root);

    QDomComment typesComment = doc.createComment("Types namespace=\"std\"></Types");
    root.appendChild(typesComment);

    // --- Добавляем Types ---
    QDomElement typesElement = doc.createElement("Types");
    typesElement.setAttribute("namespace", "localtype");
    root.appendChild(typesElement);

    // Добавляем BitfieldType TBit1Byte
    QDomElement bitfieldTypeElement = doc.createElement("BitfieldType");
    bitfieldTypeElement.setAttribute("basetype", "std:BYTE");
    bitfieldTypeElement.setAttribute("name", "TBit1Byte");
    typesElement.appendChild(bitfieldTypeElement);

    // Добавляем компоненты Bit0 - Bit7
    for (int i = 0; i < 8; ++i) {
        QDomElement componentElement = doc.createElement("Component");
        componentElement.setAttribute("identifier", QString("Bit%1").arg(i));
        componentElement.setAttribute("type", "std:BOOL");

        QDomElement defaultElement = doc.createElement("Default");
        componentElement.appendChild(defaultElement);

        QDomElement visibleNameElement = doc.createElement("VisibleName");
        visibleNameElement.setAttribute("name", QString("localtype:TBit1Byte.Bit%1").arg(i));
        visibleNameElement.appendChild(doc.createTextNode(QString("BIT%1").arg(i)));
        componentElement.appendChild(visibleNameElement);

        bitfieldTypeElement.appendChild(componentElement);
    }
    // --- /Добавляем Types ---

    // --- Добавляем File ---
    QDomElement fileElement = doc.createElement("File");
    fileElement.setAttribute("fileref", "local");
    fileElement.setAttribute("identifier", "image_output");

    QDomElement localFileElement = doc.createElement("LocalFile");
    localFileElement.appendChild(doc.createTextNode("MyDiscreteOutput.gif"));
    fileElement.appendChild(localFileElement);

    root.appendChild(fileElement);
    // --- /Добавляем File ---

    // --- Добавляем Device ---
    QDomElement deviceElement = doc.createElement("Device");
    deviceElement.setAttribute("hideInCatalogue", "false");
    root.appendChild(deviceElement);

    // DeviceIdentification
    QDomElement deviceIdentElement = doc.createElement("DeviceIdentification");
    deviceElement.appendChild(deviceIdentElement);

    QDomElement typeElement = doc.createElement("Type");
    typeElement.appendChild(doc.createTextNode("40302"));
    deviceIdentElement.appendChild(typeElement);

    QDomElement idElement = doc.createElement("Id");
    //idElement.appendChild(doc.createTextNode("10ad 1b7f"));
    idElement.appendChild(doc.createTextNode("10ad mydiscreteoutput"));
    deviceIdentElement.appendChild(idElement);

    QDomElement versionElement = doc.createElement("Version");
    versionElement.appendChild(doc.createTextNode("2.0.0.0"));
    deviceIdentElement.appendChild(versionElement);

    // DeviceInfo
    QDomElement deviceInfoElement = doc.createElement("DeviceInfo");
    deviceElement.appendChild(deviceInfoElement);

    QDomElement nameElement = doc.createElement("Name");
    nameElement.setAttribute("name", "local:ModelName");
    //nameElement.appendChild(doc.createTextNode("GT-1B7F"));
    nameElement.appendChild(doc.createTextNode("MyDiscreteOutput"));
    deviceInfoElement.appendChild(nameElement);

    QDomElement descElement = doc.createElement("Description");
    descElement.setAttribute("name", "local:DeviceDescription");
    descElement.appendChild(doc.createTextNode("Source Output Terminal 32CH, 24Vdc/0.3A"));
    deviceInfoElement.appendChild(descElement);

    QDomElement vendorElement = doc.createElement("Vendor");
    vendorElement.setAttribute("name", "local:VendorName");
    vendorElement.appendChild(doc.createTextNode("CREVIS CO.,LTD"));
    deviceInfoElement.appendChild(vendorElement);

    QDomElement orderNumElement = doc.createElement("OrderNumber");
    //orderNumElement.appendChild(doc.createTextNode("GT-1B7F"));
    orderNumElement.appendChild(doc.createTextNode("MyDiscreteOutput"));
    deviceInfoElement.appendChild(orderNumElement);

    QDomElement imageElement = doc.createElement("Image");
    imageElement.setAttribute("name", "local:image_io");
    //imageElement.appendChild(doc.createTextNode("GT-1B7F.gif"));
    imageElement.appendChild(doc.createTextNode("MyDiscreteOutput.gif"));
    deviceInfoElement.appendChild(imageElement);

    QDomComment connectorComment = doc.createComment("Connector moduleType=\"257\" interface=\"Common.PCI\" role=\"child\" explicit=\"false\" connectorId=\"1\" hostpath=\"-1\">\n      <InterfaceName name=\"local:PCI\">PCI-Bus</InterfaceName>\n      <Slot count=\"1\" allowEmpty=\"false\"></Slot>\n    </Connector");
    deviceElement.appendChild(connectorComment);

    QDomElement connector1Element = doc.createElement("Connector");
    connector1Element.setAttribute("moduleType", "47000");
    connector1Element.setAttribute("interface", "CVS.OptionG");
    connector1Element.setAttribute("role", "child");
    connector1Element.setAttribute("explicit", "false");
    connector1Element.setAttribute("connectorId", "1");
    connector1Element.setAttribute("hostpath", "-1");

    QDomElement interfaceName1Element = doc.createElement("InterfaceName");
    interfaceName1Element.setAttribute("name", "local:PCI");
    interfaceName1Element.appendChild(doc.createTextNode("CVS Option"));
    connector1Element.appendChild(interfaceName1Element);

    QDomElement slotElement = doc.createElement("Slot");
    slotElement.setAttribute("count", "1");
    slotElement.setAttribute("allowEmpty", "false");
    connector1Element.appendChild(slotElement);

    deviceElement.appendChild(connector1Element);

    QDomElement connector2Element = doc.createElement("Connector");
    connector2Element.setAttribute("moduleType", "40302");
    connector2Element.setAttribute("interface", "MyCompany:Internal");
    connector2Element.setAttribute("role", "parent");
    connector2Element.setAttribute("explicit", "false");
    connector2Element.setAttribute("connectorId", "2");
    connector2Element.setAttribute("hostpath", "1");
    deviceElement.appendChild(connector2Element);

    QDomElement interfaceName2Element = doc.createElement("InterfaceName");
    interfaceName2Element.setAttribute("name", "local:DP");
    interfaceName2Element.appendChild(doc.createTextNode("Digital IOs"));
    connector2Element.appendChild(interfaceName2Element);

    QDomElement varElement = doc.createElement("Var");
    varElement.setAttribute("max", "8");
    connector2Element.appendChild(varElement);

    QDomElement driverInfoElement = doc.createElement("DriverInfo");
    driverInfoElement.setAttribute("needsBusCycle", "true");
    connector2Element.appendChild(driverInfoElement);

    // HostParameterSet
    QDomElement hostParamSetElement = doc.createElement("HostParameterSet");
    connector2Element.appendChild(hostParamSetElement);

    struct ParamDef {
        QString id;
        QString type;
        QString channel;
        QString download;
        QString functional;
        QString offlineAccess;
        QString onlineAccess;
        QString defaultValue;
        QString nameKey;
        QString nameValue;
        QString descKey;
        QString descValue;
    };

    std::vector<ParamDef> params = {
        {"2000", "localtype:TBit1Byte", "output", "true", "false", "readwrite", "readwrite", "0", "local:out0", "OUT0", "", ""},
        {"2001", "localtype:TBit1Byte", "output", "true", "false", "readwrite", "readwrite", "0", "local:out1", "OUT1", "", ""},
        {"2002", "localtype:TBit1Byte", "output", "true", "false", "readwrite", "readwrite", "0", "local:out2", "OUT2", "", ""},
        {"2003", "localtype:TBit1Byte", "output", "true", "false", "readwrite", "readwrite", "0", "local:out3", "OUT3", "", ""},
        {"393218", "std:STRING", "none", "true", "false", "read", "read", "'CREVIS'", "local:Id393218", "Vendor", "local:Id393218.Desc", "Vendor of the device"},
        {"393219", "std:STRING", "none", "true", "false", "read", "read", "'MyDiscreteOutput'", "local:Id393219", "Module ID", "local:Id393219.Desc", "Module ID of the device"},
        {"400000", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400000", "Fault Action(CH0~CH7)", "local:Id400000.Desc", "0:Falut value, 1:Hold last state"},
        {"400001", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400001", "Fault Action(CH8~CH15)", "local:Id400001.Desc", "0:Falut value, 1:Hold last state"},
        {"400002", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400002", "Fault Action(CH16~CH23)", "local:Id400002.Desc", "0:Falut value, 1:Hold last state"},
        {"400003", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400003", "Fault Action(CH24~CH31)", "local:Id400003.Desc", "0:Falut value, 1:Hold last state"},
        {"400004", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400004", "Fault value(CH0~CH7)", "local:Id400004.Desc", "0:OFF, 1:ON"},
        {"400005", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400005", "Fault value(CH8~CH15)", "local:Id400005.Desc", "0:OFF, 1:ON"},
        {"400006", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400006", "Fault value(CH16~CH23)", "local:Id400006.Desc", "0:OFF, 1:ON"},
        {"400007", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400007", "Fault value(CH24~CH31)", "local:Id400007.Desc", "0:OFF, 1:ON"},
    };

    for (const auto& pDef : params) {
        QDomElement paramElement = doc.createElement("Parameter");
        paramElement.setAttribute("ParameterId", pDef.id);
        paramElement.setAttribute("type", pDef.type);
        hostParamSetElement.appendChild(paramElement);

        QDomElement attributesElement = doc.createElement("Attributes");
        attributesElement.setAttribute("channel", pDef.channel);
        attributesElement.setAttribute("download", pDef.download);
        attributesElement.setAttribute("functional", pDef.functional);
        attributesElement.setAttribute("offlineaccess", pDef.offlineAccess);
        attributesElement.setAttribute("onlineaccess", pDef.onlineAccess);
        paramElement.appendChild(attributesElement);

        QDomElement defaultElement = doc.createElement("Default");
        defaultElement.appendChild(doc.createTextNode(pDef.defaultValue));
        paramElement.appendChild(defaultElement);

        QDomElement nameElement = doc.createElement("Name");
        nameElement.setAttribute("name", pDef.nameKey);
        nameElement.appendChild(doc.createTextNode(pDef.nameValue));
        paramElement.appendChild(nameElement);

        if (!pDef.descKey.isEmpty() && !pDef.descValue.isEmpty()) {
            QDomElement descElement = doc.createElement("Description");
            descElement.setAttribute("name", pDef.descKey);
            descElement.appendChild(doc.createTextNode(pDef.descValue));
            paramElement.appendChild(descElement);
        }
    }

    return doc;
}

QDomDocument createTemplateForAnalogInput() {

    QDomDocument doc;
    // Добавляем декларацию XML
    QDomProcessingInstruction xmlDeclaration = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(xmlDeclaration);

    // Создаем корневой элемент DeviceDescription с пространствами имен
    QDomElement root = doc.createElementNS("http://www.3s-software.com/schemas/DeviceDescription-1.0.xsd", "DeviceDescription");
    root.setAttribute("xmlns", "http://www.3s-software.com/schemas/DeviceDescription-1.0.xsd");
    root.setAttribute("xmlns:ts", "http://www.3s-software.com/schemas/TargetSettings-0.1.xsd");
    root.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    doc.appendChild(root);

    QDomComment typesComment = doc.createComment("Types namespace=\"std\"></Types");
    root.appendChild(typesComment);

    QDomElement typesElement = doc.createElement("Types");
    typesElement.setAttribute("namespace", "localtype");
    root.appendChild(typesElement);

    QDomElement fileElement = doc.createElement("File");
    fileElement.setAttribute("fileref", "local");
    fileElement.setAttribute("identifier", "image_input");

    QDomElement localFileElement = doc.createElement("LocalFile");
    localFileElement.appendChild(doc.createTextNode("MyDiscreteOutput.gif"));
    fileElement.appendChild(localFileElement);

    root.appendChild(fileElement);

    // --- Добавляем Device ---
    QDomElement deviceElement = doc.createElement("Device");
    deviceElement.setAttribute("hideInCatalogue", "false");
    root.appendChild(deviceElement);

    // DeviceIdentification
    QDomElement deviceIdentElement = doc.createElement("DeviceIdentification");
    deviceElement.appendChild(deviceIdentElement);

    QDomElement typeElement = doc.createElement("Type");
    typeElement.appendChild(doc.createTextNode("40301"));
    deviceIdentElement.appendChild(typeElement);

    QDomElement idElement = doc.createElement("Id");
    //idElement.appendChild(doc.createTextNode("10ad 1b7f"));
    idElement.appendChild(doc.createTextNode("10ad myanaloginput"));
    deviceIdentElement.appendChild(idElement);

    QDomElement versionElement = doc.createElement("Version");
    versionElement.appendChild(doc.createTextNode("2.0.0.0"));
    deviceIdentElement.appendChild(versionElement);

    // DeviceInfo
    QDomElement deviceInfoElement = doc.createElement("DeviceInfo");
    deviceElement.appendChild(deviceInfoElement);

    QDomElement nameElement = doc.createElement("Name");
    nameElement.setAttribute("name", "local:ModelName");
    //nameElement.appendChild(doc.createTextNode("GT-1B7F"));
    nameElement.appendChild(doc.createTextNode("MyAnalogInput"));
    deviceInfoElement.appendChild(nameElement);

    QDomElement descElement = doc.createElement("Description");
    descElement.setAttribute("name", "local:DeviceDescription");
    descElement.appendChild(doc.createTextNode("Voltage Input 16CH, 0~5Vdc/0~10Vdc/1~5Vdc, 16bit"));
    deviceInfoElement.appendChild(descElement);

    QDomElement vendorElement = doc.createElement("Vendor");
    vendorElement.setAttribute("name", "local:VendorName");
    vendorElement.appendChild(doc.createTextNode("CREVIS CO.,LTD"));
    deviceInfoElement.appendChild(vendorElement);

    QDomElement orderNumElement = doc.createElement("OrderNumber");
    //orderNumElement.appendChild(doc.createTextNode("GT-1B7F"));
    orderNumElement.appendChild(doc.createTextNode("MyAnalogInput"));
    deviceInfoElement.appendChild(orderNumElement);

    QDomElement imageElement = doc.createElement("Image");
    imageElement.setAttribute("name", "local:image_io");
    //imageElement.appendChild(doc.createTextNode("GT-1B7F.gif"));
    imageElement.appendChild(doc.createTextNode("MyAnalogInput.gif"));
    deviceInfoElement.appendChild(imageElement);

    QDomComment connectorComment = doc.createComment("Connector moduleType=\"257\" interface=\"Common.PCI\" role=\"child\" explicit=\"false\" connectorId=\"1\" hostpath=\"-1\">\n      <InterfaceName name=\"local:PCI\">PCI-Bus</InterfaceName>\n      <Slot count=\"1\" allowEmpty=\"false\"></Slot>\n    </Connector");
    deviceElement.appendChild(connectorComment);

    QDomElement connector1Element = doc.createElement("Connector");
    connector1Element.setAttribute("moduleType", "47000");
    connector1Element.setAttribute("interface", "CVS.OptionG");
    connector1Element.setAttribute("role", "child");
    connector1Element.setAttribute("explicit", "false");
    connector1Element.setAttribute("connectorId", "1");
    connector1Element.setAttribute("hostpath", "-1");

    QDomElement interfaceName1Element = doc.createElement("InterfaceName");
    interfaceName1Element.setAttribute("name", "local:PCI");
    interfaceName1Element.appendChild(doc.createTextNode("CVS Option"));
    connector1Element.appendChild(interfaceName1Element);

    QDomElement slotElement = doc.createElement("Slot");
    slotElement.setAttribute("count", "1");
    slotElement.setAttribute("allowEmpty", "false");
    connector1Element.appendChild(slotElement);

    deviceElement.appendChild(connector1Element);

    QDomElement connector2Element = doc.createElement("Connector");
    connector2Element.setAttribute("moduleType", "40302");
    connector2Element.setAttribute("interface", "MyCompany:Internal");
    connector2Element.setAttribute("role", "parent");
    connector2Element.setAttribute("explicit", "false");
    connector2Element.setAttribute("connectorId", "2");
    connector2Element.setAttribute("hostpath", "1");
    deviceElement.appendChild(connector2Element);

    QDomElement interfaceName2Element = doc.createElement("InterfaceName");
    interfaceName2Element.setAttribute("name", "local:DP");
    interfaceName2Element.appendChild(doc.createTextNode("Analog IOs"));
    connector2Element.appendChild(interfaceName2Element);

    QDomElement varElement = doc.createElement("Var");
    varElement.setAttribute("max", "8");
    connector2Element.appendChild(varElement);

    QDomElement driverInfoElement = doc.createElement("DriverInfo");
    driverInfoElement.setAttribute("needsBusCycle", "true");
    connector2Element.appendChild(driverInfoElement);

    // HostParameterSet
    QDomElement hostParamSetElement = doc.createElement("HostParameterSet");
    connector2Element.appendChild(hostParamSetElement);

    struct ParamDef {
        QString id;
        QString type;
        QString channel;
        QString download;
        QString functional;
        QString offlineAccess;
        QString onlineAccess;
        QString defaultValue;
        QString nameKey;
        QString nameValue;
        QString descKey;
        QString descValue;
    };

    std::vector<ParamDef> params = {
        {"3000", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in0", "AI0", "", ""},
        {"3001", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in1", "AI1", "", ""},
        {"3002", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in2", "AI2", "", ""},
        {"3003", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in3", "AI3", "", ""},
        {"3004", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in4", "AI4", "", ""},
        {"3005", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in5", "AI5", "", ""},
        {"3006", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in6", "AI6", "", ""},
        {"3007", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in7", "AI7", "", ""},
        {"3008", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in8", "AI8", "", ""},
        {"3009", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in9", "AI9", "", ""},
        {"3010", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in10", "AI10", "", ""},
        {"3011", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in11", "AI11", "", ""},
        {"3012", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in12", "AI12", "", ""},
        {"3013", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in13", "AI13", "", ""},
        {"3014", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in14", "AI14", "", ""},
        {"3015", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in15", "AI15", "", ""},
        {"393218", "std:STRING", "none", "true", "false", "read", "read", "'CREVIS'", "local:Id393218", "Vendor", "local:Id393218.Desc", "Vendor of the device"},
        {"393219", "std:STRING", "none", "true", "false", "read", "read", "'MyAnalogInput'", "local:Id393219", "Module ID", "local:Id393219.Desc", "Module ID of the device"},
        {"400000", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400000", "Voltage Range for CH0", "local:Id400000.Desc", "H00:0~10Vdc, H01:0~5Vdc, H02:1~5Vdc"},
        {"400001", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400001", "Voltage Range for CH1", "local:Id400001.Desc", "H00:0~10Vdc, H01:0~5Vdc, H02:1~5Vdc"},
        {"400002", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400002", "Voltage Range for CH2", "local:Id400002.Desc", "H00:0~10Vdc, H01:0~5Vdc, H02:1~5Vdc"},
        {"400003", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400003", "Voltage Range for CH3", "local:Id400003.Desc", "H00:0~10Vdc, H01:0~5Vdc, H02:1~5Vdc"},
        {"400004", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400004", "Voltage Range for CH4", "local:Id400004.Desc", "H00:0~10Vdc, H01:0~5Vdc, H02:1~5Vdc"},
        {"400005", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400005", "Voltage Range for CH5", "local:Id400005.Desc", "H00:0~10Vdc, H01:0~5Vdc, H02:1~5Vdc"},
        {"400006", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400006", "Voltage Range for CH6", "local:Id400006.Desc", "H00:0~10Vdc, H01:0~5Vdc, H02:1~5Vdc"},
        {"400007", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400007", "Voltage Range for CH7", "local:Id400007.Desc", "H00:0~10Vdc, H01:0~5Vdc, H02:1~5Vdc"},
        {"400008", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400008", "Voltage Range for CH8", "local:Id400008.Desc", "H00:0~10Vdc, H01:0~5Vdc, H02:1~5Vdc"},
        {"400009", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400009", "Voltage Range for CH9", "local:Id400009.Desc", "H00:0~10Vdc, H01:0~5Vdc, H02:1~5Vdc"},
        {"400010", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400010", "Voltage Range for CH10", "local:Id400010.Desc", "H00:0~10Vdc, H01:0~5Vdc, H02:1~5Vdc"},
        {"400011", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400011", "Voltage Range for CH11", "local:Id400011.Desc", "H00:0~10Vdc, H01:0~5Vdc, H02:1~5Vdc"},
        {"400012", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400012", "Voltage Range for CH12", "local:Id400012.Desc", "H00:0~10Vdc, H01:0~5Vdc, H02:1~5Vdc"},
        {"400013", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400013", "Voltage Range for CH13", "local:Id400013.Desc", "H00:0~10Vdc, H01:0~5Vdc, H02:1~5Vdc"},
        {"400014", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400014", "Voltage Range for CH14", "local:Id400014.Desc", "H00:0~10Vdc, H01:0~5Vdc, H02:1~5Vdc"},
        {"400015", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400015", "Voltage Range for CH15", "local:Id400015.Desc", "H00:0~10Vdc, H01:0~5Vdc, H02:1~5Vdc"},
        {"400016", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400016", "Filter Time", "local:Id400016.Desc", "H00:Default Filter(20), H01:Fastest, H3E:Slowest"},
        {"400017", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400017", "Not used", "local:Id400016.Desc", "Not used(=00)"}
    };

    for (const auto& pDef : params) {
        QDomElement paramElement = doc.createElement("Parameter");
        paramElement.setAttribute("ParameterId", pDef.id);
        paramElement.setAttribute("type", pDef.type);
        hostParamSetElement.appendChild(paramElement);

        QDomElement attributesElement = doc.createElement("Attributes");
        attributesElement.setAttribute("channel", pDef.channel);
        attributesElement.setAttribute("download", pDef.download);
        attributesElement.setAttribute("functional", pDef.functional);
        attributesElement.setAttribute("offlineaccess", pDef.offlineAccess);
        attributesElement.setAttribute("onlineaccess", pDef.onlineAccess);
        paramElement.appendChild(attributesElement);

        QDomElement defaultElement = doc.createElement("Default");
        defaultElement.appendChild(doc.createTextNode(pDef.defaultValue));
        paramElement.appendChild(defaultElement);

        QDomElement nameElement = doc.createElement("Name");
        nameElement.setAttribute("name", pDef.nameKey);
        nameElement.appendChild(doc.createTextNode(pDef.nameValue));
        paramElement.appendChild(nameElement);

        if (!pDef.descKey.isEmpty() && !pDef.descValue.isEmpty()) {
            QDomElement descElement = doc.createElement("Description");
            descElement.setAttribute("name", pDef.descKey);
            descElement.appendChild(doc.createTextNode(pDef.descValue));
            paramElement.appendChild(descElement);
        }
    }

    return doc;
}

QDomDocument createTemplateForAnalogOutput() {

    QDomDocument doc;
    // Добавляем декларацию XML
    QDomProcessingInstruction xmlDeclaration = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(xmlDeclaration);

    // Создаем корневой элемент DeviceDescription с пространствами имен
    QDomElement root = doc.createElementNS("http://www.3s-software.com/schemas/DeviceDescription-1.0.xsd", "DeviceDescription");
    root.setAttribute("xmlns", "http://www.3s-software.com/schemas/DeviceDescription-1.0.xsd");
    root.setAttribute("xmlns:ts", "http://www.3s-software.com/schemas/TargetSettings-0.1.xsd");
    root.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    doc.appendChild(root);

    QDomComment typesComment = doc.createComment("Types namespace=\"std\"></Types");
    root.appendChild(typesComment);

    QDomElement typesElement = doc.createElement("Types");
    typesElement.setAttribute("namespace", "localtype");
    root.appendChild(typesElement);

    QDomElement fileElement = doc.createElement("File");
    fileElement.setAttribute("fileref", "local");
    fileElement.setAttribute("identifier", "image_output");

    QDomElement localFileElement = doc.createElement("LocalFile");
    localFileElement.appendChild(doc.createTextNode("MyDiscreteOutput.gif"));
    fileElement.appendChild(localFileElement);

    root.appendChild(fileElement);

    // --- Добавляем Device ---
    QDomElement deviceElement = doc.createElement("Device");
    deviceElement.setAttribute("hideInCatalogue", "false");
    root.appendChild(deviceElement);

    // DeviceIdentification
    QDomElement deviceIdentElement = doc.createElement("DeviceIdentification");
    deviceElement.appendChild(deviceIdentElement);

    QDomElement typeElement = doc.createElement("Type");
    typeElement.appendChild(doc.createTextNode("40300"));
    deviceIdentElement.appendChild(typeElement);

    QDomElement idElement = doc.createElement("Id");
    //idElement.appendChild(doc.createTextNode("10ad 1b7f"));
    idElement.appendChild(doc.createTextNode("10ad myanalogoutput"));
    deviceIdentElement.appendChild(idElement);

    QDomElement versionElement = doc.createElement("Version");
    versionElement.appendChild(doc.createTextNode("2.0.0.0"));
    deviceIdentElement.appendChild(versionElement);

    // DeviceInfo
    QDomElement deviceInfoElement = doc.createElement("DeviceInfo");
    deviceElement.appendChild(deviceInfoElement);

    QDomElement nameElement = doc.createElement("Name");
    nameElement.setAttribute("name", "local:ModelName");
    //nameElement.appendChild(doc.createTextNode("GT-1B7F"));
    nameElement.appendChild(doc.createTextNode("MyAnalogOutput"));
    deviceInfoElement.appendChild(nameElement);

    QDomElement descElement = doc.createElement("Description");
    descElement.setAttribute("name", "local:DeviceDescription");
    descElement.appendChild(doc.createTextNode("Voltage Output 16CH, 0~10V, 16bit"));
    deviceInfoElement.appendChild(descElement);

    QDomElement vendorElement = doc.createElement("Vendor");
    vendorElement.setAttribute("name", "local:VendorName");
    vendorElement.appendChild(doc.createTextNode("CREVIS CO.,LTD"));
    deviceInfoElement.appendChild(vendorElement);

    QDomElement orderNumElement = doc.createElement("OrderNumber");
    //orderNumElement.appendChild(doc.createTextNode("GT-1B7F"));
    orderNumElement.appendChild(doc.createTextNode("MyAnalogOutput"));
    deviceInfoElement.appendChild(orderNumElement);

    QDomElement imageElement = doc.createElement("Image");
    imageElement.setAttribute("name", "local:image_io");
    //imageElement.appendChild(doc.createTextNode("GT-1B7F.gif"));
    imageElement.appendChild(doc.createTextNode("MyAnalogOutput.gif"));
    deviceInfoElement.appendChild(imageElement);

    QDomComment connectorComment = doc.createComment("Connector moduleType=\"257\" interface=\"Common.PCI\" role=\"child\" explicit=\"false\" connectorId=\"1\" hostpath=\"-1\">\n      <InterfaceName name=\"local:PCI\">PCI-Bus</InterfaceName>\n      <Slot count=\"1\" allowEmpty=\"false\"></Slot>\n    </Connector");
    deviceElement.appendChild(connectorComment);

    QDomElement connector1Element = doc.createElement("Connector");
    connector1Element.setAttribute("moduleType", "47000");
    connector1Element.setAttribute("interface", "CVS.OptionG");
    connector1Element.setAttribute("role", "child");
    connector1Element.setAttribute("explicit", "false");
    connector1Element.setAttribute("connectorId", "1");
    connector1Element.setAttribute("hostpath", "-1");

    QDomElement interfaceName1Element = doc.createElement("InterfaceName");
    interfaceName1Element.setAttribute("name", "local:PCI");
    interfaceName1Element.appendChild(doc.createTextNode("CVS Option"));
    connector1Element.appendChild(interfaceName1Element);

    QDomElement slotElement = doc.createElement("Slot");
    slotElement.setAttribute("count", "1");
    slotElement.setAttribute("allowEmpty", "false");
    connector1Element.appendChild(slotElement);

    deviceElement.appendChild(connector1Element);

    QDomElement connector2Element = doc.createElement("Connector");
    connector2Element.setAttribute("moduleType", "40302");
    connector2Element.setAttribute("interface", "MyCompany:Internal");
    connector2Element.setAttribute("role", "parent");
    connector2Element.setAttribute("explicit", "false");
    connector2Element.setAttribute("connectorId", "2");
    connector2Element.setAttribute("hostpath", "1");
    deviceElement.appendChild(connector2Element);

    QDomElement interfaceName2Element = doc.createElement("InterfaceName");
    interfaceName2Element.setAttribute("name", "local:DP");
    interfaceName2Element.appendChild(doc.createTextNode("Analog IOs"));
    connector2Element.appendChild(interfaceName2Element);

    QDomElement varElement = doc.createElement("Var");
    varElement.setAttribute("max", "8");
    connector2Element.appendChild(varElement);

    QDomElement driverInfoElement = doc.createElement("DriverInfo");
    driverInfoElement.setAttribute("needsBusCycle", "true");
    connector2Element.appendChild(driverInfoElement);

    // HostParameterSet
    QDomElement hostParamSetElement = doc.createElement("HostParameterSet");
    connector2Element.appendChild(hostParamSetElement);

    struct ParamDef {
        QString id;
        QString type;
        QString channel;
        QString download;
        QString functional;
        QString offlineAccess;
        QString onlineAccess;
        QString defaultValue;
        QString nameKey;
        QString nameValue;
        QString descKey;
        QString descValue;
    };

    std::vector<ParamDef> params = {
        {"4000", "std:WORD", "Output", "true", "false", "readwrite", "readwrite", "0", "local:out0", "AO0", "", ""},
        {"4001", "std:WORD", "Output", "true", "false", "readwrite", "readwrite", "0", "local:out1", "AO1", "", ""},
        {"4002", "std:WORD", "Output", "true", "false", "readwrite", "readwrite", "0", "local:out2", "AO2", "", ""},
        {"4003", "std:WORD", "Output", "true", "false", "readwrite", "readwrite", "0", "local:out3", "AO3", "", ""},
        {"4004", "std:WORD", "Output", "true", "false", "readwrite", "readwrite", "0", "local:out4", "AO4", "", ""},
        {"4005", "std:WORD", "Output", "true", "false", "readwrite", "readwrite", "0", "local:out5", "AO5", "", ""},
        {"4006", "std:WORD", "Output", "true", "false", "readwrite", "readwrite", "0", "local:out6", "AO6", "", ""},
        {"4007", "std:WORD", "Output", "true", "false", "readwrite", "readwrite", "0", "local:out7", "AO7", "", ""},
        {"4008", "std:WORD", "Output", "true", "false", "readwrite", "readwrite", "0", "local:out8", "AO8", "", ""},
        {"4009", "std:WORD", "Output", "true", "false", "readwrite", "readwrite", "0", "local:out9", "AO9", "", ""},
        {"4010", "std:WORD", "Output", "true", "false", "readwrite", "readwrite", "0", "local:out10", "AO10", "", ""},
        {"4011", "std:WORD", "Output", "true", "false", "readwrite", "readwrite", "0", "local:out11", "AO11", "", ""},
        {"4012", "std:WORD", "Output", "true", "false", "readwrite", "readwrite", "0", "local:out12", "AO12", "", ""},
        {"4013", "std:WORD", "Output", "true", "false", "readwrite", "readwrite", "0", "local:out13", "AO13", "", ""},
        {"4014", "std:WORD", "Output", "true", "false", "readwrite", "readwrite", "0", "local:out14", "AO14", "", ""},
        {"4015", "std:WORD", "Output", "true", "false", "readwrite", "readwrite", "0", "local:out15", "AO15", "", ""},
        {"393218", "std:STRING", "none", "true", "false", "read", "read", "'CREVIS'", "local:Id393218", "Vendor", "local:Id393218.Desc", "Vendor of the device"},
        {"393219", "std:STRING", "none", "true", "false", "read", "read", "'MyAnalogOutput'", "local:Id393219", "Module ID", "local:Id393219.Desc", "Module ID of the device"},
        {"400000", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400000", "Fault Action(CH0~CH3)", "local:Id400000.Desc", "00:Falut Value, 01:Hold Last State, 10:Low Limit, 11:High Limit"},
        {"400001", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400001", "Fault Action(CH4~CH7)", "local:Id400001.Desc", "00:Falut Value, 01:Hold Last State, 10:Low Limit, 11:High Limit"},
        {"400002", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400002", "Fault Action(CH8~CH11)", "local:Id400002.Desc", "00:Falut Value, 01:Hold Last State, 10:Low Limit, 11:High Limit"},
        {"400003", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400003", "Fault Action(CH12~CH15)", "local:Id400003.Desc", "00:Falut Value, 01:Hold Last State, 10:Low Limit, 11:High Limit"},
        {"400004", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400004", "Fault Value Low Byte", "local:Id400004.Desc", ""},
        {"400005", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400005", "Fault Value High Byte", "local:Id400005.Desc", ""},
    };

    for (const auto& pDef : params) {
        QDomElement paramElement = doc.createElement("Parameter");
        paramElement.setAttribute("ParameterId", pDef.id);
        paramElement.setAttribute("type", pDef.type);
        hostParamSetElement.appendChild(paramElement);

        QDomElement attributesElement = doc.createElement("Attributes");
        attributesElement.setAttribute("channel", pDef.channel);
        attributesElement.setAttribute("download", pDef.download);
        attributesElement.setAttribute("functional", pDef.functional);
        attributesElement.setAttribute("offlineaccess", pDef.offlineAccess);
        attributesElement.setAttribute("onlineaccess", pDef.onlineAccess);
        paramElement.appendChild(attributesElement);

        QDomElement defaultElement = doc.createElement("Default");
        defaultElement.appendChild(doc.createTextNode(pDef.defaultValue));
        paramElement.appendChild(defaultElement);

        QDomElement nameElement = doc.createElement("Name");
        nameElement.setAttribute("name", pDef.nameKey);
        nameElement.appendChild(doc.createTextNode(pDef.nameValue));
        paramElement.appendChild(nameElement);

        if (!pDef.descKey.isEmpty() && !pDef.descValue.isEmpty()) {
            QDomElement descElement = doc.createElement("Description");
            descElement.setAttribute("name", pDef.descKey);
            descElement.appendChild(doc.createTextNode(pDef.descValue));
            paramElement.appendChild(descElement);
        }
    }

    return doc;
}

QDomDocument createTemplateForSpecialized() {
    QDomDocument doc;
    QDomProcessingInstruction xmlDeclaration = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(xmlDeclaration);

    QDomElement root = doc.createElementNS("http://www.3s-software.com/schemas/DeviceDescription-1.0.xsd", "DeviceDescription");
    root.setAttribute("xmlns", "http://www.3s-software.com/schemas/DeviceDescription-1.0.xsd");
    root.setAttribute("xmlns:ts", "http://www.3s-software.com/schemas/TargetSettings-0.1.xsd");
    root.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    doc.appendChild(root);

    QDomElement deviceElement = doc.createElement("Device");
    deviceElement.setAttribute("hideInCatalogue", "false");
    root.appendChild(deviceElement);

    QDomElement deviceIdentElement = doc.createElement("DeviceIdentification");
    deviceElement.appendChild(deviceIdentElement);

    QDomElement typeElement = doc.createElement("Type");
    typeElement.appendChild(doc.createTextNode("50006"));
    deviceIdentElement.appendChild(typeElement);

    QDomElement idElement = doc.createElement("Id");
    idElement.appendChild(doc.createTextNode("MY_VENDOR 0006"));
    deviceIdentElement.appendChild(idElement);

    QDomElement versionElement = doc.createElement("Version");
    versionElement.appendChild(doc.createTextNode("1.0.0.0"));
    deviceIdentElement.appendChild(versionElement);

    QDomElement deviceInfoElement = doc.createElement("DeviceInfo");
    deviceElement.appendChild(deviceInfoElement);

    QDomElement nameElement = doc.createElement("Name");
    nameElement.setAttribute("name", "local:Specialized.Name");
    nameElement.appendChild(doc.createTextNode("MySpecialized"));
    deviceInfoElement.appendChild(nameElement);

    QDomElement descElement = doc.createElement("Description");
    descElement.setAttribute("name", "local:Specialized.Desc");
    descElement.appendChild(doc.createTextNode("Generic Specialized Module (e.g., Encoder, PWM)"));
    deviceInfoElement.appendChild(descElement);

    QDomElement vendorElement = doc.createElement("Vendor");
    vendorElement.setAttribute("name", "local:VendorName");
    vendorElement.appendChild(doc.createTextNode("Your Company"));
    deviceInfoElement.appendChild(vendorElement);

    QDomElement orderNumElement = doc.createElement("OrderNumber");
    orderNumElement.appendChild(doc.createTextNode("SPEC-GEN-001"));
    deviceInfoElement.appendChild(orderNumElement);

    // Connector для связи с родительским устройством
    QDomElement connectorElement = doc.createElement("Connector");
    connectorElement.setAttribute("moduleType", "0");
    connectorElement.setAttribute("interface", "MyCompany:Backplane");
    connectorElement.setAttribute("role", "child");
    connectorElement.setAttribute("explicit", "false");
    connectorElement.setAttribute("connectorId", "1");
    connectorElement.setAttribute("hostpath", "-1");
    deviceElement.appendChild(connectorElement);

    QDomElement interfaceNameElement = doc.createElement("InterfaceName");
    interfaceNameElement.setAttribute("name", "local:Backplane");
    interfaceNameElement.appendChild(doc.createTextNode("Backplane"));
    connectorElement.appendChild(interfaceNameElement);

    QDomElement slotElement = doc.createElement("Slot");
    slotElement.setAttribute("count", "1");
    slotElement.setAttribute("allowEmpty", "false");
    connectorElement.appendChild(slotElement);

    // HostParameterSet (если нужен для конфигурации)
    QDomElement hostParamSetElement = doc.createElement("HostParameterSet");
    connectorElement.appendChild(hostParamSetElement);

    // Пример параметра для типа функции (Encoder, PWM)
    QDomElement paramElement = doc.createElement("Parameter");
    paramElement.setAttribute("ParameterId", "1000");
    paramElement.setAttribute("type", "std:UDINT");
    hostParamSetElement.appendChild(paramElement);

    QDomElement nameParamElement = doc.createElement("Name");
    nameParamElement.setAttribute("name", "local:FunctionType");
    nameParamElement.appendChild(doc.createTextNode("Function Type"));
    paramElement.appendChild(nameParamElement);

    QDomElement defaultParamElement = doc.createElement("Default");
    defaultParamElement.appendChild(doc.createTextNode("0")); // 0 - Encoder, 1 - PWM
    paramElement.appendChild(defaultParamElement);

    QDomElement attributesElement = doc.createElement("Attributes");
    attributesElement.setAttribute("channel", "none");
    attributesElement.setAttribute("download", "true");
    attributesElement.setAttribute("functional", "false");
    attributesElement.setAttribute("offlineaccess", "readwrite");
    attributesElement.setAttribute("onlineaccess", "read");
    paramElement.appendChild(attributesElement);

    return doc;
}

QDomDocument createTemplateForPower() {
    QDomDocument doc;
    // Добавляем декларацию XML
    QDomProcessingInstruction xmlDeclaration = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(xmlDeclaration);

    // Создаем корневой элемент DeviceDescription с пространствами имен
    QDomElement root = doc.createElementNS("http://www.3s-software.com/schemas/DeviceDescription-1.0.xsd", "DeviceDescription");
    root.setAttribute("xmlns", "http://www.3s-software.com/schemas/DeviceDescription-1.0.xsd");
    root.setAttribute("xmlns:ts", "http://www.3s-software.com/schemas/TargetSettings-0.1.xsd");
    root.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    doc.appendChild(root);

    QDomComment typesComment = doc.createComment("Types namespace=\"std\"></Types");
    root.appendChild(typesComment);

    QDomElement typesElement = doc.createElement("Types");
    typesElement.setAttribute("namespace", "localtype");
    root.appendChild(typesElement);

    QDomElement fileElement = doc.createElement("File");
    fileElement.setAttribute("fileref", "local");
    fileElement.setAttribute("identifier", "image_io");

    QDomElement localFileElement = doc.createElement("LocalFile");
    localFileElement.appendChild(doc.createTextNode("MyPowerSupply.gif"));
    fileElement.appendChild(localFileElement);

    root.appendChild(fileElement);

    // --- Добавляем Device ---
    QDomElement deviceElement = doc.createElement("Device");
    deviceElement.setAttribute("hideInCatalogue", "false");
    root.appendChild(deviceElement);

    // DeviceIdentification
    QDomElement deviceIdentElement = doc.createElement("DeviceIdentification");
    deviceElement.appendChild(deviceIdentElement);

    QDomElement typeElement = doc.createElement("Type");
    typeElement.appendChild(doc.createTextNode("40304"));
    deviceIdentElement.appendChild(typeElement);

    QDomElement idElement = doc.createElement("Id");
    //idElement.appendChild(doc.createTextNode("10ad 1b7f"));
    idElement.appendChild(doc.createTextNode("10ad mypowersupply"));
    deviceIdentElement.appendChild(idElement);

    QDomElement versionElement = doc.createElement("Version");
    versionElement.appendChild(doc.createTextNode("2.0.0.0"));
    deviceIdentElement.appendChild(versionElement);

    // DeviceInfo
    QDomElement deviceInfoElement = doc.createElement("DeviceInfo");
    deviceElement.appendChild(deviceInfoElement);

    QDomElement nameElement = doc.createElement("Name");
    nameElement.setAttribute("name", "local:ModelName");
    //nameElement.appendChild(doc.createTextNode("GT-1B7F"));
    nameElement.appendChild(doc.createTextNode("MyPowerSupply"));
    deviceInfoElement.appendChild(nameElement);

    QDomElement descElement = doc.createElement("Description");
    descElement.setAttribute("name", "local:DeviceDescription");
    descElement.appendChild(doc.createTextNode("Common 24Vdc, ID Type"));
    deviceInfoElement.appendChild(descElement);

    QDomElement vendorElement = doc.createElement("Vendor");
    vendorElement.setAttribute("name", "local:VendorName");
    vendorElement.appendChild(doc.createTextNode("CREVIS CO.,LTD"));
    deviceInfoElement.appendChild(vendorElement);

    QDomElement orderNumElement = doc.createElement("OrderNumber");
    //orderNumElement.appendChild(doc.createTextNode("GT-1B7F"));
    orderNumElement.appendChild(doc.createTextNode("MyPowerSupply"));
    deviceInfoElement.appendChild(orderNumElement);

    QDomElement imageElement = doc.createElement("Image");
    imageElement.setAttribute("name", "local:image_io");
    //imageElement.appendChild(doc.createTextNode("GT-1B7F.gif"));
    imageElement.appendChild(doc.createTextNode("MyPowerSupply.gif"));
    deviceInfoElement.appendChild(imageElement);

    QDomComment connectorComment = doc.createComment("Connector moduleType=\"257\" interface=\"Common.PCI\" role=\"child\" explicit=\"false\" connectorId=\"1\" hostpath=\"-1\">\n      <InterfaceName name=\"local:PCI\">PCI-Bus</InterfaceName>\n      <Slot count=\"1\" allowEmpty=\"false\"></Slot>\n    </Connector");
    deviceElement.appendChild(connectorComment);

    QDomElement connector1Element = doc.createElement("Connector");
    connector1Element.setAttribute("moduleType", "47000");
    connector1Element.setAttribute("interface", "CVS.OptionG");
    connector1Element.setAttribute("role", "child");
    connector1Element.setAttribute("explicit", "false");
    connector1Element.setAttribute("connectorId", "1");
    connector1Element.setAttribute("hostpath", "-1");

    QDomElement interfaceName1Element = doc.createElement("InterfaceName");
    interfaceName1Element.setAttribute("name", "local:PCI");
    interfaceName1Element.appendChild(doc.createTextNode("CVS Option"));
    connector1Element.appendChild(interfaceName1Element);

    QDomElement slotElement = doc.createElement("Slot");
    slotElement.setAttribute("count", "1");
    slotElement.setAttribute("allowEmpty", "false");
    connector1Element.appendChild(slotElement);

    deviceElement.appendChild(connector1Element);

    QDomElement connector2Element = doc.createElement("Connector");
    connector2Element.setAttribute("moduleType", "40304");
    connector2Element.setAttribute("interface", "MyCompany:Internal");
    connector2Element.setAttribute("role", "parent");
    connector2Element.setAttribute("explicit", "false");
    connector2Element.setAttribute("connectorId", "2");
    connector2Element.setAttribute("hostpath", "1");
    deviceElement.appendChild(connector2Element);

    QDomElement interfaceName2Element = doc.createElement("InterfaceName");
    interfaceName2Element.setAttribute("name", "local:DP");
    interfaceName2Element.appendChild(doc.createTextNode("Power IOs"));
    connector2Element.appendChild(interfaceName2Element);

    QDomElement varElement = doc.createElement("Var");
    varElement.setAttribute("max", "8");
    connector2Element.appendChild(varElement);

    QDomElement driverInfoElement = doc.createElement("DriverInfo");
    driverInfoElement.setAttribute("needsBusCycle", "true");
    connector2Element.appendChild(driverInfoElement);

    // HostParameterSet
    QDomElement hostParamSetElement = doc.createElement("HostParameterSet");
    connector2Element.appendChild(hostParamSetElement);

    struct ParamDef {
        QString id;
        QString type;
        QString channel;
        QString download;
        QString functional;
        QString offlineAccess;
        QString onlineAccess;
        QString defaultValue;
        QString nameKey;
        QString nameValue;
        QString descKey;
        QString descValue;
    };

    std::vector<ParamDef> params = {
        {"393218", "std:STRING", "none", "true", "false", "read", "read", "'CREVIS'", "local:Id393218", "Vendor", "local:Id393218.Desc", "Vendor of the device"},
        {"393219", "std:STRING", "none", "true", "false", "read", "read", "'MyPowerSupply'", "local:Id393219", "Module ID", "local:Id393219.Desc", "Module ID of the device"}
    };

    for (const auto& pDef : params) {
        QDomElement paramElement = doc.createElement("Parameter");
        paramElement.setAttribute("ParameterId", pDef.id);
        paramElement.setAttribute("type", pDef.type);
        hostParamSetElement.appendChild(paramElement);

        QDomElement attributesElement = doc.createElement("Attributes");
        attributesElement.setAttribute("channel", pDef.channel);
        attributesElement.setAttribute("download", pDef.download);
        attributesElement.setAttribute("functional", pDef.functional);
        attributesElement.setAttribute("offlineaccess", pDef.offlineAccess);
        attributesElement.setAttribute("onlineaccess", pDef.onlineAccess);
        paramElement.appendChild(attributesElement);

        QDomElement defaultElement = doc.createElement("Default");
        defaultElement.appendChild(doc.createTextNode(pDef.defaultValue));
        paramElement.appendChild(defaultElement);

        QDomElement nameElement = doc.createElement("Name");
        nameElement.setAttribute("name", pDef.nameKey);
        nameElement.appendChild(doc.createTextNode(pDef.nameValue));
        paramElement.appendChild(nameElement);

        if (!pDef.descKey.isEmpty() && !pDef.descValue.isEmpty()) {
            QDomElement descElement = doc.createElement("Description");
            descElement.setAttribute("name", pDef.descKey);
            descElement.appendChild(doc.createTextNode(pDef.descValue));
            paramElement.appendChild(descElement);
        }
    }

    return doc;
}

QDomDocument createTemplateForRS232() {

    QDomDocument doc;
    // Добавляем декларацию XML
    QDomProcessingInstruction xmlDeclaration = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(xmlDeclaration);

    // Создаем корневой элемент DeviceDescription с пространствами имен
    QDomElement root = doc.createElementNS("http://www.3s-software.com/schemas/DeviceDescription-1.0.xsd", "DeviceDescription");
    root.setAttribute("xmlns", "http://www.3s-software.com/schemas/DeviceDescription-1.0.xsd");
    root.setAttribute("xmlns:ts", "http://www.3s-software.com/schemas/TargetSettings-0.1.xsd");
    root.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    doc.appendChild(root);

    QDomComment typesComment = doc.createComment("Types namespace=\"std\"></Types");
    root.appendChild(typesComment);

    // --- Добавляем Types ---
    QDomElement typesElement = doc.createElement("Types");
    typesElement.setAttribute("namespace", "localtype");
    root.appendChild(typesElement);


    QDomElement rangeTypeElement = doc.createElement("RangeType");
    rangeTypeElement.setAttribute("basetype", "std:UINT"); // Пример: базовый тип UINT
    rangeTypeElement.setAttribute("name", "AssemblySizeRangeType"); // Пример: название диапазона
    typesElement.appendChild(rangeTypeElement);

    QDomElement minElement = doc.createElement("Min");
    minElement.appendChild(doc.createTextNode("4")); // Пример: минимальное значение
    rangeTypeElement.appendChild(minElement);

    QDomElement maxElement = doc.createElement("Max");
    maxElement.appendChild(doc.createTextNode("1024")); // Пример: максимальное значение
    rangeTypeElement.appendChild(maxElement);

    QDomElement defaultElement = doc.createElement("Default");
    defaultElement.appendChild(doc.createTextNode("4")); // Пример: значение по умолчанию
    rangeTypeElement.appendChild(defaultElement);


    // Добавляем BitfieldType TBit1Byte
    QDomElement bitfieldTypeElement = doc.createElement("BitfieldType");
    bitfieldTypeElement.setAttribute("basetype", "std:BYTE");
    bitfieldTypeElement.setAttribute("name", "TBit1Byte");
    typesElement.appendChild(bitfieldTypeElement);

    // Добавляем компоненты Bit0 - Bit7
    for (int i = 0; i < 8; ++i) {
        QDomElement componentElement = doc.createElement("Component");
        componentElement.setAttribute("identifier", QString("Bit%1").arg(i));
        componentElement.setAttribute("type", "std:BOOL");

        QDomElement defaultElement = doc.createElement("Default");
        componentElement.appendChild(defaultElement);

        QDomElement visibleNameElement = doc.createElement("VisibleName");
        visibleNameElement.setAttribute("name", QString("localtype:TBit1Byte.Bit%1").arg(i));
        visibleNameElement.appendChild(doc.createTextNode(QString("BIT%1").arg(i)));
        componentElement.appendChild(visibleNameElement);

        bitfieldTypeElement.appendChild(componentElement);
    }
    // --- /Добавляем Types ---

    // --- Добавляем File ---
    QDomElement fileElement = doc.createElement("File");
    fileElement.setAttribute("fileref", "local");
    fileElement.setAttribute("identifier", "image_io");

    QDomElement localFileElement = doc.createElement("LocalFile");
    localFileElement.appendChild(doc.createTextNode("MyRS232.gif"));
    fileElement.appendChild(localFileElement);

    root.appendChild(fileElement);
    // --- /Добавляем File ---

    // --- Добавляем Device ---
    QDomElement deviceElement = doc.createElement("Device");
    deviceElement.setAttribute("hideInCatalogue", "false");
    root.appendChild(deviceElement);

    // DeviceIdentification
    QDomElement deviceIdentElement = doc.createElement("DeviceIdentification");
    deviceElement.appendChild(deviceIdentElement);

    QDomElement typeElement = doc.createElement("Type");
    typeElement.appendChild(doc.createTextNode("40305"));
    deviceIdentElement.appendChild(typeElement);

    QDomElement idElement = doc.createElement("Id");
    //idElement.appendChild(doc.createTextNode("10ad 1b7f"));
    idElement.appendChild(doc.createTextNode("10ad myrs232"));
    deviceIdentElement.appendChild(idElement);

    QDomElement versionElement = doc.createElement("Version");
    versionElement.appendChild(doc.createTextNode("2.0.0.0"));
    deviceIdentElement.appendChild(versionElement);

    // DeviceInfo
    QDomElement deviceInfoElement = doc.createElement("DeviceInfo");
    deviceElement.appendChild(deviceInfoElement);

    QDomElement nameElement = doc.createElement("Name");
    nameElement.setAttribute("name", "local:ModelName");
    //nameElement.appendChild(doc.createTextNode("GT-1B7F"));
    nameElement.appendChild(doc.createTextNode("MyRS232"));
    deviceInfoElement.appendChild(nameElement);

    QDomElement descElement = doc.createElement("Description");
    descElement.setAttribute("name", "local:DeviceDescription");
    descElement.appendChild(doc.createTextNode("RS-232 Serial Interface 1CH, RTS/CTS"));
    deviceInfoElement.appendChild(descElement);

    QDomElement vendorElement = doc.createElement("Vendor");
    vendorElement.setAttribute("name", "local:VendorName");
    vendorElement.appendChild(doc.createTextNode("CREVIS CO.,LTD"));
    deviceInfoElement.appendChild(vendorElement);

    QDomElement orderNumElement = doc.createElement("OrderNumber");
    //orderNumElement.appendChild(doc.createTextNode("GT-1B7F"));
    orderNumElement.appendChild(doc.createTextNode("MyRS232"));
    deviceInfoElement.appendChild(orderNumElement);

    QDomElement imageElement = doc.createElement("Image");
    imageElement.setAttribute("name", "local:image_io");
    //imageElement.appendChild(doc.createTextNode("GT-1B7F.gif"));
    imageElement.appendChild(doc.createTextNode("MyRS232.gif"));
    deviceInfoElement.appendChild(imageElement);

    // --- Добавляем первый Connector (child) ---
    QDomComment connectorComment = doc.createComment("Connector moduleType=\"257\" interface=\"Common.PCI\" role=\"child\" explicit=\"false\" connectorId=\"1\" hostpath=\"-1\">\n      <InterfaceName name=\"local:PCI\">PCI-Bus</InterfaceName>\n      <Slot count=\"1\" allowEmpty=\"false\"></Slot>\n    </Connector");
    deviceElement.appendChild(connectorComment);

    QDomElement connector1Element = doc.createElement("Connector");
    connector1Element.setAttribute("moduleType", "47000");
    connector1Element.setAttribute("interface", "CVS.OptionG");
    connector1Element.setAttribute("role", "child");
    connector1Element.setAttribute("explicit", "false");
    connector1Element.setAttribute("connectorId", "1");
    connector1Element.setAttribute("hostpath", "-1");

    QDomElement interfaceName1Element = doc.createElement("InterfaceName");
    interfaceName1Element.setAttribute("name", "local:PCI");
    interfaceName1Element.appendChild(doc.createTextNode("CVS Option"));
    connector1Element.appendChild(interfaceName1Element);

    QDomElement slotElement = doc.createElement("Slot");
    slotElement.setAttribute("count", "1");
    slotElement.setAttribute("allowEmpty", "false");
    connector1Element.appendChild(slotElement);

    deviceElement.appendChild(connector1Element);
    // --- /Добавляем первый Connector (child) ---

    // --- Добавляем второй Connector (parent) ---
    QDomElement connector2Element = doc.createElement("Connector");
    connector2Element.setAttribute("moduleType", "40305");
    connector2Element.setAttribute("interface", "MyCompany:Internal");
    connector2Element.setAttribute("role", "parent");
    connector2Element.setAttribute("explicit", "false");
    connector2Element.setAttribute("connectorId", "2");
    connector2Element.setAttribute("hostpath", "1");
    deviceElement.appendChild(connector2Element);

    QDomElement interfaceName2Element = doc.createElement("InterfaceName");
    interfaceName2Element.setAttribute("name", "local:DP");
    interfaceName2Element.appendChild(doc.createTextNode("ETC IOs"));
    connector2Element.appendChild(interfaceName2Element);

    QDomElement varElement = doc.createElement("Var");
    varElement.setAttribute("max", "8");
    connector2Element.appendChild(varElement);

    QDomElement driverInfoElement = doc.createElement("DriverInfo");
    driverInfoElement.setAttribute("needsBusCycle", "true");
    connector2Element.appendChild(driverInfoElement);

    // HostParameterSet
    QDomElement hostParamSetElement = doc.createElement("HostParameterSet");
    connector2Element.appendChild(hostParamSetElement);

    struct ParamDef {
        QString id;
        QString type;
        QString channel;
        QString download;
        QString functional;
        QString offlineAccess;
        QString onlineAccess;
        QString defaultValue;
        QString nameKey;
        QString nameValue;
        QString descKey;
        QString descValue;
    };

    std::vector<ParamDef> params = {
        {"7000", "localTypes:ARRAY [0..15] OF TBit1Byte", "input", "true", "false", "read", "read", "0", "local:in0", "IN", "", ""},
        {"8000", "localTypes:ARRAY [0..15] OF TBit1Byte", "output", "true", "false", "readwrite", "readwrite", "0", "local:out0", "OUT", "", ""},
        {"393218", "std:STRING", "none", "true", "false", "read", "read", "'CREVIS'", "local:Id393218", "Vendor", "local:Id393218.Desc", "Vendor of the device"},
        {"393219", "std:STRING", "none", "true", "false", "read", "read", "'MyRS232'", "local:Id393219", "Module ID", "local:Id393219.Desc", "Module ID of the device"},
        {"400000", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400000", "Parameter0", "local:Id400000.Desc", "Bardrate[0-3], Parity Bit[4-5], Stop Bit[6], TxD Process[7]"},
        {"400001", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400001", "Parameter1", "local:Id400001.Desc", "Flow Control[0-1], Data Bit[2-7]:16~63"},
        {"400002", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400002", "Reserved", "local:Id400002.Desc", "Reserved"},
        {"400003", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400003", "Reserved", "local:Id400003.Desc", "Reserved"},
    };

    for (const auto& pDef : params) {
        QDomElement paramElement = doc.createElement("Parameter");
        paramElement.setAttribute("ParameterId", pDef.id);
        paramElement.setAttribute("type", pDef.type);
        hostParamSetElement.appendChild(paramElement);

        QDomElement attributesElement = doc.createElement("Attributes");
        attributesElement.setAttribute("channel", pDef.channel);
        attributesElement.setAttribute("download", pDef.download);
        attributesElement.setAttribute("functional", pDef.functional);
        attributesElement.setAttribute("offlineaccess", pDef.offlineAccess);
        attributesElement.setAttribute("onlineaccess", pDef.onlineAccess);
        paramElement.appendChild(attributesElement);

        QDomElement defaultElement = doc.createElement("Default");
        defaultElement.appendChild(doc.createTextNode(pDef.defaultValue));
        paramElement.appendChild(defaultElement);

        QDomElement nameElement = doc.createElement("Name");
        nameElement.setAttribute("name", pDef.nameKey);
        nameElement.appendChild(doc.createTextNode(pDef.nameValue));
        paramElement.appendChild(nameElement);

        if (!pDef.descKey.isEmpty() && !pDef.descValue.isEmpty()) {
            QDomElement descElement = doc.createElement("Description");
            descElement.setAttribute("name", pDef.descKey);
            descElement.appendChild(doc.createTextNode(pDef.descValue));
            paramElement.appendChild(descElement);
        }
    }
    // --- /Добавляем второй Connector (parent) ---

    return doc;
}

QDomDocument createTemplateForRS422() {

    QDomDocument doc;
    // Добавляем декларацию XML
    QDomProcessingInstruction xmlDeclaration = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(xmlDeclaration);

    // Создаем корневой элемент DeviceDescription с пространствами имен
    QDomElement root = doc.createElementNS("http://www.3s-software.com/schemas/DeviceDescription-1.0.xsd", "DeviceDescription");
    root.setAttribute("xmlns", "http://www.3s-software.com/schemas/DeviceDescription-1.0.xsd");
    root.setAttribute("xmlns:ts", "http://www.3s-software.com/schemas/TargetSettings-0.1.xsd");
    root.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    doc.appendChild(root);

    QDomComment typesComment = doc.createComment("Types namespace=\"std\"></Types");
    root.appendChild(typesComment);

    // --- Добавляем Types ---
    QDomElement typesElement = doc.createElement("Types");
    typesElement.setAttribute("namespace", "localtype");
    root.appendChild(typesElement);

    QDomElement rangeTypeElement = doc.createElement("RangeType");
    rangeTypeElement.setAttribute("basetype", "std:UINT"); // Пример: базовый тип UINT
    rangeTypeElement.setAttribute("name", "AssemblySizeRangeType"); // Пример: название диапазона
    typesElement.appendChild(rangeTypeElement);

    QDomElement minElement = doc.createElement("Min");
    minElement.appendChild(doc.createTextNode("4")); // Пример: минимальное значение
    rangeTypeElement.appendChild(minElement);

    QDomElement maxElement = doc.createElement("Max");
    maxElement.appendChild(doc.createTextNode("1024")); // Пример: максимальное значение
    rangeTypeElement.appendChild(maxElement);

    QDomElement defaultElement = doc.createElement("Default");
    defaultElement.appendChild(doc.createTextNode("4")); // Пример: значение по умолчанию
    rangeTypeElement.appendChild(defaultElement);

    // Добавляем BitfieldType TBit1Byte
    QDomElement bitfieldTypeElement = doc.createElement("BitfieldType");
    bitfieldTypeElement.setAttribute("basetype", "std:BYTE");
    bitfieldTypeElement.setAttribute("name", "TBit1Byte");
    typesElement.appendChild(bitfieldTypeElement);

    // Добавляем компоненты Bit0 - Bit7
    for (int i = 0; i < 8; ++i) {
        QDomElement componentElement = doc.createElement("Component");
        componentElement.setAttribute("identifier", QString("Bit%1").arg(i));
        componentElement.setAttribute("type", "std:BOOL");

        QDomElement defaultElement = doc.createElement("Default");
        componentElement.appendChild(defaultElement);

        QDomElement visibleNameElement = doc.createElement("VisibleName");
        visibleNameElement.setAttribute("name", QString("localtype:TBit1Byte.Bit%1").arg(i));
        visibleNameElement.appendChild(doc.createTextNode(QString("BIT%1").arg(i)));
        componentElement.appendChild(visibleNameElement);

        bitfieldTypeElement.appendChild(componentElement);
    }
    // --- /Добавляем Types ---

    // --- Добавляем File ---
    QDomElement fileElement = doc.createElement("File");
    fileElement.setAttribute("fileref", "local");
    fileElement.setAttribute("identifier", "image_io");

    QDomElement localFileElement = doc.createElement("LocalFile");
    localFileElement.appendChild(doc.createTextNode("MyRS232.gif"));
    fileElement.appendChild(localFileElement);

    root.appendChild(fileElement);
    // --- /Добавляем File ---

    // --- Добавляем Device ---
    QDomElement deviceElement = doc.createElement("Device");
    deviceElement.setAttribute("hideInCatalogue", "false");
    root.appendChild(deviceElement);

    // DeviceIdentification
    QDomElement deviceIdentElement = doc.createElement("DeviceIdentification");
    deviceElement.appendChild(deviceIdentElement);

    QDomElement typeElement = doc.createElement("Type");
    typeElement.appendChild(doc.createTextNode("40305"));
    deviceIdentElement.appendChild(typeElement);

    QDomElement idElement = doc.createElement("Id");
    //idElement.appendChild(doc.createTextNode("10ad 1b7f"));
    idElement.appendChild(doc.createTextNode("10ad myrs422"));
    deviceIdentElement.appendChild(idElement);

    QDomElement versionElement = doc.createElement("Version");
    versionElement.appendChild(doc.createTextNode("2.0.0.0"));
    deviceIdentElement.appendChild(versionElement);

    // DeviceInfo
    QDomElement deviceInfoElement = doc.createElement("DeviceInfo");
    deviceElement.appendChild(deviceInfoElement);

    QDomElement nameElement = doc.createElement("Name");
    nameElement.setAttribute("name", "local:ModelName");
    //nameElement.appendChild(doc.createTextNode("GT-1B7F"));
    nameElement.appendChild(doc.createTextNode("MyRS422"));
    deviceInfoElement.appendChild(nameElement);

    QDomElement descElement = doc.createElement("Description");
    descElement.setAttribute("name", "local:DeviceDescription");
    descElement.appendChild(doc.createTextNode("RS-422 Serial Interface 1CH"));
    deviceInfoElement.appendChild(descElement);

    QDomElement vendorElement = doc.createElement("Vendor");
    vendorElement.setAttribute("name", "local:VendorName");
    vendorElement.appendChild(doc.createTextNode("CREVIS CO.,LTD"));
    deviceInfoElement.appendChild(vendorElement);

    QDomElement orderNumElement = doc.createElement("OrderNumber");
    //orderNumElement.appendChild(doc.createTextNode("GT-1B7F"));
    orderNumElement.appendChild(doc.createTextNode("MyRS422"));
    deviceInfoElement.appendChild(orderNumElement);

    QDomElement imageElement = doc.createElement("Image");
    imageElement.setAttribute("name", "local:image_io");
    //imageElement.appendChild(doc.createTextNode("GT-1B7F.gif"));
    imageElement.appendChild(doc.createTextNode("MyRS422.gif"));
    deviceInfoElement.appendChild(imageElement);

    // --- Добавляем первый Connector (child) ---
    QDomComment connectorComment = doc.createComment("Connector moduleType=\"257\" interface=\"Common.PCI\" role=\"child\" explicit=\"false\" connectorId=\"1\" hostpath=\"-1\">\n      <InterfaceName name=\"local:PCI\">PCI-Bus</InterfaceName>\n      <Slot count=\"1\" allowEmpty=\"false\"></Slot>\n    </Connector");
    deviceElement.appendChild(connectorComment);

    QDomElement connector1Element = doc.createElement("Connector");
    connector1Element.setAttribute("moduleType", "47000");
    connector1Element.setAttribute("interface", "CVS.OptionG");
    connector1Element.setAttribute("role", "child");
    connector1Element.setAttribute("explicit", "false");
    connector1Element.setAttribute("connectorId", "1");
    connector1Element.setAttribute("hostpath", "-1");

    QDomElement interfaceName1Element = doc.createElement("InterfaceName");
    interfaceName1Element.setAttribute("name", "local:PCI");
    interfaceName1Element.appendChild(doc.createTextNode("CVS Option"));
    connector1Element.appendChild(interfaceName1Element);

    QDomElement slotElement = doc.createElement("Slot");
    slotElement.setAttribute("count", "1");
    slotElement.setAttribute("allowEmpty", "false");
    connector1Element.appendChild(slotElement);

    deviceElement.appendChild(connector1Element);
    // --- /Добавляем первый Connector (child) ---

    // --- Добавляем второй Connector (parent) ---
    QDomElement connector2Element = doc.createElement("Connector");
    connector2Element.setAttribute("moduleType", "40305");
    connector2Element.setAttribute("interface", "MyCompany:Internal");
    connector2Element.setAttribute("role", "parent");
    connector2Element.setAttribute("explicit", "false");
    connector2Element.setAttribute("connectorId", "2");
    connector2Element.setAttribute("hostpath", "1");
    deviceElement.appendChild(connector2Element);

    QDomElement interfaceName2Element = doc.createElement("InterfaceName");
    interfaceName2Element.setAttribute("name", "local:DP");
    interfaceName2Element.appendChild(doc.createTextNode("ETC IOs"));
    connector2Element.appendChild(interfaceName2Element);

    QDomElement varElement = doc.createElement("Var");
    varElement.setAttribute("max", "8");
    connector2Element.appendChild(varElement);

    QDomElement driverInfoElement = doc.createElement("DriverInfo");
    driverInfoElement.setAttribute("needsBusCycle", "true");
    connector2Element.appendChild(driverInfoElement);

    // HostParameterSet
    QDomElement hostParamSetElement = doc.createElement("HostParameterSet");
    connector2Element.appendChild(hostParamSetElement);

    struct ParamDef {
        QString id;
        QString type;
        QString channel;
        QString download;
        QString functional;
        QString offlineAccess;
        QString onlineAccess;
        QString defaultValue;
        QString nameKey;
        QString nameValue;
        QString descKey;
        QString descValue;
    };

    std::vector<ParamDef> params = {
        {"7000", "localTypes:ARRAY [0..15] OF TBit1Byte", "input", "true", "false", "read", "read", "0", "local:in0", "IN", "", ""},
        {"8000", "localTypes:ARRAY [0..15] OF TBit1Byte", "output", "true", "false", "readwrite", "readwrite", "0", "local:out0", "OUT", "", ""},
        {"393218", "std:STRING", "none", "true", "false", "read", "read", "'CREVIS'", "local:Id393218", "Vendor", "local:Id393218.Desc", "Vendor of the device"},
        {"393219", "std:STRING", "none", "true", "false", "read", "read", "'MyRS422'", "local:Id393219", "Module ID", "local:Id393219.Desc", "Module ID of the device"},
        {"400000", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400000", "Parameter0", "local:Id400000.Desc", "Bardrate[0-3], Parity Bit[4-5], Stop Bit[6], TxD Process[7]"},
        {"400001", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400001", "Parameter1", "local:Id400001.Desc", "Data Bit[0-5]:16~63"},
        {"400002", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400002", "Reserved", "local:Id400002.Desc", "Reserved"},
        {"400003", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400003", "Reserved", "local:Id400003.Desc", "Reserved"},
    };

    for (const auto& pDef : params) {
        QDomElement paramElement = doc.createElement("Parameter");
        paramElement.setAttribute("ParameterId", pDef.id);
        paramElement.setAttribute("type", pDef.type);
        hostParamSetElement.appendChild(paramElement);

        QDomElement attributesElement = doc.createElement("Attributes");
        attributesElement.setAttribute("channel", pDef.channel);
        attributesElement.setAttribute("download", pDef.download);
        attributesElement.setAttribute("functional", pDef.functional);
        attributesElement.setAttribute("offlineaccess", pDef.offlineAccess);
        attributesElement.setAttribute("onlineaccess", pDef.onlineAccess);
        paramElement.appendChild(attributesElement);

        QDomElement defaultElement = doc.createElement("Default");
        defaultElement.appendChild(doc.createTextNode(pDef.defaultValue));
        paramElement.appendChild(defaultElement);

        QDomElement nameElement = doc.createElement("Name");
        nameElement.setAttribute("name", pDef.nameKey);
        nameElement.appendChild(doc.createTextNode(pDef.nameValue));
        paramElement.appendChild(nameElement);

        if (!pDef.descKey.isEmpty() && !pDef.descValue.isEmpty()) {
            QDomElement descElement = doc.createElement("Description");
            descElement.setAttribute("name", pDef.descKey);
            descElement.appendChild(doc.createTextNode(pDef.descValue));
            paramElement.appendChild(descElement);
        }
    }
    // --- /Добавляем второй Connector (parent) ---

    return doc;
}

QDomDocument createTemplateForRS485() {

    QDomDocument doc;
    // Добавляем декларацию XML
    QDomProcessingInstruction xmlDeclaration = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(xmlDeclaration);

    // Создаем корневой элемент DeviceDescription с пространствами имен
    QDomElement root = doc.createElementNS("http://www.3s-software.com/schemas/DeviceDescription-1.0.xsd", "DeviceDescription");
    root.setAttribute("xmlns", "http://www.3s-software.com/schemas/DeviceDescription-1.0.xsd");
    root.setAttribute("xmlns:ts", "http://www.3s-software.com/schemas/TargetSettings-0.1.xsd");
    root.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    doc.appendChild(root);

    QDomComment typesComment = doc.createComment("Types namespace=\"std\"></Types");
    root.appendChild(typesComment);

    // --- Добавляем Types ---
    QDomElement typesElement = doc.createElement("Types");
    typesElement.setAttribute("namespace", "localtype");
    root.appendChild(typesElement);

    QDomElement rangeTypeElement = doc.createElement("RangeType");
    rangeTypeElement.setAttribute("basetype", "std:UINT"); // Пример: базовый тип UINT
    rangeTypeElement.setAttribute("name", "AssemblySizeRangeType"); // Пример: название диапазона
    typesElement.appendChild(rangeTypeElement);

    QDomElement minElement = doc.createElement("Min");
    minElement.appendChild(doc.createTextNode("4")); // Пример: минимальное значение
    rangeTypeElement.appendChild(minElement);

    QDomElement maxElement = doc.createElement("Max");
    maxElement.appendChild(doc.createTextNode("1024")); // Пример: максимальное значение
    rangeTypeElement.appendChild(maxElement);

    QDomElement defaultElement = doc.createElement("Default");
    defaultElement.appendChild(doc.createTextNode("4")); // Пример: значение по умолчанию
    rangeTypeElement.appendChild(defaultElement);

    // Добавляем BitfieldType TBit1Byte
    QDomElement bitfieldTypeElement = doc.createElement("BitfieldType");
    bitfieldTypeElement.setAttribute("basetype", "std:BYTE");
    bitfieldTypeElement.setAttribute("name", "TBit1Byte");
    typesElement.appendChild(bitfieldTypeElement);

    // Добавляем компоненты Bit0 - Bit7
    for (int i = 0; i < 8; ++i) {
        QDomElement componentElement = doc.createElement("Component");
        componentElement.setAttribute("identifier", QString("Bit%1").arg(i));
        componentElement.setAttribute("type", "std:BOOL");

        QDomElement defaultElement = doc.createElement("Default");
        componentElement.appendChild(defaultElement);

        QDomElement visibleNameElement = doc.createElement("VisibleName");
        visibleNameElement.setAttribute("name", QString("localtype:TBit1Byte.Bit%1").arg(i));
        visibleNameElement.appendChild(doc.createTextNode(QString("BIT%1").arg(i)));
        componentElement.appendChild(visibleNameElement);

        bitfieldTypeElement.appendChild(componentElement);
    }
    // --- /Добавляем Types ---

    // --- Добавляем File ---
    QDomElement fileElement = doc.createElement("File");
    fileElement.setAttribute("fileref", "local");
    fileElement.setAttribute("identifier", "image_io");

    QDomElement localFileElement = doc.createElement("LocalFile");
    localFileElement.appendChild(doc.createTextNode("MyRS232.gif"));
    fileElement.appendChild(localFileElement);

    root.appendChild(fileElement);
    // --- /Добавляем File ---

    // --- Добавляем Device ---
    QDomElement deviceElement = doc.createElement("Device");
    deviceElement.setAttribute("hideInCatalogue", "false");
    root.appendChild(deviceElement);

    // DeviceIdentification
    QDomElement deviceIdentElement = doc.createElement("DeviceIdentification");
    deviceElement.appendChild(deviceIdentElement);

    QDomElement typeElement = doc.createElement("Type");
    typeElement.appendChild(doc.createTextNode("40305"));
    deviceIdentElement.appendChild(typeElement);

    QDomElement idElement = doc.createElement("Id");
    //idElement.appendChild(doc.createTextNode("10ad 1b7f"));
    idElement.appendChild(doc.createTextNode("10ad myrs485"));
    deviceIdentElement.appendChild(idElement);

    QDomElement versionElement = doc.createElement("Version");
    versionElement.appendChild(doc.createTextNode("2.0.0.0"));
    deviceIdentElement.appendChild(versionElement);

    // DeviceInfo
    QDomElement deviceInfoElement = doc.createElement("DeviceInfo");
    deviceElement.appendChild(deviceInfoElement);

    QDomElement nameElement = doc.createElement("Name");
    nameElement.setAttribute("name", "local:ModelName");
    //nameElement.appendChild(doc.createTextNode("GT-1B7F"));
    nameElement.appendChild(doc.createTextNode("MyRS485"));
    deviceInfoElement.appendChild(nameElement);

    QDomElement descElement = doc.createElement("Description");
    descElement.setAttribute("name", "local:DeviceDescription");
    descElement.appendChild(doc.createTextNode("RS-485 Serial Interface 1CH"));
    deviceInfoElement.appendChild(descElement);

    QDomElement vendorElement = doc.createElement("Vendor");
    vendorElement.setAttribute("name", "local:VendorName");
    vendorElement.appendChild(doc.createTextNode("CREVIS CO.,LTD"));
    deviceInfoElement.appendChild(vendorElement);

    QDomElement orderNumElement = doc.createElement("OrderNumber");
    //orderNumElement.appendChild(doc.createTextNode("GT-1B7F"));
    orderNumElement.appendChild(doc.createTextNode("MyRS485"));
    deviceInfoElement.appendChild(orderNumElement);

    QDomElement imageElement = doc.createElement("Image");
    imageElement.setAttribute("name", "local:image_io");
    //imageElement.appendChild(doc.createTextNode("GT-1B7F.gif"));
    imageElement.appendChild(doc.createTextNode("MyRS485.gif"));
    deviceInfoElement.appendChild(imageElement);

    // --- Добавляем первый Connector (child) ---
    QDomComment connectorComment = doc.createComment("Connector moduleType=\"257\" interface=\"Common.PCI\" role=\"child\" explicit=\"false\" connectorId=\"1\" hostpath=\"-1\">\n      <InterfaceName name=\"local:PCI\">PCI-Bus</InterfaceName>\n      <Slot count=\"1\" allowEmpty=\"false\"></Slot>\n    </Connector");
    deviceElement.appendChild(connectorComment);

    QDomElement connector1Element = doc.createElement("Connector");
    connector1Element.setAttribute("moduleType", "47000");
    connector1Element.setAttribute("interface", "CVS.OptionG");
    connector1Element.setAttribute("role", "child");
    connector1Element.setAttribute("explicit", "false");
    connector1Element.setAttribute("connectorId", "1");
    connector1Element.setAttribute("hostpath", "-1");

    QDomElement interfaceName1Element = doc.createElement("InterfaceName");
    interfaceName1Element.setAttribute("name", "local:PCI");
    interfaceName1Element.appendChild(doc.createTextNode("CVS Option"));
    connector1Element.appendChild(interfaceName1Element);

    QDomElement slotElement = doc.createElement("Slot");
    slotElement.setAttribute("count", "1");
    slotElement.setAttribute("allowEmpty", "false");
    connector1Element.appendChild(slotElement);

    deviceElement.appendChild(connector1Element);
    // --- /Добавляем первый Connector (child) ---

    // --- Добавляем второй Connector (parent) ---
    QDomElement connector2Element = doc.createElement("Connector");
    connector2Element.setAttribute("moduleType", "40305");
    connector2Element.setAttribute("interface", "MyCompany:Internal");
    connector2Element.setAttribute("role", "parent");
    connector2Element.setAttribute("explicit", "false");
    connector2Element.setAttribute("connectorId", "2");
    connector2Element.setAttribute("hostpath", "1");
    deviceElement.appendChild(connector2Element);

    QDomElement interfaceName2Element = doc.createElement("InterfaceName");
    interfaceName2Element.setAttribute("name", "local:DP");
    interfaceName2Element.appendChild(doc.createTextNode("ETC IOs"));
    connector2Element.appendChild(interfaceName2Element);

    QDomElement varElement = doc.createElement("Var");
    varElement.setAttribute("max", "8");
    connector2Element.appendChild(varElement);

    QDomElement driverInfoElement = doc.createElement("DriverInfo");
    driverInfoElement.setAttribute("needsBusCycle", "true");
    connector2Element.appendChild(driverInfoElement);

    // HostParameterSet
    QDomElement hostParamSetElement = doc.createElement("HostParameterSet");
    connector2Element.appendChild(hostParamSetElement);

    struct ParamDef {
        QString id;
        QString type;
        QString channel;
        QString download;
        QString functional;
        QString offlineAccess;
        QString onlineAccess;
        QString defaultValue;
        QString nameKey;
        QString nameValue;
        QString descKey;
        QString descValue;
    };

    std::vector<ParamDef> params = {
        {"7000", "localTypes:ARRAY [0..15] OF TBit1Byte", "input", "true", "false", "read", "read", "0", "local:in0", "IN", "", ""},
        {"8000", "localTypes:ARRAY [0..15] OF TBit1Byte", "output", "true", "false", "readwrite", "readwrite", "0", "local:out0", "OUT", "", ""},
        {"393218", "std:STRING", "none", "true", "false", "read", "read", "'CREVIS'", "local:Id393218", "Vendor", "local:Id393218.Desc", "Vendor of the device"},
        {"393219", "std:STRING", "none", "true", "false", "read", "read", "'MyRS485'", "local:Id393219", "Module ID", "local:Id393219.Desc", "Module ID of the device"},
        {"400000", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400000", "Parameter0", "local:Id400000.Desc", "Bardrate[0-3], Parity Bit[4-5], Stop Bit[6], TxD Process[7]"},
        {"400001", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400001", "Parameter1", "local:Id400001.Desc", "Data Bit[0-5]:16~63"},
        {"400002", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400002", "Reserved", "local:Id400002.Desc", "Reserved"},
        {"400003", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400003", "Reserved", "local:Id400003.Desc", "Reserved"},
    };

    for (const auto& pDef : params) {
        QDomElement paramElement = doc.createElement("Parameter");
        paramElement.setAttribute("ParameterId", pDef.id);
        paramElement.setAttribute("type", pDef.type);
        hostParamSetElement.appendChild(paramElement);

        QDomElement attributesElement = doc.createElement("Attributes");
        attributesElement.setAttribute("channel", pDef.channel);
        attributesElement.setAttribute("download", pDef.download);
        attributesElement.setAttribute("functional", pDef.functional);
        attributesElement.setAttribute("offlineaccess", pDef.offlineAccess);
        attributesElement.setAttribute("onlineaccess", pDef.onlineAccess);
        paramElement.appendChild(attributesElement);

        QDomElement defaultElement = doc.createElement("Default");
        defaultElement.appendChild(doc.createTextNode(pDef.defaultValue));
        paramElement.appendChild(defaultElement);

        QDomElement nameElement = doc.createElement("Name");
        nameElement.setAttribute("name", pDef.nameKey);
        nameElement.appendChild(doc.createTextNode(pDef.nameValue));
        paramElement.appendChild(nameElement);

        if (!pDef.descKey.isEmpty() && !pDef.descValue.isEmpty()) {
            QDomElement descElement = doc.createElement("Description");
            descElement.setAttribute("name", pDef.descKey);
            descElement.appendChild(doc.createTextNode(pDef.descValue));
            paramElement.appendChild(descElement);
        }
    }
    // --- /Добавляем второй Connector (parent) ---

    return doc;
}

QDomDocument createTemplateForSSI() {

    QDomDocument doc;
    // Добавляем декларацию XML
    QDomProcessingInstruction xmlDeclaration = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(xmlDeclaration);

    // Создаем корневой элемент DeviceDescription с пространствами имен
    QDomElement root = doc.createElementNS("http://www.3s-software.com/schemas/DeviceDescription-1.0.xsd", "DeviceDescription");
    //root.setAttribute("xmlns", "http://www.3s-software.com/schemas/DeviceDescription-1.0.xsd");
    root.setAttribute("xmlns:ts", "http://www.3s-software.com/schemas/TargetSettings-0.1.xsd");
    root.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    doc.appendChild(root);

    QDomComment typesComment = doc.createComment("Types namespace=\"std\"></Types");
    root.appendChild(typesComment);

    // --- Добавляем Types ---
    QDomElement typesElement = doc.createElement("Types");
    typesElement.setAttribute("namespace", "localtype");
    root.appendChild(typesElement);

    QDomElement rangeTypeElement = doc.createElement("RangeType");
    rangeTypeElement.setAttribute("basetype", "std:UINT"); // Пример: базовый тип UINT
    rangeTypeElement.setAttribute("name", "AssemblySizeRangeType"); // Пример: название диапазона
    typesElement.appendChild(rangeTypeElement);

    QDomElement minElement = doc.createElement("Min");
    minElement.appendChild(doc.createTextNode("4")); // Пример: минимальное значение
    rangeTypeElement.appendChild(minElement);

    QDomElement maxElement = doc.createElement("Max");
    maxElement.appendChild(doc.createTextNode("1024")); // Пример: максимальное значение
    rangeTypeElement.appendChild(maxElement);

    QDomElement defaultElement = doc.createElement("Default");
    defaultElement.appendChild(doc.createTextNode("4")); // Пример: значение по умолчанию
    rangeTypeElement.appendChild(defaultElement);

    // Добавляем BitfieldType TBit1Byte
    QDomElement bitfieldTypeElement = doc.createElement("BitfieldType");
    bitfieldTypeElement.setAttribute("basetype", "std:BYTE");
    bitfieldTypeElement.setAttribute("name", "TBit1Byte");
    typesElement.appendChild(bitfieldTypeElement);

    // Добавляем компоненты Bit0 - Bit7
    for (int i = 0; i < 8; ++i) {
        QDomElement componentElement = doc.createElement("Component");
        componentElement.setAttribute("identifier", QString("Bit%1").arg(i));
        componentElement.setAttribute("type", "std:BOOL");

        QDomElement defaultElement = doc.createElement("Default");
        componentElement.appendChild(defaultElement);

        QDomElement visibleNameElement = doc.createElement("VisibleName");
        visibleNameElement.setAttribute("name", QString("localtype:TBit1Byte.Bit%1").arg(i));
        visibleNameElement.appendChild(doc.createTextNode(QString("BIT%1").arg(i)));
        componentElement.appendChild(visibleNameElement);

        bitfieldTypeElement.appendChild(componentElement);
    }
    // --- /Добавляем Types ---

    // --- Добавляем File ---
    QDomElement fileElement = doc.createElement("File");
    fileElement.setAttribute("fileref", "local");
    fileElement.setAttribute("identifier", "image_io");

    QDomElement localFileElement = doc.createElement("LocalFile");
    localFileElement.appendChild(doc.createTextNode("MyRS232.gif"));
    fileElement.appendChild(localFileElement);

    root.appendChild(fileElement);
    // --- /Добавляем File ---

    // --- Добавляем Device ---
    QDomElement deviceElement = doc.createElement("Device");
    deviceElement.setAttribute("hideInCatalogue", "false");
    root.appendChild(deviceElement);

    // DeviceIdentification
    QDomElement deviceIdentElement = doc.createElement("DeviceIdentification");
    deviceElement.appendChild(deviceIdentElement);

    QDomElement typeElement = doc.createElement("Type");
    typeElement.appendChild(doc.createTextNode("40305"));
    deviceIdentElement.appendChild(typeElement);

    QDomElement idElement = doc.createElement("Id");
    //idElement.appendChild(doc.createTextNode("10ad 1b7f"));
    idElement.appendChild(doc.createTextNode("10ad myssi"));
    deviceIdentElement.appendChild(idElement);

    QDomElement versionElement = doc.createElement("Version");
    versionElement.appendChild(doc.createTextNode("2.0.0.0"));
    deviceIdentElement.appendChild(versionElement);

    // DeviceInfo
    QDomElement deviceInfoElement = doc.createElement("DeviceInfo");
    deviceElement.appendChild(deviceInfoElement);

    QDomElement nameElement = doc.createElement("Name");
    nameElement.setAttribute("name", "local:ModelName");
    //nameElement.appendChild(doc.createTextNode("GT-1B7F"));
    nameElement.appendChild(doc.createTextNode("MySSI"));
    deviceInfoElement.appendChild(nameElement);

    QDomElement descElement = doc.createElement("Description");
    descElement.setAttribute("name", "local:DeviceDescription");
    descElement.appendChild(doc.createTextNode("Synchronous Serial Interface Input 2CH"));
    deviceInfoElement.appendChild(descElement);

    QDomElement vendorElement = doc.createElement("Vendor");
    vendorElement.setAttribute("name", "local:VendorName");
    vendorElement.appendChild(doc.createTextNode("CREVIS CO.,LTD"));
    deviceInfoElement.appendChild(vendorElement);

    QDomElement orderNumElement = doc.createElement("OrderNumber");
    //orderNumElement.appendChild(doc.createTextNode("GT-1B7F"));
    orderNumElement.appendChild(doc.createTextNode("MySSI"));
    deviceInfoElement.appendChild(orderNumElement);

    QDomElement imageElement = doc.createElement("Image");
    imageElement.setAttribute("name", "local:image_io");
    //imageElement.appendChild(doc.createTextNode("GT-1B7F.gif"));
    imageElement.appendChild(doc.createTextNode("MySSI.gif"));
    deviceInfoElement.appendChild(imageElement);

    // --- Добавляем первый Connector (child) ---
    QDomComment connectorComment = doc.createComment("Connector moduleType=\"257\" interface=\"Common.PCI\" role=\"child\" explicit=\"false\" connectorId=\"1\" hostpath=\"-1\">\n      <InterfaceName name=\"local:PCI\">PCI-Bus</InterfaceName>\n      <Slot count=\"1\" allowEmpty=\"false\"></Slot>\n    </Connector");
    deviceElement.appendChild(connectorComment);

    QDomElement connector1Element = doc.createElement("Connector");
    connector1Element.setAttribute("moduleType", "47000");
    connector1Element.setAttribute("interface", "CVS.OptionG");
    connector1Element.setAttribute("role", "child");
    connector1Element.setAttribute("explicit", "false");
    connector1Element.setAttribute("connectorId", "1");
    connector1Element.setAttribute("hostpath", "-1");

    QDomElement interfaceName1Element = doc.createElement("InterfaceName");
    interfaceName1Element.setAttribute("name", "local:PCI");
    interfaceName1Element.appendChild(doc.createTextNode("CVS Option"));
    connector1Element.appendChild(interfaceName1Element);

    QDomElement slotElement = doc.createElement("Slot");
    slotElement.setAttribute("count", "1");
    slotElement.setAttribute("allowEmpty", "false");
    connector1Element.appendChild(slotElement);

    deviceElement.appendChild(connector1Element);
    // --- /Добавляем первый Connector (child) ---

    // --- Добавляем второй Connector (parent) ---
    QDomElement connector2Element = doc.createElement("Connector");
    connector2Element.setAttribute("moduleType", "40305");
    connector2Element.setAttribute("interface", "MyCompany:Internal");
    connector2Element.setAttribute("role", "parent");
    connector2Element.setAttribute("explicit", "false");
    connector2Element.setAttribute("connectorId", "2");
    connector2Element.setAttribute("hostpath", "1");
    deviceElement.appendChild(connector2Element);

    QDomElement interfaceName2Element = doc.createElement("InterfaceName");
    interfaceName2Element.setAttribute("name", "local:DP");
    interfaceName2Element.appendChild(doc.createTextNode("ETC IOs"));
    connector2Element.appendChild(interfaceName2Element);

    QDomElement varElement = doc.createElement("Var");
    varElement.setAttribute("max", "8");
    connector2Element.appendChild(varElement);

    QDomElement driverInfoElement = doc.createElement("DriverInfo");
    driverInfoElement.setAttribute("needsBusCycle", "true");
    connector2Element.appendChild(driverInfoElement);

    // HostParameterSet
    QDomElement hostParamSetElement = doc.createElement("HostParameterSet");
    connector2Element.appendChild(hostParamSetElement);

    struct ParamDef {
        QString id;
        QString type;
        QString channel;
        QString download;
        QString functional;
        QString offlineAccess;
        QString onlineAccess;
        QString defaultValue;
        QString nameKey;
        QString nameValue;
        QString descKey;
        QString descValue;
    };

    std::vector<ParamDef> params = {
        {"7000", "localTypes:ARRAY [0..15] OF TBit1Byte", "input", "true", "false", "read", "read", "0", "local:in0", "IN", "", ""},
        {"8000", "localTypes:ARRAY [0..15] OF TBit1Byte", "output", "true", "false", "readwrite", "readwrite", "0", "local:out0", "OUT", "", ""},
        {"393218", "std:STRING", "none", "true", "false", "read", "read", "'CREVIS'", "local:Id393218", "Vendor", "local:Id393218.Desc", "Vendor of the device"},
        {"393219", "std:STRING", "none", "true", "false", "read", "read", "'MySSI'", "local:Id393219", "Module ID", "local:Id393219.Desc", "Module ID of the device"},
        {"400000", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400000", "Parameter0", "local:Id400000.Desc", "Bardrate[0-3], Parity Bit[4-5], Stop Bit[6], TxD Process[7]"},
        {"400001", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400001", "Parameter1", "local:Id400001.Desc", "Data Bit[0-5]:16~63"},
        {"400002", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400002", "Reserved", "local:Id400002.Desc", "Reserved"},
        {"400003", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400003", "Reserved", "local:Id400003.Desc", "Reserved"},
    };

    for (const auto& pDef : params) {
        QDomElement paramElement = doc.createElement("Parameter");
        paramElement.setAttribute("ParameterId", pDef.id);
        paramElement.setAttribute("type", pDef.type);
        hostParamSetElement.appendChild(paramElement);

        QDomElement attributesElement = doc.createElement("Attributes");
        attributesElement.setAttribute("channel", pDef.channel);
        attributesElement.setAttribute("download", pDef.download);
        attributesElement.setAttribute("functional", pDef.functional);
        attributesElement.setAttribute("offlineaccess", pDef.offlineAccess);
        attributesElement.setAttribute("onlineaccess", pDef.onlineAccess);
        paramElement.appendChild(attributesElement);

        QDomElement defaultElement = doc.createElement("Default");
        defaultElement.appendChild(doc.createTextNode(pDef.defaultValue));
        paramElement.appendChild(defaultElement);

        QDomElement nameElement = doc.createElement("Name");
        nameElement.setAttribute("name", pDef.nameKey);
        nameElement.appendChild(doc.createTextNode(pDef.nameValue));
        paramElement.appendChild(nameElement);

        if (!pDef.descKey.isEmpty() && !pDef.descValue.isEmpty()) {
            QDomElement descElement = doc.createElement("Description");
            descElement.setAttribute("name", pDef.descKey);
            descElement.appendChild(doc.createTextNode(pDef.descValue));
            paramElement.appendChild(descElement);
        }
    }
    // --- /Добавляем второй Connector (parent) ---

    return doc;
}

QDomDocument createTemplateForEncoder() {

    QDomDocument doc;
    // Добавляем декларацию XML
    QDomProcessingInstruction xmlDeclaration = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(xmlDeclaration);

    // Создаем корневой элемент DeviceDescription с пространствами имен
    QDomElement root = doc.createElementNS("http://www.3s-software.com/schemas/DeviceDescription-1.0.xsd", "DeviceDescription");
    root.setAttribute("xmlns", "http://www.3s-software.com/schemas/DeviceDescription-1.0.xsd");
    root.setAttribute("xmlns:ts", "http://www.3s-software.com/schemas/TargetSettings-0.1.xsd");
    root.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    doc.appendChild(root);

    QDomComment typesComment = doc.createComment("Types namespace=\"std\"></Types");
    root.appendChild(typesComment);

    // --- Добавляем Types ---
    QDomElement typesElement = doc.createElement("Types");
    typesElement.setAttribute("namespace", "localtype");
    root.appendChild(typesElement);

    // Добавляем BitfieldType TBit1Byte
    QDomElement bitfieldTypeElement = doc.createElement("BitfieldType");
    bitfieldTypeElement.setAttribute("basetype", "std:BYTE");
    bitfieldTypeElement.setAttribute("name", "TBit1Byte");
    typesElement.appendChild(bitfieldTypeElement);

    // Добавляем компоненты Bit0 - Bit7
    for (int i = 0; i < 8; ++i) {
        QDomElement componentElement = doc.createElement("Component");
        componentElement.setAttribute("identifier", QString("Bit%1").arg(i));
        componentElement.setAttribute("type", "std:BOOL");

        QDomElement defaultElement = doc.createElement("Default");
        componentElement.appendChild(defaultElement);

        QDomElement visibleNameElement = doc.createElement("VisibleName");
        visibleNameElement.setAttribute("name", QString("localtype:TBit1Byte.Bit%1").arg(i));
        visibleNameElement.appendChild(doc.createTextNode(QString("BIT%1").arg(i)));
        componentElement.appendChild(visibleNameElement);

        bitfieldTypeElement.appendChild(componentElement);
    }

    //QDomElement typesElement = doc.createElement("Types");
    typesElement.setAttribute("namespace", "localtype");
    root.appendChild(typesElement);

    QDomElement fileElement = doc.createElement("File");
    fileElement.setAttribute("fileref", "local");
    fileElement.setAttribute("identifier", "image_input");

    QDomElement localFileElement = doc.createElement("LocalFile");
    localFileElement.appendChild(doc.createTextNode("MyEncoder.gif"));
    fileElement.appendChild(localFileElement);

    root.appendChild(fileElement);

    // --- Добавляем Device ---
    QDomElement deviceElement = doc.createElement("Device");
    deviceElement.setAttribute("hideInCatalogue", "false");
    root.appendChild(deviceElement);

    // DeviceIdentification
    QDomElement deviceIdentElement = doc.createElement("DeviceIdentification");
    deviceElement.appendChild(deviceIdentElement);

    QDomElement typeElement = doc.createElement("Type");
    typeElement.appendChild(doc.createTextNode("40305"));
    deviceIdentElement.appendChild(typeElement);

    QDomElement idElement = doc.createElement("Id");
    //idElement.appendChild(doc.createTextNode("10ad 1b7f"));
    idElement.appendChild(doc.createTextNode("10ad myencoder"));
    deviceIdentElement.appendChild(idElement);

    QDomElement versionElement = doc.createElement("Version");
    versionElement.appendChild(doc.createTextNode("2.0.0.0"));
    deviceIdentElement.appendChild(versionElement);

    // DeviceInfo
    QDomElement deviceInfoElement = doc.createElement("DeviceInfo");
    deviceElement.appendChild(deviceInfoElement);

    QDomElement nameElement = doc.createElement("Name");
    nameElement.setAttribute("name", "local:ModelName");
    //nameElement.appendChild(doc.createTextNode("GT-1B7F"));
    nameElement.appendChild(doc.createTextNode("MyEncoder"));
    deviceInfoElement.appendChild(nameElement);

    QDomElement descElement = doc.createElement("Description");
    descElement.setAttribute("name", "local:DeviceDescription");
    descElement.appendChild(doc.createTextNode("High Speed Counter Encoder Input 2CH, 5Vdc"));
    deviceInfoElement.appendChild(descElement);

    QDomElement vendorElement = doc.createElement("Vendor");
    vendorElement.setAttribute("name", "local:VendorName");
    vendorElement.appendChild(doc.createTextNode("CREVIS CO.,LTD"));
    deviceInfoElement.appendChild(vendorElement);

    QDomElement orderNumElement = doc.createElement("OrderNumber");
    //orderNumElement.appendChild(doc.createTextNode("GT-1B7F"));
    orderNumElement.appendChild(doc.createTextNode("MyEncoder"));
    deviceInfoElement.appendChild(orderNumElement);

    QDomElement imageElement = doc.createElement("Image");
    imageElement.setAttribute("name", "local:image_io");
    //imageElement.appendChild(doc.createTextNode("GT-1B7F.gif"));
    imageElement.appendChild(doc.createTextNode("MyEncoder.gif"));
    deviceInfoElement.appendChild(imageElement);

    QDomComment connectorComment = doc.createComment("Connector moduleType=\"257\" interface=\"Common.PCI\" role=\"child\" explicit=\"false\" connectorId=\"1\" hostpath=\"-1\">\n      <InterfaceName name=\"local:PCI\">PCI-Bus</InterfaceName>\n      <Slot count=\"1\" allowEmpty=\"false\"></Slot>\n    </Connector");
    deviceElement.appendChild(connectorComment);

    QDomElement connector1Element = doc.createElement("Connector");
    connector1Element.setAttribute("moduleType", "47000");
    connector1Element.setAttribute("interface", "CVS.OptionG");
    connector1Element.setAttribute("role", "child");
    connector1Element.setAttribute("explicit", "false");
    connector1Element.setAttribute("connectorId", "1");
    connector1Element.setAttribute("hostpath", "-1");

    QDomElement interfaceName1Element = doc.createElement("InterfaceName");
    interfaceName1Element.setAttribute("name", "local:PCI");
    interfaceName1Element.appendChild(doc.createTextNode("CVS Option"));
    connector1Element.appendChild(interfaceName1Element);

    QDomElement slotElement = doc.createElement("Slot");
    slotElement.setAttribute("count", "1");
    slotElement.setAttribute("allowEmpty", "false");
    connector1Element.appendChild(slotElement);

    deviceElement.appendChild(connector1Element);

    QDomElement connector2Element = doc.createElement("Connector");
    connector2Element.setAttribute("moduleType", "40302");
    connector2Element.setAttribute("interface", "MyCompany:Internal");
    connector2Element.setAttribute("role", "parent");
    connector2Element.setAttribute("explicit", "false");
    connector2Element.setAttribute("connectorId", "2");
    connector2Element.setAttribute("hostpath", "1");
    deviceElement.appendChild(connector2Element);

    QDomElement interfaceName2Element = doc.createElement("InterfaceName");
    interfaceName2Element.setAttribute("name", "local:DP");
    interfaceName2Element.appendChild(doc.createTextNode("ETC IOs"));
    connector2Element.appendChild(interfaceName2Element);

    QDomElement varElement = doc.createElement("Var");
    varElement.setAttribute("max", "8");
    connector2Element.appendChild(varElement);

    QDomElement driverInfoElement = doc.createElement("DriverInfo");
    driverInfoElement.setAttribute("needsBusCycle", "true");
    connector2Element.appendChild(driverInfoElement);

    // HostParameterSet
    QDomElement hostParamSetElement = doc.createElement("HostParameterSet");
    connector2Element.appendChild(hostParamSetElement);

    struct ParamDef {
        QString id;
        QString type;
        QString channel;
        QString download;
        QString functional;
        QString offlineAccess;
        QString onlineAccess;
        QString defaultValue;
        QString nameKey;
        QString nameValue;
        QString descKey;
        QString descValue;
    };

    std::vector<ParamDef> params = {
        {"3000", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in0", "IN0", "", ""},
        {"3001", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in1", "IN1", "", ""},
        {"3002", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in2", "IN2", "", ""},
        {"3003", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in3", "IN3", "", ""},


        {"2001", "std:WORD", "input", "true", "false", "read", "read", "0", "local:out0", "OUT0", "", ""},
        {"2002", "std:WORD", "input", "true", "false", "read", "read", "0", "local:out1", "OUT1", "", ""},


        {"393218", "std:STRING", "none", "true", "false", "read", "read", "'CREVIS'", "local:Id393218", "Vendor", "local:Id393218.Desc", "Vendor of the device"},
        {"393219", "std:STRING", "none", "true", "false", "read", "read", "'MyEncoder'", "local:Id393219", "Module ID", "local:Id393219.Desc", "Module ID of the device"},

        {"400000", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400000", "Reserved", "local:Id400000.Desc", ""},
        {"400001", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400001", "Reserved", "local:Id400001.Desc", ""},

    };

    for (const auto& pDef : params) {
        QDomElement paramElement = doc.createElement("Parameter");
        paramElement.setAttribute("ParameterId", pDef.id);
        paramElement.setAttribute("type", pDef.type);
        hostParamSetElement.appendChild(paramElement);

        QDomElement attributesElement = doc.createElement("Attributes");
        attributesElement.setAttribute("channel", pDef.channel);
        attributesElement.setAttribute("download", pDef.download);
        attributesElement.setAttribute("functional", pDef.functional);
        attributesElement.setAttribute("offlineaccess", pDef.offlineAccess);
        attributesElement.setAttribute("onlineaccess", pDef.onlineAccess);
        paramElement.appendChild(attributesElement);

        QDomElement defaultElement = doc.createElement("Default");
        defaultElement.appendChild(doc.createTextNode(pDef.defaultValue));
        paramElement.appendChild(defaultElement);

        QDomElement nameElement = doc.createElement("Name");
        nameElement.setAttribute("name", pDef.nameKey);
        nameElement.appendChild(doc.createTextNode(pDef.nameValue));
        paramElement.appendChild(nameElement);

        if (!pDef.descKey.isEmpty() && !pDef.descValue.isEmpty()) {
            QDomElement descElement = doc.createElement("Description");
            descElement.setAttribute("name", pDef.descKey);
            descElement.appendChild(doc.createTextNode(pDef.descValue));
            paramElement.appendChild(descElement);
        }
    }

    return doc;
}

QDomDocument createTemplateForPWM() {

    QDomDocument doc;
    // Добавляем декларацию XML
    QDomProcessingInstruction xmlDeclaration = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(xmlDeclaration);

    // Создаем корневой элемент DeviceDescription с пространствами имен
    QDomElement root = doc.createElementNS("http://www.3s-software.com/schemas/DeviceDescription-1.0.xsd", "DeviceDescription");
    root.setAttribute("xmlns", "http://www.3s-software.com/schemas/DeviceDescription-1.0.xsd");
    root.setAttribute("xmlns:ts", "http://www.3s-software.com/schemas/TargetSettings-0.1.xsd");
    root.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    doc.appendChild(root);

    QDomComment typesComment = doc.createComment("Types namespace=\"std\"></Types");
    root.appendChild(typesComment);

    QDomElement typesElement = doc.createElement("Types");
    typesElement.setAttribute("namespace", "localtype");
    root.appendChild(typesElement);

    QDomElement bitfieldTypeElement = doc.createElement("BitfieldType");
    bitfieldTypeElement.setAttribute("basetype", "std:BYTE");
    bitfieldTypeElement.setAttribute("name", "TBit1Byte");
    typesElement.appendChild(bitfieldTypeElement);


    for (int i = 0; i < 8; ++i) {
        QDomElement componentElement = doc.createElement("Component");
        componentElement.setAttribute("identifier", QString("Bit%1").arg(i));
        componentElement.setAttribute("type", "std:BOOL");

        QDomElement defaultElement = doc.createElement("Default");
        componentElement.appendChild(defaultElement);

        QDomElement visibleNameElement = doc.createElement("VisibleName");
        visibleNameElement.setAttribute("name", QString("localtype:TBit1Byte.Bit%1").arg(i));
        visibleNameElement.appendChild(doc.createTextNode(QString("BIT%1").arg(i)));
        componentElement.appendChild(visibleNameElement);

        bitfieldTypeElement.appendChild(componentElement);
    }


    QDomElement fileElement = doc.createElement("File");
    fileElement.setAttribute("fileref", "local");
    fileElement.setAttribute("identifier", "image_input");

    QDomElement localFileElement = doc.createElement("LocalFile");
    localFileElement.appendChild(doc.createTextNode("MyPWM.gif"));
    fileElement.appendChild(localFileElement);

    root.appendChild(fileElement);

    // --- Добавляем Device ---
    QDomElement deviceElement = doc.createElement("Device");
    deviceElement.setAttribute("hideInCatalogue", "false");
    root.appendChild(deviceElement);

    // DeviceIdentification
    QDomElement deviceIdentElement = doc.createElement("DeviceIdentification");
    deviceElement.appendChild(deviceIdentElement);

    QDomElement typeElement = doc.createElement("Type");
    typeElement.appendChild(doc.createTextNode("40301"));
    deviceIdentElement.appendChild(typeElement);

    QDomElement idElement = doc.createElement("Id");
    //idElement.appendChild(doc.createTextNode("10ad 1b7f"));
    idElement.appendChild(doc.createTextNode("10ad mypwm"));
    deviceIdentElement.appendChild(idElement);

    QDomElement versionElement = doc.createElement("Version");
    versionElement.appendChild(doc.createTextNode("2.0.0.0"));
    deviceIdentElement.appendChild(versionElement);

    // DeviceInfo
    QDomElement deviceInfoElement = doc.createElement("DeviceInfo");
    deviceElement.appendChild(deviceInfoElement);

    QDomElement nameElement = doc.createElement("Name");
    nameElement.setAttribute("name", "local:ModelName");
    //nameElement.appendChild(doc.createTextNode("GT-1B7F"));
    nameElement.appendChild(doc.createTextNode("MyPWM"));
    deviceInfoElement.appendChild(nameElement);

    QDomElement descElement = doc.createElement("Description");
    descElement.setAttribute("name", "local:DeviceDescription");
    descElement.appendChild(doc.createTextNode("PWM Output 4CH, Push-pull, 24Vdc/0.5A"));
    deviceInfoElement.appendChild(descElement);

    QDomElement vendorElement = doc.createElement("Vendor");
    vendorElement.setAttribute("name", "local:VendorName");
    vendorElement.appendChild(doc.createTextNode("CREVIS CO.,LTD"));
    deviceInfoElement.appendChild(vendorElement);

    QDomElement orderNumElement = doc.createElement("OrderNumber");
    //orderNumElement.appendChild(doc.createTextNode("GT-1B7F"));
    orderNumElement.appendChild(doc.createTextNode("MyPWM"));
    deviceInfoElement.appendChild(orderNumElement);

    QDomElement imageElement = doc.createElement("Image");
    imageElement.setAttribute("name", "local:image_io");
    //imageElement.appendChild(doc.createTextNode("GT-1B7F.gif"));
    imageElement.appendChild(doc.createTextNode("MyPWM.gif"));
    deviceInfoElement.appendChild(imageElement);

    QDomComment connectorComment = doc.createComment("Connector moduleType=\"257\" interface=\"Common.PCI\" role=\"child\" explicit=\"false\" connectorId=\"1\" hostpath=\"-1\">\n      <InterfaceName name=\"local:PCI\">PCI-Bus</InterfaceName>\n      <Slot count=\"1\" allowEmpty=\"false\"></Slot>\n    </Connector");
    deviceElement.appendChild(connectorComment);

    QDomElement connector1Element = doc.createElement("Connector");
    connector1Element.setAttribute("moduleType", "47000");
    connector1Element.setAttribute("interface", "CVS.OptionG");
    connector1Element.setAttribute("role", "child");
    connector1Element.setAttribute("explicit", "false");
    connector1Element.setAttribute("connectorId", "1");
    connector1Element.setAttribute("hostpath", "-1");

    QDomElement interfaceName1Element = doc.createElement("InterfaceName");
    interfaceName1Element.setAttribute("name", "local:PCI");
    interfaceName1Element.appendChild(doc.createTextNode("CVS Option"));
    connector1Element.appendChild(interfaceName1Element);

    QDomElement slotElement = doc.createElement("Slot");
    slotElement.setAttribute("count", "1");
    slotElement.setAttribute("allowEmpty", "false");
    connector1Element.appendChild(slotElement);

    deviceElement.appendChild(connector1Element);

    QDomElement connector2Element = doc.createElement("Connector");
    connector2Element.setAttribute("moduleType", "40302");
    connector2Element.setAttribute("interface", "MyCompany:Internal");
    connector2Element.setAttribute("role", "parent");
    connector2Element.setAttribute("explicit", "false");
    connector2Element.setAttribute("connectorId", "2");
    connector2Element.setAttribute("hostpath", "1");
    deviceElement.appendChild(connector2Element);

    QDomElement interfaceName2Element = doc.createElement("InterfaceName");
    interfaceName2Element.setAttribute("name", "local:DP");
    interfaceName2Element.appendChild(doc.createTextNode("ETC IOs"));
    connector2Element.appendChild(interfaceName2Element);

    QDomElement varElement = doc.createElement("Var");
    varElement.setAttribute("max", "8");
    connector2Element.appendChild(varElement);

    QDomElement driverInfoElement = doc.createElement("DriverInfo");
    driverInfoElement.setAttribute("needsBusCycle", "true");
    connector2Element.appendChild(driverInfoElement);

    // HostParameterSet
    QDomElement hostParamSetElement = doc.createElement("HostParameterSet");
    connector2Element.appendChild(hostParamSetElement);

    struct ParamDef {
        QString id;
        QString type;
        QString channel;
        QString download;
        QString functional;
        QString offlineAccess;
        QString onlineAccess;
        QString defaultValue;
        QString nameKey;
        QString nameValue;
        QString descKey;
        QString descValue;
    };

    std::vector<ParamDef> params = {
        {"1000", "localtype:TBit1Byte", "input", "true", "false", "read", "read", "0", "local:in0", "IN0", "", ""},
        {"1001", "localtype:TBit1Byte", "input", "true", "false", "read", "read", "0", "local:in1", "IN1", "", ""},

        {"4000", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in2", "Frequency CH0", "", ""},
        {"4001", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in3", "Duty CH0", "", ""},
        {"4002", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in4", "Frequency CH1", "", ""},
        {"4003", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in5", "Duty CH1", "", ""},


        {"393218", "std:STRING", "none", "true", "false", "read", "read", "'CREVIS'", "local:Id393218", "Vendor", "local:Id393218.Desc", "Vendor of the device"},
        {"393219", "std:STRING", "none", "true", "false", "read", "read", "'MyPWM'", "local:Id393219", "Module ID", "local:Id393219.Desc", "Module ID of the device"},


        {"400000", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400000", "Reserved", "local:Id400000.Desc", ""},
        {"400001", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400001", "Reserved", "local:Id400001.Desc", ""},
    };

    for (const auto& pDef : params) {
        QDomElement paramElement = doc.createElement("Parameter");
        paramElement.setAttribute("ParameterId", pDef.id);
        paramElement.setAttribute("type", pDef.type);
        hostParamSetElement.appendChild(paramElement);

        QDomElement attributesElement = doc.createElement("Attributes");
        attributesElement.setAttribute("channel", pDef.channel);
        attributesElement.setAttribute("download", pDef.download);
        attributesElement.setAttribute("functional", pDef.functional);
        attributesElement.setAttribute("offlineaccess", pDef.offlineAccess);
        attributesElement.setAttribute("onlineaccess", pDef.onlineAccess);
        paramElement.appendChild(attributesElement);

        QDomElement defaultElement = doc.createElement("Default");
        defaultElement.appendChild(doc.createTextNode(pDef.defaultValue));
        paramElement.appendChild(defaultElement);

        QDomElement nameElement = doc.createElement("Name");
        nameElement.setAttribute("name", pDef.nameKey);
        nameElement.appendChild(doc.createTextNode(pDef.nameValue));
        paramElement.appendChild(nameElement);

        if (!pDef.descKey.isEmpty() && !pDef.descValue.isEmpty()) {
            QDomElement descElement = doc.createElement("Description");
            descElement.setAttribute("name", pDef.descKey);
            descElement.appendChild(doc.createTextNode(pDef.descValue));
            paramElement.appendChild(descElement);
        }
    }

    return doc;
}

QDomDocument createTemplateForDrivverSteppingMotor() {

    QDomDocument doc;
    // Добавляем декларацию XML
    QDomProcessingInstruction xmlDeclaration = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(xmlDeclaration);

    // Создаем корневой элемент DeviceDescription с пространствами имен
    QDomElement root = doc.createElementNS("http://www.3s-software.com/schemas/DeviceDescription-1.0.xsd", "DeviceDescription");
    root.setAttribute("xmlns", "http://www.3s-software.com/schemas/DeviceDescription-1.0.xsd");
    root.setAttribute("xmlns:ts", "http://www.3s-software.com/schemas/TargetSettings-0.1.xsd");
    root.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    doc.appendChild(root);

    QDomComment typesComment = doc.createComment("Types namespace=\"std\"></Types");
    root.appendChild(typesComment);

    QDomElement typesElement = doc.createElement("Types");
    typesElement.setAttribute("namespace", "localtype");
    root.appendChild(typesElement);

    QDomElement bitfieldTypeElement = doc.createElement("BitfieldType");
    bitfieldTypeElement.setAttribute("basetype", "std:BYTE");
    bitfieldTypeElement.setAttribute("name", "TBit1Byte");
    typesElement.appendChild(bitfieldTypeElement);


    for (int i = 0; i < 8; ++i) {
        QDomElement componentElement = doc.createElement("Component");
        componentElement.setAttribute("identifier", QString("Bit%1").arg(i));
        componentElement.setAttribute("type", "std:BOOL");

        QDomElement defaultElement = doc.createElement("Default");
        componentElement.appendChild(defaultElement);

        QDomElement visibleNameElement = doc.createElement("VisibleName");
        visibleNameElement.setAttribute("name", QString("localtype:TBit1Byte.Bit%1").arg(i));
        visibleNameElement.appendChild(doc.createTextNode(QString("BIT%1").arg(i)));
        componentElement.appendChild(visibleNameElement);

        bitfieldTypeElement.appendChild(componentElement);
    }


    QDomElement fileElement = doc.createElement("File");
    fileElement.setAttribute("fileref", "local");
    fileElement.setAttribute("identifier", "image_input");

    QDomElement localFileElement = doc.createElement("LocalFile");
    localFileElement.appendChild(doc.createTextNode("MyDrivverSteppingMoror.gif"));
    fileElement.appendChild(localFileElement);

    root.appendChild(fileElement);

    // --- Добавляем Device ---
    QDomElement deviceElement = doc.createElement("Device");
    deviceElement.setAttribute("hideInCatalogue", "false");
    root.appendChild(deviceElement);

    // DeviceIdentification
    QDomElement deviceIdentElement = doc.createElement("DeviceIdentification");
    deviceElement.appendChild(deviceIdentElement);

    QDomElement typeElement = doc.createElement("Type");
    typeElement.appendChild(doc.createTextNode("40305"));
    deviceIdentElement.appendChild(typeElement);

    QDomElement idElement = doc.createElement("Id");
    //idElement.appendChild(doc.createTextNode("10ad 1b7f"));
    idElement.appendChild(doc.createTextNode("10ad mydrivverdteppingmotor"));
    deviceIdentElement.appendChild(idElement);

    QDomElement versionElement = doc.createElement("Version");
    versionElement.appendChild(doc.createTextNode("2.0.0.0"));
    deviceIdentElement.appendChild(versionElement);

    // DeviceInfo
    QDomElement deviceInfoElement = doc.createElement("DeviceInfo");
    deviceElement.appendChild(deviceInfoElement);

    QDomElement nameElement = doc.createElement("Name");
    nameElement.setAttribute("name", "local:ModelName");
    //nameElement.appendChild(doc.createTextNode("GT-1B7F"));
    nameElement.appendChild(doc.createTextNode("MyDrivverSteppingMoror"));
    deviceInfoElement.appendChild(nameElement);

    QDomElement descElement = doc.createElement("Description");
    descElement.setAttribute("name", "local:DeviceDescription");
    descElement.appendChild(doc.createTextNode("2-Phase Bipolar Stepping Motor Driver, 24Vdc 1A"));
    deviceInfoElement.appendChild(descElement);

    QDomElement vendorElement = doc.createElement("Vendor");
    vendorElement.setAttribute("name", "local:VendorName");
    vendorElement.appendChild(doc.createTextNode("CREVIS CO.,LTD"));
    deviceInfoElement.appendChild(vendorElement);

    QDomElement orderNumElement = doc.createElement("OrderNumber");
    //orderNumElement.appendChild(doc.createTextNode("GT-1B7F"));
    orderNumElement.appendChild(doc.createTextNode("MyDrivverSteppingMoror"));
    deviceInfoElement.appendChild(orderNumElement);

    QDomElement imageElement = doc.createElement("Image");
    imageElement.setAttribute("name", "local:image_io");
    //imageElement.appendChild(doc.createTextNode("GT-1B7F.gif"));
    imageElement.appendChild(doc.createTextNode("MyDrivverSteppingMoror.gif"));
    deviceInfoElement.appendChild(imageElement);

    QDomComment connectorComment = doc.createComment("Connector moduleType=\"257\" interface=\"Common.PCI\" role=\"child\" explicit=\"false\" connectorId=\"1\" hostpath=\"-1\">\n      <InterfaceName name=\"local:PCI\">PCI-Bus</InterfaceName>\n      <Slot count=\"1\" allowEmpty=\"false\"></Slot>\n    </Connector");
    deviceElement.appendChild(connectorComment);

    QDomElement connector1Element = doc.createElement("Connector");
    connector1Element.setAttribute("moduleType", "47000");
    connector1Element.setAttribute("interface", "CVS.OptionG");
    connector1Element.setAttribute("role", "child");
    connector1Element.setAttribute("explicit", "false");
    connector1Element.setAttribute("connectorId", "1");
    connector1Element.setAttribute("hostpath", "-1");

    QDomElement interfaceName1Element = doc.createElement("InterfaceName");
    interfaceName1Element.setAttribute("name", "local:PCI");
    interfaceName1Element.appendChild(doc.createTextNode("CVS Option"));
    connector1Element.appendChild(interfaceName1Element);

    QDomElement slotElement = doc.createElement("Slot");
    slotElement.setAttribute("count", "1");
    slotElement.setAttribute("allowEmpty", "false");
    connector1Element.appendChild(slotElement);

    deviceElement.appendChild(connector1Element);

    QDomElement connector2Element = doc.createElement("Connector");
    connector2Element.setAttribute("moduleType", "40302");
    connector2Element.setAttribute("interface", "MyCompany:Internal");
    connector2Element.setAttribute("role", "parent");
    connector2Element.setAttribute("explicit", "false");
    connector2Element.setAttribute("connectorId", "2");
    connector2Element.setAttribute("hostpath", "1");
    deviceElement.appendChild(connector2Element);

    QDomElement interfaceName2Element = doc.createElement("InterfaceName");
    interfaceName2Element.setAttribute("name", "local:DP");
    interfaceName2Element.appendChild(doc.createTextNode("ETC IOs"));
    connector2Element.appendChild(interfaceName2Element);

    QDomElement varElement = doc.createElement("Var");
    varElement.setAttribute("max", "8");
    connector2Element.appendChild(varElement);

    QDomElement driverInfoElement = doc.createElement("DriverInfo");
    driverInfoElement.setAttribute("needsBusCycle", "true");
    connector2Element.appendChild(driverInfoElement);

    // HostParameterSet
    QDomElement hostParamSetElement = doc.createElement("HostParameterSet");
    connector2Element.appendChild(hostParamSetElement);

    struct ParamDef {
        QString id;
        QString type;
        QString channel;
        QString download;
        QString functional;
        QString offlineAccess;
        QString onlineAccess;
        QString defaultValue;
        QString nameKey;
        QString nameValue;
        QString descKey;
        QString descValue;
    };

    std::vector<ParamDef> params = {
        {"1000", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in0", "IN0", "", ""},
        {"1001", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in1", "IN1", "", ""},
        {"1002", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in2", "IN2", "", ""},
        {"1003", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in3", "IN3", "", ""},
        {"1004", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in4", "IN4", "", ""},
        {"1005", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in5", "IN5", "", ""},
        {"1006", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in6", "IN6", "", ""},
        {"1007", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in7", "IN7", "", ""},
        {"1008", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in8", "IN8", "", ""},
        {"1009", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in9", "IN9", "", ""},
        {"1010", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in10", "IN10", "", ""},
        {"1011", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in11", "IN11", "", ""},
        {"1012", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in12", "IN12", "", ""},
        {"1013", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in13", "IN13", "", ""},
        {"1014", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in14", "IN14", "", ""},
        {"1015", "std:WORD", "input", "true", "false", "read", "read", "0", "local:in15", "IN15", "", ""},

        {"2000", "std:WORD", "input", "true", "false", "read", "read", "0", "local:out0", "OUT0", "", ""},
        {"2001", "std:WORD", "input", "true", "false", "read", "read", "0", "local:out1", "OUT1", "", ""},
        {"2002", "std:WORD", "input", "true", "false", "read", "read", "0", "local:out2", "OUT2", "", ""},
        {"2003", "std:WORD", "input", "true", "false", "read", "read", "0", "local:out3", "OUT3", "", ""},
        {"2004", "std:WORD", "input", "true", "false", "read", "read", "0", "local:out4", "OUT4", "", ""},
        {"2005", "std:WORD", "input", "true", "false", "read", "read", "0", "local:out5", "OUT5", "", ""},
        {"2006", "std:WORD", "input", "true", "false", "read", "read", "0", "local:out6", "OUT6", "", ""},
        {"2007", "std:WORD", "input", "true", "false", "read", "read", "0", "local:out7", "OUT7", "", ""},
        {"2008", "std:WORD", "input", "true", "false", "read", "read", "0", "local:out8", "OUT8", "", ""},
        {"2009", "std:WORD", "input", "true", "false", "read", "read", "0", "local:out9", "OUT9", "", ""},

        {"393218", "std:STRING", "none", "true", "false", "read", "read", "'CREVIS'", "local:Id393218", "Vendor", "local:Id393218.Desc", "Vendor of the device"},
        {"393219", "std:STRING", "none", "true", "false", "read", "read", "'MyDrivverSteppingMotor'", "local:Id393219", "Module ID", "local:Id393219.Desc", "Module ID of the device"},

        {"400000", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400000", "Parameter 0", "local:Id400000.Desc", "Acceleration Current(in peak current)"},
        {"400001", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400001", "Parameter 1", "local:Id400001.Desc", "Deceleration Current(in peak current)"},
        {"400002", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400002", "Parameter 2", "local:Id400002.Desc", "Holding Current(in peak current)"},
        {"400003", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400003", "Parameter 3", "local:Id400003.Desc", "Running Current(in peak current)"},
        {"400004", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400004", "Parameter 4", "local:Id400004.Desc", "Reserved"},
        {"400005", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400005", "Parameter 5", "local:Id400005.Desc", "Step mode and Encoder/Digital Input selection"},
        {"400006", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400006", "Parameter 6", "local:Id400006.Desc", "Acceleration Speed"},
        {"400007", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400007", "Parameter 7", "local:Id400007.Desc", "Deceleration Speed"},
        {"400008", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400008", "Parameter 8", "local:Id400008.Desc", "Maximum Speed"},
        {"400009", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400009", "Parameter 9", "local:Id400009.Desc", "Minimum Speed"},
        {"400010", "std:BYTE", "none", "true", "false", "readwrite", "read", "0", "local:Id400010", "Parameter 10", "local:Id400010.Desc","Full Step Speed"},

    };

    for (const auto& pDef : params) {
        QDomElement paramElement = doc.createElement("Parameter");
        paramElement.setAttribute("ParameterId", pDef.id);
        paramElement.setAttribute("type", pDef.type);
        hostParamSetElement.appendChild(paramElement);

        QDomElement attributesElement = doc.createElement("Attributes");
        attributesElement.setAttribute("channel", pDef.channel);
        attributesElement.setAttribute("download", pDef.download);
        attributesElement.setAttribute("functional", pDef.functional);
        attributesElement.setAttribute("offlineaccess", pDef.offlineAccess);
        attributesElement.setAttribute("onlineaccess", pDef.onlineAccess);
        paramElement.appendChild(attributesElement);

        QDomElement defaultElement = doc.createElement("Default");
        defaultElement.appendChild(doc.createTextNode(pDef.defaultValue));
        paramElement.appendChild(defaultElement);

        QDomElement nameElement = doc.createElement("Name");
        nameElement.setAttribute("name", pDef.nameKey);
        nameElement.appendChild(doc.createTextNode(pDef.nameValue));
        paramElement.appendChild(nameElement);

        if (!pDef.descKey.isEmpty() && !pDef.descValue.isEmpty()) {
            QDomElement descElement = doc.createElement("Description");
            descElement.setAttribute("name", pDef.descKey);
            descElement.appendChild(doc.createTextNode(pDef.descValue));
            paramElement.appendChild(descElement);
        }
    }

    return doc;
}

QDomDocument createNewDeviceDescriptionDocumentBasedOnType(DeviceType type) {
    switch (type) {
        case DeviceType::DiscreteIO:
            return createTemplateForDiscreteIO();
        case DeviceType::DiscreteInput:
            return createTemplateForDiscreteInput();
        case DeviceType::DiscreteOutput:
            return createTemplateForDiscreteOutput();
        case DeviceType::AnalogInput:
            return createTemplateForAnalogInput();
        case DeviceType::AnalogOutput:
            return createTemplateForAnalogOutput();
//        case DeviceType::Specialized:
//            return createTemplateForSpecialized();
        case DeviceType::RS232:
            return createTemplateForRS232();
        case DeviceType::RS422:
            return createTemplateForRS422();
        case DeviceType::RS485:
            return createTemplateForRS485();
        case DeviceType::SSI:
            return createTemplateForSSI();
        case DeviceType::Encoder:
            return createTemplateForEncoder();
        case DeviceType::PWM:
            return createTemplateForPWM();
        case DeviceType::Drivver_Stepping_Moror:
            return createTemplateForDrivverSteppingMotor();
        case DeviceType::Power:
            return createTemplateForPower();
        default:
            return createTemplateForDiscreteIO(); // Значение по умолчанию
    }
}
