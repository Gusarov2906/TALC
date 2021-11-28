#include <iostream>
#include <QtCore>
#include <stdlib.h>
#include <windows.h> 
#include "ConsoleColors.h"
#include <stack>
#include "Element.h"
#include "Block.h"
#include "Column.h"
#include "Row.h"

class XmlStreamLint
{
public:
    Q_DECLARE_TR_FUNCTIONS(XmlStreamLint)
};

void addNodeRoot(QXmlStreamReader& reader, std::stack<QString>& nestingStack, std::map<int, Element*>& levelParents, Element*& nodeRoot)
{
    nestingStack.push(reader.tokenString() + "_" + reader.name());
    if (reader.name() == "block")
    {
        if (reader.attributes().size() > 0)
        {
            nodeRoot = new Block(0, 1);
            for (QXmlStreamAttribute atribute : reader.attributes())
            {
                QRegExp re("\\d+");
                if (re.exactMatch(atribute.value().toString()))
                {
                    //qDebug() << atribute.name();
                    if (atribute.name() == "rows")
                    {
                        static_cast<Block*>(nodeRoot)->setRowsCount(atribute.value().toInt());
                    }
                    else if (atribute.name() == "columns")
                    {
                        static_cast<Block*>(nodeRoot)->setColumnsCount(atribute.value().toInt());
                    }
                    else
                    {
                        throw (QString("No width argument!"));
                    }
                }
                else
                {
                    throw (QString("No width argument!"));
                }
            }
            std::pair<int, Element*> toAdd(0, nodeRoot);
            levelParents.insert(toAdd);
            //qDebug() << static_cast<Block*>(nodeRoot)->getColumnsCount() << static_cast<Block*>(nodeRoot)->getRowsCount();
        }
    }
}

