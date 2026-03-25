#include "xmlvalidator.h"
#include <QRegExp>
#include <QStringList>

XmlValidator::XmlValidator(QObject *parent)
    : QObject(parent)
{
}

XmlValidator::ValidationResult XmlValidator::validateXml(const QString &xmlContent, ValidationTypes types)
{
    ValidationResult result;
    result.isValid = true;
    result.errorCount = 0;
    result.warningCount = 0;
    
    QString errorMessage;
    int errorLine, errorColumn;
    QString check;
    QString temp;
    
    // Well-formed XML check
    if (types.testFlag(WellFormedCheck)) {
        if (!isWellFormed(xmlContent, errorMessage, errorLine, errorColumn)) {
            addError(result, tr("XML не является правильно сформированным"), errorLine, errorColumn,
                    tr("Правильно сформированный"), errorMessage);
            result.isValid = false;
        }
        temp = tr(" (Проверка правильности XML) ");
        check += temp;
    }
    
    // Syntax check
    if (types.testFlag(SyntaxCheck)) {
        if (!checkSyntax(xmlContent, errorMessage, errorLine, errorColumn)) {
            addError(result, tr("Ошибка синтаксиса XML"), errorLine, errorColumn,
                    tr("Ошибка синтаксиса XML"), errorMessage);
            result.isValid = false;
        }
        temp = tr(" (Проверка синтаксиса) ");
        check += temp;
    }
    
    // Element names check
    if (types.testFlag(ElementNameCheck)) {
        if (!checkElementNames(xmlContent, errorMessage, errorLine, errorColumn)) {
            addError(result, tr("Недопустимое имя элемента"), errorLine, errorColumn,
                    tr("Ммя элемента"), errorMessage);
            result.isValid = false;
        }
        temp = tr(" (Проверка имен элементов) ");
        check += temp;
    }
    
    // Attribute syntax check
    if (types.testFlag(AttributeCheck)) {
        if (!checkAttributeSyntax(xmlContent, errorMessage, errorLine, errorColumn)) {
            addError(result, tr("Ошибка синтаксиса атрибута"), errorLine, errorColumn,
                    tr("Атрибут"), errorMessage);
            result.isValid = false;
        }
        temp = tr(" (Проверка синтаксиса атрибутов) ");
        check += temp;
    }
    
    // Character data check
    if (types.testFlag(CharacterDataCheck)) {
        if (!checkCharacterData(xmlContent, errorMessage, errorLine, errorColumn)) {
            addError(result, tr("Недопустимые символьные данные"), errorLine, errorColumn,
                    tr("Символы"), errorMessage);
            result.isValid = false;
        }
        temp = tr(" (Проверка символов) ");
        check += temp;
    }
    
    // Namespace check
    if (types.testFlag(NamespaceCheck)) {
        if (!checkNamespaceDeclarations(xmlContent, errorMessage, errorLine, errorColumn)) {
            addWarning(result, tr("Проблема с объявлением пространства имен"), errorLine, errorColumn,
                      tr("Пространства имен"), errorMessage);
        }
        temp = tr(" (Проверка пространства имен) ");
        check += temp;
    }
    
    // Entity check
    if (types.testFlag(EntityCheck)) {
        if (!checkEntityReferences(xmlContent, errorMessage, errorLine, errorColumn)) {
            addWarning(result, tr("Проблема со ссылкой на сущность"), errorLine, errorColumn,
                      tr("Entity"), errorMessage);
        }
        temp = tr(" (Проверка на ссылки на сущности) ");
        check += temp;
    }
    
    // Processing instruction check
    if (types.testFlag(ProcessingInstructionCheck)) {
        if (!checkProcessingInstructions(xmlContent, errorMessage, errorLine, errorColumn)) {
            addError(result, tr("Ошибка инструкции по обработке"), errorLine, errorColumn,
                    tr("Инструкция по обработке"), errorMessage);
            result.isValid = false;
        }
        temp = tr(" (Проверка инструкции по обработке) ");
        check += temp;
    }
    
    // Comment check
    if (types.testFlag(CommentCheck)) {
        if (!checkComments(xmlContent, errorMessage, errorLine, errorColumn)) {
            addError(result, tr("Ошибка синтаксиса комментария"), errorLine, errorColumn,
                    tr("Комментарии"), errorMessage);
            result.isValid = false;
        }
        temp = tr(" (Проверка комментариев) ");
        check += temp;
    }
    
    // Declaration check
    if (types.testFlag(DeclarationCheck)) {
        if (!checkXmlDeclaration(xmlContent, errorMessage, errorLine, errorColumn)) {
            addError(result, tr("Ошибка декларации XML"), errorLine, errorColumn,
                    tr("Декларации"), errorMessage);
            result.isValid = false;
        }
        temp = tr(" (Проверка декларации) ");
        check += temp;
    }
    
    // Generate summary
    if (result.errors.isEmpty()) {
        result.summary = tr("XML документ действителен ") + check;
    } else {
        result.summary = QString(tr("Проверка завершена с %1 errors and %2 warnings"))
                        .arg(result.errorCount).arg(result.warningCount);
    }
    
    return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Выполняет базовую, но критически важную проверку XML-документа на соответствие фундаментальным правилам формирования, известным
// как "well-formedness" (корректность/согласованность структуры).
// Убедиться, что XML-документ является корректно сформированным. Это означает, что он соответствует самым базовым синтаксическим правилам XML,
// таким как правильное вложение тегов, наличие корневого элемента, корректное использование специальных символов и т.д. Это не полная валидация по схеме (DTD, XSD), а проверка на базовую структурную целостность.
bool XmlValidator::isWellFormed(const QString &xmlContent, QString &errorMessage, int &errorLine, int &errorColumn)
{
    QXmlStreamReader reader(xmlContent);
    
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.hasError()) {
            errorMessage = reader.errorString();
            errorLine = reader.lineNumber();
            errorColumn = reader.columnNumber();
            return false;
        }
    }
    
    errorMessage.clear();
    errorLine = -1;
    errorColumn = -1;
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Выполняет базовую проверку синтаксиса XML-документа
// Проверяет два фундаментальных правила:
// Документ не должен быть полностью пустым (после игнорирования пробелов)
// В документе должен присутствовать хотя бы один элемент (который становится корневым)
bool XmlValidator::checkSyntax(const QString &xmlContent, QString &errorMessage, int &errorLine, int &errorColumn)
{
    // trimmed() удаляет пробельные символы (пробелы, табуляции, переводы строк) с начала и конца строки
    if (xmlContent.trimmed().isEmpty()) {
        errorMessage = tr("XML document is empty");
        errorLine = 1;
        errorColumn = 1;
        return false;
    }
    
    // Check for root element
    QXmlStreamReader reader(xmlContent);
    bool hasRootElement = false;
    
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.isStartElement()) {
            hasRootElement = true;
            break; // Найден первый начальный тег, это корень
        }
    }
    
    if (!hasRootElement) {
        errorMessage = tr("XML документ должен иметь корневой элемент");
        errorLine = 1;
        errorColumn = 1;
        return false;
    }
    
    errorMessage.clear();
    errorLine = -1;
    errorColumn = -1;
    return true;
}

