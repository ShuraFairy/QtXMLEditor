#pragma once

#include <QObject>
#include <QString>
#include <QStack>
#include <QList>
#include <QXmlStreamReader>

class XmlValidator : public QObject
{
    Q_OBJECT

public:
    enum ValidationType {
        WellFormedCheck             = 1,
        SyntaxCheck                 = 2,
        ElementNameCheck            = 4,
        AttributeCheck              = 8,
        CharacterDataCheck          = 16,
        NamespaceCheck              = 32,
        EntityCheck                 = 64,
        ProcessingInstructionCheck  = 128,
        CommentCheck                = 256,
        DeclarationCheck            = 512,
        FullValidation = WellFormedCheck | SyntaxCheck | ElementNameCheck | 
                        AttributeCheck | CharacterDataCheck | NamespaceCheck |
                        EntityCheck | ProcessingInstructionCheck | CommentCheck | DeclarationCheck
    };
    Q_DECLARE_FLAGS(ValidationTypes, ValidationType)
    Q_FLAG(ValidationTypes)

    struct ValidationError {
        QString message;
        int     line;
        int     column;
        QString errorType;
        QString details;
        QString severity; // "Error" or "Warning" // строгость
    };

    struct ValidationResult {
        bool    isValid;
        QList<ValidationError> errors;
        QString summary;
        int     errorCount;
        int     warningCount;
    };

    explicit XmlValidator(QObject *parent = nullptr);

    // Main validation methods
    ValidationResult validateXml(const QString &xmlContent, ValidationTypes types = FullValidation);
    
    // Specific validation checks
    bool isWellFormed(const QString &xmlContent, QString &errorMessage, int &errorLine, int &errorColumn);
    bool checkSyntax(const QString &xmlContent, QString &errorMessage, int &errorLine, int &errorColumn);
    bool checkElementNames(const QString &xmlContent, QString &errorMessage, int &errorLine, int &errorColumn);
    bool checkAttributeSyntax(const QString &xmlContent, QString &errorMessage, int &errorLine, int &errorColumn);
    bool checkCharacterData(const QString &xmlContent, QString &errorMessage, int &errorLine, int &errorColumn);
    bool checkNamespaceDeclarations(const QString &xmlContent, QString &errorMessage, int &errorLine, int &errorColumn);
    bool checkEntityReferences(const QString &xmlContent, QString &errorMessage, int &errorLine, int &errorColumn);
    bool checkProcessingInstructions(const QString &xmlContent, QString &errorMessage, int &errorLine, int &errorColumn);
    bool checkComments(const QString &xmlContent, QString &errorMessage, int &errorLine, int &errorColumn);
    bool checkXmlDeclaration(const QString &xmlContent, QString &errorMessage, int &errorLine, int &errorColumn);
    bool checkTagBalancing(const QString &xmlContent, QString &errorMessage, int &errorLine, int &errorColumn);
    bool checkProperNesting(const QString &xmlContent, QString &errorMessage, int &errorLine, int &errorColumn);
    
    // Utility methods
    QString getDetailedErrorReport(const ValidationResult &result);
    QString getValidationSummary(const ValidationResult &result);
    bool hasErrors(const ValidationResult &result);
    bool hasWarnings(const ValidationResult &result);

private:
    void addError(ValidationResult &result, const QString &message, int line = -1, int column = -1, 
                  const QString &type = "Error", const QString &details = "");
    void addWarning(ValidationResult &result, const QString &message, int line = -1, int column = -1, 
                    const QString &type = "Warning", const QString &details = "");
    
    // Helper methods
    bool isValidXmlName(const QString &name);
    bool isValidXmlCharacter(QChar ch);
    int  getLineFromPosition(const QString &content, int position);
    int  getColumnFromPosition(const QString &content, int position);
    bool isReservedXmlName(const QString &name);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(XmlValidator::ValidationTypes)