void addElement(ElementType elementType, QXmlStreamReader& reader, std::stack<QString>& nestingStack,
    std::map<int, Element*>& levelParents, Element* parent)
{
    nestingStack.push(reader.tokenString() + "_" + reader.name());

    VAlignType vAlign = VAlignType::top;
    HAlignType hAlign = HAlignType::left;
    uint8_t textColor = 15;
    uint8_t bgColor = 0;
    int width = -1;
    int height = -1;
    ElementType parentType;
    QRegExp re("\\d+");
    bool isNeededWidth = false;
    bool isNeededHeight = false;
    if (elementType == ElementType::COLUMN)
    {
        isNeededWidth = true;
    }
    else if (elementType == ElementType::ROW)
    {
        isNeededHeight = true;
    }
    bool foundWidth = false;
    bool foundHeight = false;

    if (parent)
    {
        parentType = parent->getType();
        if (parentType == ElementType::BLOCK)
        {
            if (elementType == ElementType::COLUMN)
            {
                if (static_cast<Block*>(parent)->getColumnsCount() == (static_cast<Block*>(parent)->getColumnsCounter()+1))
                {
                    isNeededWidth = false;
                    width = parent->getWidth() - parent->getPosX();
                }
            }
            else if (elementType == ElementType::ROW)
            {
                if (static_cast<Block*>(parent)->getRowsCount() == (static_cast<Block*>(parent)->getRowsCounter()+1))
                {
                    isNeededHeight = false;
                    height = parent->getHeight() - parent->getPosY();
                }
            }
        }
        else if (parentType == ElementType::ROW || parentType == ElementType::COLUMN)
        {
            if (parent->getParent()->getType() == ElementType::BLOCK)
            {
                if (elementType == ElementType::COLUMN)
                {
                    if (static_cast<Block*>(parent->getParent())->getColumnsCount() == (static_cast<Block*>(parent)->getColumnsCounter()+1))
                    {
                        isNeededWidth = false;
                        width = parent->getParent()->getWidth() - parent->getParent()->getPosX();
                    }
                }
                else if (elementType == ElementType::ROW)
                {
                    if (static_cast<Block*>(parent->getParent())->getRowsCount() == (static_cast<Block*>(parent)->getRowsCounter()+1))
                    {
                        isNeededHeight = false;
                        height = parent->getParent()->getHeight() - parent->getParent()->getPosY();
                    }
                }
            }
        }
    }

    if (reader.attributes().size() > 0)
    {
        for (QXmlStreamAttribute atribute : reader.attributes())
        {
            if (atribute.name() == "valign")
            {
                QString val = atribute.value().toString();
                if (val == "top")
                {
                    vAlign = VAlignType::top;
                }
                else if (val == "center")
                {
                    vAlign = VAlignType::center;
                }
                else if (val == "bottom")
                {
                    vAlign = VAlignType::bottom;
                }
                else
                {
                    throw (QString("Bad argument!"));
                }
            }
            else if (atribute.name() == "halign")
            {
                QString val = atribute.value().toString();
                if (val == "left")
                {
                    hAlign = HAlignType::left;
                }
                else if (val == "center")
                {
                    hAlign = HAlignType::center;
                }
                else if (val == "right")
                {
                    hAlign = HAlignType::right;
                }
                else
                {
                    throw (QString("Bad argument!"));
                }
            }
            else if (re.exactMatch(atribute.value().toString()))
            {
                if (atribute.name() == "textcolor")
                {
                    textColor = atribute.value().toInt();
                }
                else if (atribute.name() == "bgcolor")
                {
                    bgColor = atribute.value().toInt();
                }
                else if (elementType == ElementType::COLUMN && atribute.name() == "width")
                {
                    foundWidth = true;
                    width = atribute.value().toInt();
                }
                else if (elementType == ElementType::ROW && atribute.name() == "height")
                {
                    foundHeight = true;
                    height = atribute.value().toInt();
                }
                else
                {
                    throw (QString("Bad argument!"));
                }
            }
            else
            {
                throw (QString("Bad argument!"));
            }
        }

        if (isNeededWidth)
        {
            if (!foundWidth && elementType == ElementType::COLUMN)
            {
                throw (QString("No width argument!"));
            }
        }
        else if (isNeededHeight)
        {
            if (!foundHeight && elementType == ElementType::ROW)
            {
                throw (QString("No height argument!"));
            }
        }
    }

    if (width == -1 && elementType == ElementType::COLUMN)
    {
        throw (QString("No width argument!"));
    }
    else if (height == -1 && elementType == ElementType::ROW)
    {
        throw (QString("No height argument!"));
    }

    Element* curElement = nullptr;
    if (parent != nullptr && parent->getType() == ElementType::BLOCK)
    {
        if (elementType == ElementType::COLUMN)
        {
            curElement = new Column(static_cast<Block*>(parent), width, vAlign, hAlign, textColor, bgColor, parent->getLevel() + 1);
        }
        else if (elementType == ElementType::ROW)
        {
            curElement = new Row(static_cast<Block*>(parent), height, vAlign, hAlign, textColor, bgColor, parent->getLevel() + 1);
        }
    }
    else if (parent != nullptr && parent->getType() == ElementType::ROW && elementType == ElementType::COLUMN)
    {
        curElement = new Column(static_cast<Row*>(parent), width, vAlign, hAlign, textColor, bgColor, parent->getLevel()+1);
    }
    else if (parent != nullptr && parent->getType() == ElementType::COLUMN && elementType == ElementType::ROW)
    {
        curElement = new Row(static_cast<Column*>(parent), height, vAlign, hAlign, textColor, bgColor, parent->getLevel() + 1);
    }
    if (curElement != nullptr)
    {
        parent->addElement(curElement);
        //std::pair<int, Element*> toAdd(curElement->getLevel(), curElement);
        levelParents[curElement->getLevel()] = curElement;
    }
}