bool XmlValidator::checkElementNames(const QString &xmlContent, QString &errorMessage, int &errorLine, int &errorColumn)
{
    // Check element names
    QRegExp elementRegex("<([A-Za-z_:][A-Za-z0-9_:.-]*)");
    // Регулярное выражение <([A-Za-z_:][A-Za-z0-9_:.-]*) для поиска открывающих тегов. Оно ищет символ <, за которым следует имя элемента.
    // ([A-Za-z_:][A-Za-z0-9_:.-]*) - это группа захвата, которая выделяет само имя
    // [A-Za-z_:] - Первый символ имени должен быть буквой (латинской), подчеркиванием _ или двоеточием :.
    // [A-Za-z0-9_:.-]* - Последующие символы могут быть буквами, цифрами, подчеркиванием, двоеточием, точкой . или дефисом -.
    int pos = 0;
    
    while ((pos = elementRegex.indexIn(xmlContent, pos)) != -1) {
        QString elementName = elementRegex.cap(1);
        int namePos = pos + 1;  // +1 для пропуска '<'
        
        // Проверяет, соответствует ли имя правилам XML
        if (!isValidXmlName(elementName)) {
            errorMessage = QString(tr("Недопустимое имя элемента: '%1'")).arg(elementName);
            errorLine = getLineFromPosition(xmlContent, namePos);
            errorColumn = getColumnFromPosition(xmlContent, namePos);
            return false;
        }
        
        // Проверяет использование зарезервированных имен типа "xml", "xmlns"
        if (isReservedXmlName(elementName)) {
            errorMessage = QString(tr("Используется зарезервированное имя: '%1'")).arg(elementName);
            errorLine = getLineFromPosition(xmlContent, namePos);
            errorColumn = getColumnFromPosition(xmlContent, namePos);
            return false;
        }
        
        pos += elementRegex.matchedLength();
    }
    
    // Check names starting with digits
    QRegExp invalidStartRegex("<([0-9][^\\s>]*)");
    // Регулярное выражение <([0-9][^\\s>]*) ищет теги, имена которых начинаются с цифры
    int invalidPos = invalidStartRegex.indexIn(xmlContent);
    
    if (invalidPos != -1) {
        QString invalidName = invalidStartRegex.cap(1);
        int nameStartPos = invalidPos + 1;  // +1 для пропуска '<'
        errorMessage = QString(tr("Имена элементов не могут начинаться с цифры: '%1'")).arg(invalidName);
        errorLine = getLineFromPosition(xmlContent, nameStartPos);
        errorColumn = getColumnFromPosition(xmlContent, nameStartPos);
        return false;
    }
    
    errorMessage.clear();
    errorLine = -1;
    errorColumn = -1;
    return true;
}