int main(int argc, char *argv[])
{
    HANDLE  hConsole;
    //int k;
    std::stack<QString> nestingStack;
    std::map<int, Element*> levelParents;
    Element* nodeRoot;
    Element* curElemnt;
    int level = 0;

    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // color your text in Windows console mode
    // colors are 0=black 1=blue 2=green and so on to 15=white  
    // colorattribute = foreground + background * 16
    // to get red text on yellow use 4 + 14*16 = 228
    // light red on yellow would be 12 + 14*16 = 236


    int k = static_cast<int>(ConsoleColors::DARK_BLUE_BACKGROUND) + static_cast<int>(ConsoleColors::LIGHT_CYAN_TEXT);
    SetConsoleTextAttribute(hConsole, k);
    std::cout << k
        << " I want to be nice today!" << std::endl;
    SetConsoleTextAttribute(hConsole, 15);


    enum ExitCode
    {
        Success,
        ParseFailure,
        ArgumentError,
        WriteError,
        FileFailure
    };

    QCoreApplication app(argc, argv);

    QTextStream errorStream(stderr);
    QTextStream qout(stdout);
    QString inputFilePath("test.xml");
    QFile inputFile("test.xml");

    if (!QFile::exists(inputFilePath))
    {
        errorStream << XmlStreamLint::tr(
            "File %1 does not exist.\n").arg(inputFilePath);
        return FileFailure;

    }
    else if (!inputFile.open(QIODevice::ReadOnly)) {
        errorStream << XmlStreamLint::tr(
            "Failed to open file %1.\n").arg(inputFilePath);
        return FileFailure;
    }

    QXmlStreamReader reader(&inputFile);

    int counter = 1;

    bool isFirstElem = true;

    while (!reader.atEnd())
    {
        reader.readNext();

        if (reader.error())
        {
            errorStream << XmlStreamLint::tr(
                "Error: %1 in file %2 at line %3, column %4.\n").arg(
                    reader.errorString(), inputFilePath,
                    QString::number(reader.lineNumber()),
                    QString::number(reader.columnNumber()));
            return ParseFailure;
        }
        else
        {
            try
            {
                if (isFirstElem && reader.name() == "block")
                {
                    level = 0;
                    addNodeRoot(reader, nestingStack, levelParents, nodeRoot);
                    isFirstElem = false;
                }
                else if (reader.name() == "column" && reader.tokenString() == "StartElement")
                {
                    addElement(ElementType::COLUMN ,reader, nestingStack, levelParents, levelParents[level]);
                    level++;
                }
                else if (reader.name() == "row" && reader.tokenString() == "StartElement")
                {
                    addElement(ElementType::ROW, reader, nestingStack, levelParents, levelParents[level]);
                    level++;
                }
                else if (reader.tokenString() == "EndElement")
                {
                    nestingStack.pop();
                    level--;
                }
                else if (reader.name() == "block")
                {

                }
                else if(reader.name() == "")
                {
                    qDebug() << reader.text();
                }

            }
            catch (QString& error)
            {
                errorStream << XmlStreamLint::tr(
                    "Error: %1 in file %2 at line %3, column %4.\n").arg(
                        error, inputFilePath,
                        QString::number(reader.lineNumber()),
                        QString::number(reader.columnNumber()));
                return ParseFailure;
            }






            qout << reader.name() << reader.text() << "(    " << reader.tokenString() << "      )";
            //qout << reader.name() << reader.text() << "(" << reader.tokenString() << ")";
            //if (reader.attributes().size() > 0)
            //{
            //    qout << reader.attributes()[0].name() << reader.attributes()[0].value();
            //}
        }
    }
    return Success;
}

//for (k = 1; k < 255; k++)
//{
//    // pick the colorattribute k you want
//    SetConsoleTextAttribute(hConsole, k);
//    std::cout << k << " I want to be nice today!" << std::endl;
//}

//system("Color 1A");
//printf("\n");
//printf("\x1B[31mTexting\033[0m\t\t");
//printf("\x1B[32mTexting\033[0m\t\t");
//printf("\x1B[33mTexting\033[0m\t\t");
//printf("\x1B[34mTexting\033[0m\t\t");
//printf("\x1B[35mTexting\033[0m\n");

//printf("\x1B[36mTexting\033[0m\t\t");
//printf("\x1B[36mTexting\033[0m\t\t");
//printf("\x1B[36mTexting\033[0m\t\t");
//printf("\x1B[37mTexting\033[0m\t\t");
//printf("\x1B[93mTexting\033[0m\n");

//printf("\033[3;42;30mTexting\033[0m\t\t");
//printf("\033[3;43;30mTexting\033[0m\t\t");
//printf("\033[3;44;30mTexting\033[0m\t\t");
//printf("\033[3;104;30mTexting\033[0m\t\t");
//printf("\033[3;100;30mTexting\033[0m\n");

//printf("\033[3;47;35mTexting\t\t");
//printf("\033[2;47;35mTexting\033[0m\t\t");
//printf("\033[1;47;35mTexting\033[0m\t\t");
//printf("\t\t");
//printf("\n");