bool XmlValidator::checkAttributeSyntax(const QString &xmlContent, QString &errorMessage, int &errorLine, int &errorColumn)
{
    // Check attribute names using QRegExp
    QRegExp attrRegex("\\s+([A-Za-z_:][A-Za-z0-9_:.-]*)\\s*=\\s*");
    // Проверка имен атрибутов:
    // Регулярное выражение \\s+([A-Za-z_:][A-Za-z0-9_:.-]*)\\s*=\\s* для поиска атрибутов
    // \\s+ - Один или несколько пробельных символов (пробел, табуляция и т.д.) до имени атрибута
    // ([A-Za-z_:][A-Za-z0-9_:.-]*) - Группа захвата для имени атрибута (те же правила, что и для имен элементов)
    // \\s*=\\s* - Знак равенства =, возможно окружённый пробелами
    int pos = 0;

    while ((pos = attrRegex.indexIn(xmlContent, pos)) != -1) {
        QString attrName = attrRegex.cap(1); // Получаем имя атрибута
        int namePos = pos + xmlContent.mid(pos).indexOf(attrName); // Получаем позицию начала имени

        if (!isValidXmlName(attrName)) { // Проверяем имя
            errorMessage = QString(tr("Недопустимое имя атрибута: '%1'")).arg(attrName);
            errorLine = getLineFromPosition(xmlContent, namePos);
            errorColumn = getColumnFromPosition(xmlContent, namePos);
            return false; // Ошибка найдена
        }

        pos += attrRegex.matchedLength();
    }

    // Check attribute value quoting
    QRegExp unquotedAttrRegex("\\s+([A-Za-z_:][^\\s=]*)\\s*=\\s*([^\"'][^\\s>]*)");
    // Проверка кавычек вокруг значений атрибутов:
    // Регулярное выражение \\s+([A-Za-z_:][^\\s=]*)\\s*=\\s*([^\"'][^\\s>]*) для поиска некорректных атрибутов
    // \\s+([A-Za-z_:][^\\s=]*)\\s*=\\s* - Имя атрибута (как и выше) и знак равенства
    // ([^\"'][^\\s>]*) - Группа захвата 2: Значение атрибута, которое не начинается с двойной кавычки " или
    // одинарной кавычки ' ([^\"']), и состоит из символов, не являющихся пробелом или > ([^\\s>]*)
    int unquotedPos = unquotedAttrRegex.indexIn(xmlContent);

    if (unquotedPos != -1) {
        QString attrName = unquotedAttrRegex.cap(1); // Имя атрибута с некорректным значением
        QString attrValue = unquotedAttrRegex.cap(2); // Значение без кавычек
        int errorPos = unquotedPos; // Позиция ошибки
        errorMessage = QString(tr("Значение атрибута должно быть указано в кавычках: %1=\"%2\"")).arg(attrName, attrValue);
        errorLine = getLineFromPosition(xmlContent, errorPos);
        errorColumn = getColumnFromPosition(xmlContent, errorPos);
        return false; // Ошибка найдена
    }

    errorMessage.clear();
    errorLine = -1;
    errorColumn = -1;
    return true;
}

bool XmlValidator::checkCharacterData(const QString &xmlContent, QString &errorMessage, int &errorLine, int &errorColumn)
{
    // Check for invalid XML characters
    for (int i = 0; i < xmlContent.length(); ++i) {
        QChar ch = xmlContent[i];
        if (!isValidXmlCharacter(ch)) {
            errorMessage = QString(tr("Недопустимый символ XML: U+%1")).arg(ch.unicode(), 4, 16, QChar('0'));
            errorLine = getLineFromPosition(xmlContent, i);
            errorColumn = getColumnFromPosition(xmlContent, i);
            return false;
        }
    }
    
    errorMessage.clear();
    errorLine = -1;
    errorColumn = -1;
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Cканирует текст XML-документа, находит все объявления пространств имён вида xmlns:префикс="URI" и проверяет,
// не используются ли в них зарезервированные префиксы xml или xmlns.
bool XmlValidator::checkNamespaceDeclarations(const QString &xmlContent, QString &errorMessage, int &errorLine, int &errorColumn)
{
    // Check namespace declarations using QRegExp
    QRegExp xmlnsRegex("xmlns:([A-Za-z][A-Za-z0-9_.-]*)\\s*=\\s*\"([^\"]*)\"");
    // Регулярное выражение "xmlns:([A-Za-z][A-Za-z0-9_.-]*)\\s*=\\s*\"([^\"]*)\"" для поиска объявлений пространств имён
    // xmlns: - Ищет литерал xmlns:
    // ([A-Za-z][A-Za-z0-9_.-]*) - Группа захвата 1 (cap(1)): Захватывает префикс пространства имён. Он должен начинаться
    // с буквы и может содержать буквы, цифры, точки, подчеркивания и дефисы
    // \\s*=\\s* - Знак равенства, возможно окружённый пробелами
    // \"([^\"]*)\" - Группа захвата 2 (cap(2)): Захватывает URI пространства имён, заключённый в двойные кавычки
    int pos = 0;

    while ((pos = xmlnsRegex.indexIn(xmlContent, pos)) != -1) {
        QString prefix = xmlnsRegex.cap(1); // Получаем префикс
        QString uri = xmlnsRegex.cap(2);    // Получаем URI (может использоваться для других проверок)
        int prefixPos = pos + xmlContent.mid(pos).indexOf(prefix); // Получаем позицию начала префикса

        // Согласно спецификации XML и Namespaces, префиксы xml и xmlns зарезервированы
        // Если найденный префикс равен "xml" или "xmlns", это является ошибкой
        if (prefix == "xml" || prefix == "xmlns") {
            errorMessage = QString(tr("Используется зарезервированный префикс пространства имен: '%1'")).arg(prefix);
            errorLine = getLineFromPosition(xmlContent, prefixPos);
            errorColumn = getColumnFromPosition(xmlContent, prefixPos);
            return false;
        }

        pos += xmlnsRegex.matchedLength();
    }

    errorMessage.clear();
    errorLine = -1;
    errorColumn = -1;
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Проверяет на ссылки на сущности (например, &amp;, &lt;, &customEntity;) и проверить их корректность. В данном контексте она, используется
// для предупреждения о пользовательских (custom) сущностях, так как XML-парсер Qt, используемый в других частях валидатора, может не разрешать их напрямую без DTD
// DTD (Document Type Definition) в XML — это стандарт описания структуры XML-документа или его части. Он действует как схема, задавая элементы, атрибуты и другие особенности документа.
// Выдает предупреждение, если обнаруживает ссылку на пользовательскую (нестандартную) сущность. Она не помечает документ как недействительный из-за этого, но информирует
// пользователя о потенциальной проблеме
bool XmlValidator::checkEntityReferences(const QString &xmlContent, QString &errorMessage, int &errorLine, int &errorColumn)
{
    // Check entity references using QRegExp
    QRegExp entityRegex("&([A-Za-z][A-Za-z0-9_.:-]*);");
    // Регулярное выражение &([A-Za-z][A-Za-z0-9_.:-]*); для поиска ссылок на сущности
    // & - Ищет символ амперсанда, с которого начинается ссылка на сущность
    // ([A-Za-z][A-Za-z0-9_.:-]*) - Группа захвата 1 (cap(1)): Захватывает имя сущности. Оно должно начинаться с буквы и может содержать
    // буквы, цифры, точки, подчеркивания, дефисы и двоеточия
    // ; - Ищет точку с запятой, которой должна заканчиваться ссылка на сущность
    int pos = 0;

    while ((pos = entityRegex.indexIn(xmlContent, pos)) != -1) {
        QString entityName = entityRegex.cap(1);
        int entityPos = pos; // Позиция начала сущности

        // Check standard entities
        QStringList standardEntities;
        standardEntities << tr("lt") << tr("gt") << tr("amp") << tr("apos") << tr("quot");
        // Список имен стандартных предопределенных сущностей XML: lt (<), gt (>), amp (&), apos ('), quot (")
        // Если имя не стандартное (т.е. это пользовательская сущность, например, &myEntity;), это может быть потенциальная проблема, если DTD, определяющая
        // эту сущность, отсутствует или не будет обработана внешним парсером.
        // В этом случае формируется предупреждение (errorMessage), указывающее на пользовательскую ссылку на сущность
        if (!standardEntities.contains(entityName)) {
            errorMessage = QString(tr("Ссылка на пользовательский объект: &%1;")).arg(entityName);
            errorLine = getLineFromPosition(xmlContent, entityPos);
            errorColumn = getColumnFromPosition(xmlContent, entityPos);
            return true; // Warning, not error
        }

        pos += entityRegex.matchedLength();
    }

    errorMessage.clear();
    errorLine = -1;
    errorColumn = -1;
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Проверка корректности инструкций обработки (Processing Instructions, PI) в XML-документе
// Cканирует текст XML-документа, находит все инструкции обработки (<?имя ... ?>) и проверяет, не нарушена ли специфическое правило: XML-декларация (<?xml ... ?>)
// должна быть первым символом документа, если она присутствует
bool XmlValidator::checkProcessingInstructions(const QString &xmlContent, QString &errorMessage, int &errorLine, int &errorColumn)
{
    // Check processing instructions using QRegExp
    QRegExp piRegex("<\\?([A-Za-z][A-Za-z0-9_.:-]*)");
    // Регулярное выражение <\\?([A-Za-z][A-Za-z0-9_.:-]*) для поиска инструкций обработки
    // <\\? - Ищет литерал <?, с которого начинается PI
    // ([A-Za-z][A-Za-z0-9_.:-]*) - Группа захвата 1 (cap(1)): Захватывает имя инструкции обработки. Оно должно начинаться с буквы и может содержать буквы,
    // цифры, точки, подчеркивания, дефисы и двоеточия
    // !!! Внимание, что регулярное выражение не ищет закрывающую часть ?>. Оно останавливается на имени
    int pos = 0;

    while ((pos = piRegex.indexIn(xmlContent, pos)) != -1) {
        QString piName = piRegex.cap(1); // Получаем имя PI
        int namePos = pos + 2; // Позиция начала имени (после <?)

        if (piName.toLower() == tr("xml")) {
            // Если имя найденной инструкции обработки (в нижнем регистре) равно "xml", это означает, что это XML-декларация (<?xml version="..."?>)
            if (xmlContent.indexOf(tr("<?xml")) != 0) {
                // Проверяется, находится ли она в самом начале документа, используя xmlContent.indexOf("<?xml") != 0
                errorMessage = tr("XML-декларация должна быть в начале документа");
                errorLine = getLineFromPosition(xmlContent, namePos);
                errorColumn = getColumnFromPosition(xmlContent, namePos);
                return false;
            }
        }

        pos += piRegex.matchedLength();
    }

    errorMessage.clear();
    errorLine = -1;
    errorColumn = -1;
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Проверку корректности комментариев в XML-документе
// Убедиться, что все комментарии в XML-документе синтаксически корректны, а именно, что они не содержат последовательность символов --
// Согласно стандарту XML 1.0:
// Комментарий имеет вид <!-- текст комментария -->
// Важное правило: Строка -- (два дефиса подряд) запрещена внутри комментария. Это означает, что комментарий не может заканчиваться
// на - (если за ним не следует >), и нельзя иметь -- в середине текста
bool XmlValidator::checkComments(const QString &xmlContent, QString &errorMessage, int &errorLine, int &errorColumn)
{
    // Check comments using QRegExp
    QRegExp commentRegex(tr("<!--(.*?)-->"));
    // Регулярное выражение <!--(.*?)--> для поиска комментариев
    // <!-- - Ищет начало комментария
    // (.*?) - Группа захвата 1 (cap(1)): Нежадно захватывает весь текст внутри комментария (между <!-- и -->). .*? означает "любые символы,
    // но как можно меньше, до тех пор, пока не встретится следующая часть шаблона"
    // --> - Ищет конец комментария
    // Внимание: QRegExp по умолчанию не поддерживает DotMatchesEverythingOption, поэтому многострочные комментарии нужно обрабатывать отдельно

    // Включаем поддержку многострочных совпадений
    commentRegex.setMinimal(true); // Нежадный режим

    int pos = 0;

    while ((pos = commentRegex.indexIn(xmlContent, pos)) != -1) {
        QString commentContent = commentRegex.cap(1); // Получаем содержимое комментария
        int contentPos = pos + 4; // Позиция начала содержимого (после <!--)

        // Проверка на запрещенную последовательность --
        if (commentContent.contains(tr("--"))) {
            errorMessage = tr("Комментарии не могут содержать '--'");
            int errorPos = contentPos + commentContent.indexOf(tr("--"));
            errorLine = getLineFromPosition(xmlContent, errorPos);
            errorColumn = getColumnFromPosition(xmlContent, errorPos);
            return false;
        }

        pos += commentRegex.matchedLength();
    }

    errorMessage.clear();
    errorLine = -1;
    errorColumn = -1;
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Проверяет корректность XML-декларации (<?xml ... ?>) в XML-документе
// Убедиться, что XML-декларация, если она присутствует, синтаксически корректна (в частности, правильно закрыта)
bool XmlValidator::checkXmlDeclaration(const QString &xmlContent, QString &errorMessage, int &errorLine, int &errorColumn)
{
    // Check XML declaration
    // Проверяет, начинается ли весь документ с подстроки <?xml
    // Если документ не начинается с <?xml, функция считает, что декларации нет, и, следовательно, проверять нечего
    if (xmlContent.startsWith(tr("<?xml"))) {
        // Проверка правильного закрытия
        int endPos = xmlContent.indexOf("?>");
        if (endPos == -1) {
            errorMessage = tr("Декларация XML не закрыта должным образом");
            errorLine = 1;
            errorColumn = 1;
            return false;
        }
    }
    
    errorMessage.clear();
    errorLine = -1;
    errorColumn = -1;
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Использует мощь встроенного XML-парсера Qt (QXmlStreamReader) для проверки баланса тегов. Парсер сам обнаруживает ситуацию, когда документ
// заканчивается, но остались незакрытые теги (PrematureEndOfDocumentError), и функция сообщает об этом как об ошибке баланса тегов.
bool XmlValidator::checkTagBalancing(const QString &xmlContent, QString &errorMessage, int &errorLine, int &errorColumn)
{
    QXmlStreamReader reader(xmlContent);
    
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.hasError()) {
            // QXmlStreamReader::PrematureEndOfDocumentError - это специфическая ошибка, которая часто возникает, когда документ заканчивается преждевременно, например,
            // когда есть незакрытые теги. Если reader.error() возвращает именно это значение, это сильный признак недостающего закрывающего тега
            // Формируется сообщение об ошибке "Unbalanced tags - missing closing tag". Получают номер строки (reader.lineNumber()) и столбца (reader.columnNumber()), где парсер обнаружил проблему.
            if (reader.error() == QXmlStreamReader::PrematureEndOfDocumentError) {
                errorMessage = tr("Несбалансированные теги — отсутствует закрывающий тег");
                errorLine = reader.lineNumber();
                errorColumn = reader.columnNumber();
                return false;
            }
        }
    }
    
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Проверку правильного вложения тегов в XML-документе. Это означает, что каждый открывающий тег <тег> должен быть закрыт </тег> в правильном порядке, соответствующем структуре "стека".
// Например, это правильно:
// <root>
//   <parent>
//     <child>...</child>
//   </parent>
// </root>
// Открывается root, затем parent, затем child. Закрывается child, затем parent, затем root.
// А это неправильно (неправильное вложение):
// <root>
//   <parent>
//     <child>...</parent> <!-- Ошибка: parent закрывается до child -->
//   </child> <!-- Ошибка: child закрывается после parent -->
// </root>
// Убедиться, что теги в XML-документе вложены правильно, как матрешки. Открывающие и закрывающие теги идут парами и соблюдают порядок "последним открылся – первым закрылся" (LIFO - Last In, First Out).
bool XmlValidator::checkProperNesting(const QString &xmlContent, QString &errorMessage, int &errorLine, int &errorColumn)
{
    // Check proper tag nesting
    // Используем QStringList как стек (структура данных, работающая по принципу LIFO - Last In, First Out)
    // В него будут помещаться имена открывающихся тегов.
    // При закрытии тега из стека будет извлекаться имя последнего открытого тега для сравнения.
    QStringList tagStack;
    QRegExp tagRegex("<(/?)([A-Za-z_:][A-Za-z0-9_:.-]*)[^>]*>");
    // Регулярное выражение <(/?)([A-Za-z_:][A-Za-z0-9_:.-]*)[^>]*> используется для поиска всех тегов (и открывающих, и закрывающих)
    // < - Начало тега
    // (/?) - Группа захвата 1 (cap(1)): Захватывает необязательный символ /. Если / найден, это означает закрывающий тег (</...>), если нет - открывающий (<...>)
    // ([A-Za-z_:][A-Za-z0-9_:.-]*) - Группа захвата 2 (cap(2)): Захватывает имя тега
    // [^>]* - Любые символы, кроме >, до конца тега
    // > - Конец тега
    int pos = 0;

    while ((pos = tagRegex.indexIn(xmlContent, pos)) != -1) {
        bool isClosing = !tagRegex.cap(1).isEmpty(); // Проверяем, есть ли '/' в группе 1
        QString tagName = tagRegex.cap(2);           // Получаем имя тега из группы 2
        int tagPos = pos;                            // Получаем позицию начала тега

        // Обработка открывающего тега (Opening tag)
        if (!isClosing) {
            tagStack.append(tagName); // push в стек
        } else {
            // Обработка закрывающего тега (Closing tag)
            if (tagStack.isEmpty()) {
                errorMessage = QString(tr("Неожиданный закрывающий тег: </%1>")).arg(tagName);
                errorLine = getLineFromPosition(xmlContent, tagPos);
                errorColumn = getColumnFromPosition(xmlContent, tagPos);
                return false;
            }

            QString expectedTag = tagStack.takeLast(); // pop из стека (извлекаем последний элемент)
            // Если имена НЕ совпадают (expectedTag != tagName), это означает неправильное вложение
            if (expectedTag != tagName) {
                errorMessage = QString(tr("Несовпадающие теги: ожидаемо </%1>, found </%2>"))
                              .arg(expectedTag, tagName);
                errorLine = getLineFromPosition(xmlContent, tagPos);
                errorColumn = getColumnFromPosition(xmlContent, tagPos);
                return false;
            }
        }

        pos += tagRegex.matchedLength();
    }

    if (!tagStack.isEmpty()) {
        errorMessage = QString(tr("Незакрытый тег: <%1>")).arg(tagStack.last());
        errorLine = -1;
        errorColumn = -1;
        return false;
    }

    errorMessage.clear();
    errorLine = -1;
    errorColumn = -1;
    return true;
}

// Utility methods
void XmlValidator::addError(ValidationResult &result, const QString &message, int line, int column, 
                           const QString &type, const QString &details)
{
    ValidationError error;
    error.message = message;
    error.line = line;
    error.column = column;
    error.errorType = type;
    error.details = details;
    error.severity = tr("Error");
    result.errors.append(error);
    result.errorCount++;
}

void XmlValidator::addWarning(ValidationResult &result, const QString &message, int line, int column, 
                             const QString &type, const QString &details)
{
    ValidationError error;
    error.message = message;
    error.line = line;
    error.column = column;
    error.errorType = type;
    error.details = details;
    error.severity = tr("Warning");
    result.errors.append(error);
    result.warningCount++;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Проверяет правила формирования имен XML 1.0:
// Имя не должно быть пустым
// Первый символ должен быть буквой, подчеркиванием _ или двоеточием :
// Все последующие символы должны быть буквами, цифрами, подчеркиваниями _, двоеточиями :, дефисами - или точками .
bool XmlValidator::isValidXmlName(const QString &name)
{
    if (name.isEmpty()) return false;
    
    // First character
    QChar firstChar = name[0];
    if (!firstChar.isLetter() && firstChar != '_' && firstChar != ':') {
        return false;
    }
    
    // Other characters
    for (int i = 1; i < name.length(); ++i) {
        QChar ch = name[i];
        if (!ch.isLetter() && !ch.isDigit() && ch != '_' && ch != ':' && 
            ch != '-' && ch != '.') {
            return false;
        }
    }
    
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Диапазон разрешенных символов (XML 1.0):
// Char  ::=	#x9 | #xA | #xD | [#x20-#xD7FF] | [#xE000-#xFFFD] | [#x10000-#x10FFFF]	/* любой символ Юникода, за исключением суррогатных блоков FFFE и FFFF. */
// 1.Проверка диапазона 0x0000 - 0x0008:
//  0x0000 = NULL (NUL)
//  0x0001 = Start of Heading (SOH)
//  0x0002 = Start of Text (STX)
//  0x0003 = End of Text (ETX)
//  0x0004 = End of Transmission (EOT)
//  0x0005 = Enquiry (ENQ)
//  0x0006 = Acknowledge (ACK)
//  0x0007 = Bell (BEL)
//  0x0008 = Backspace (BS)
// 2.Проверка диапазона 0x000B - 0x000C:
//  0x000B = Vertical Tab (VT)
//  0x000C = Form Feed (FF)
// 3.Проверка диапазона 0x000E - 0x001F:
//  0x000E = Shift Out (SO)
//  0x000F = Shift In (SI)
//  0x0010 = Data Link Escape (DLE)
//  0x0011 = Device Control 1 (DC1)
//  0x0012 = Device Control 2 (DC2)
//  0x0013 = Device Control 3 (DC3)
//  0x0014 = Device Control 4 (DC4)
//  0x0015 = Negative Acknowledge (NAK)
//  0x0016 = Synchronous Idle (SYN)
//  0x0017 = End of Transmission Block (ETB)
//  0x0018 = Cancel (CAN)
//  0x0019 = End of Medium (EM)
//  0x001A = Substitute (SUB)
//  0x001B = Escape (ESC)
//  0x001C = File Separator (FS)
//  0x001D = Group Separator (GS)
//  0x001E = Record Separator (RS)
//  0x001F = Unit Separator (US)
//
// Разрешённые символы:
//  0x0009 (табуляция)
//  0x000A (перевод строки)
//  0x000D (возврат каретки)
//  0x0020 и выше (все остальные)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Qt5
bool XmlValidator::isValidXmlCharacter(QChar ch)
{
    uint unicode = ch.unicode(); // Используем uint для поддержки больших значений

    // Valid XML character ranges according to XML 1.0 specification
    if (unicode == 0x09 || unicode == 0x0A || unicode == 0x0D) {
        return true; // Allowed control characters: tab, line feed, carriage return
    }

    if (unicode >= 0x20 && unicode <= 0xD7FF) {
        return true; // Basic multilingual plane (except surrogate pairs)
    }

    if (unicode >= 0xE000 && unicode <= 0xFFFD) {
        return true; // Private use area and special characters
    }

    // Для Qt5 проверим, что значение в допустимом диапазоне
    // Qt5 может иметь ограничения на максимальное значение QChar
    if (sizeof(ushort) < sizeof(uint)) {
        // Если ushort меньше uint, проверяем диапазон
        if (unicode >= 0x10000 && unicode <= 0x10FFFF && unicode <= 0xFFFF) {
            return true; // Extended character planes (if supported)
        }
    } else {
        if (unicode >= 0x10000 && unicode <= 0x10FFFF) {
            return true; // Extended character planes
        }
    }

    return false;
}

int XmlValidator::getLineFromPosition(const QString &content, int position)
{
    int line = 1;
    for (int i = 0; i < position && i < content.length(); ++i) {
        if (content[i] == '\n') {
            line++;
        }
    }
    return line;
}

int XmlValidator::getColumnFromPosition(const QString &content, int position)
{
    int column = 1;
    for (int i = position - 1; i >= 0 && content[i] != '\n'; --i) {
        column++;
    }
    return column;
}

bool XmlValidator::isReservedXmlName(const QString &name)
{
    QStringList reservedNames = {"xml", "xmlns"};
    return reservedNames.contains(name.toLower());
}

QString XmlValidator::getDetailedErrorReport(const ValidationResult &result)
{
    if (result.errors.isEmpty()) {
        return tr("Ошибок не обнаружено. XML-документ корректен");
    }
    
    QString report = QString(tr("Отчет о проверке XML\n"));
    report += QString(tr("=====================\n"));
    report += QString(tr("Ошибки: %1, Предупреждения: %2\n\n")).arg(result.errorCount).arg(result.warningCount);
    
    for (const auto &error : result.errors) {
        QString lineInfo = (error.line > 0) ? QString(tr("Строка %1, Столбец %2: ")).arg(error.line).arg(error.column) : tr("");
        report += QString(tr("%1%2 (%3 - %4)\n")).arg(lineInfo, error.message, error.errorType, error.severity);
        if (!error.details.isEmpty()) {
            report += QString(tr("  Подробности: %1\n")).arg(error.details);
        }
        report += tr("\n");
    }
    
    return report;
}

QString XmlValidator::getValidationSummary(const ValidationResult &result)
{
    return result.summary;
}

bool XmlValidator::hasErrors(const ValidationResult &result)
{
    return result.errorCount > 0;
}

bool XmlValidator::hasWarnings(const ValidationResult &result)
{
    return result.warningCount > 0;
}

