#include <QtGui>
//#include <QObject>
#include "UndoStack.h"
#include "GUIObject.h"
#include "HopsanCore.h"
#include <sstream>
#include <QDebug>
#include "AppearanceData.h"
#include "mainwindow.h"
#include "LibraryWidget.h"
#include <assert.h>

class AppearanceData;

//! Constructor
UndoStack::UndoStack(GraphicsView *parentView)
{
    mpParentView = parentView;
    mCurrentStackPosition = 0;
    mStack.append(QStringList());
}


//! Destructor
UndoStack::~UndoStack()
{
    //Not implemented yet
}


//! Clears all contents in the stack
void UndoStack::clear()
{
    mStack.clear();
    mCurrentStackPosition = 0;
    mStack.append(QStringList());
}


//! Adds a new post to the stack
void UndoStack::newPost()
{
    if(!mStack[mCurrentStackPosition].empty())
    {
        ++mCurrentStackPosition;
        mStack.append(QStringList());
    }
}


//! Will undo the changes registered in the last stack position, and switch stack pointer one step back
void UndoStack::undoOneStep()
{
    while(mStack[mCurrentStackPosition].empty() and mCurrentStackPosition != 0)
    {
        mStack.pop_back();
        --mCurrentStackPosition;
    }
    if(!(mCurrentStackPosition == 0 and mStack[0].empty()))
    {
        string undoWord;
        for(int i = 0; i != mStack[mCurrentStackPosition].size(); ++i)
        {
            stringstream undoStream(mStack[mCurrentStackPosition][i].toStdString().c_str());
            if ( undoStream >> undoWord )
            {
                if( undoWord == "COMPONENT" )
                {
                    string componentType;
                    undoStream >> componentType;
                    string componentName;
                    undoStream >> componentName;
                    string tempString;
                    undoStream >> tempString;
                    int posX = QString(tempString.c_str()).toInt();
                    undoStream >> tempString;
                    int posY = QString(tempString.c_str()).toInt();
                    undoStream >> tempString;
                    qreal rotation = QString(tempString.c_str()).toDouble();
                    undoStream >> tempString;
                    int nameTextPos = QString(tempString.c_str()).toInt();

                    //! @todo This component need to be loaded in the library, or maybe we should auto load it if possible if missing (probably dfficult)
                    AppearanceData appearanceData = *mpParentView->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpLibrary->getAppearanceData(QString(componentType.c_str()));
                    mpParentView->addGUIObject(QString(componentType.c_str()), appearanceData, QPoint(posX, posY), 0, QString(componentName.c_str()));
                    mpParentView->getGUIObject(QString(componentName.c_str()))->setNameTextPos(nameTextPos);
                    while(mpParentView->getGUIObject(QString(componentName.c_str()))->rotation() != rotation)
                    {
                        mpParentView->getGUIObject(QString(componentName.c_str()))->rotate();
                    }
                    mStack[mCurrentStackPosition].pop_front();      //addGUIObject will register the creation in the stack, so it must be removed to avoid an endless loop
                }
                else if ( undoWord == "CONNECT" )
                {
                    string startComponentName;
                    undoStream >> startComponentName;
                    string tempString;
                    undoStream >> tempString;
                    int startPortNumber = QString(tempString.c_str()).toInt();
                    string endComponentName;
                    undoStream >> endComponentName;
                    undoStream >> tempString;
                    int endPortNumber = QString(tempString.c_str()).toInt();

                    GUIPort *startPort = mpParentView->getGUIObject(QString(startComponentName.c_str()))->getPort(startPortNumber);
                    GUIPort *endPort = mpParentView->getGUIObject(QString(endComponentName.c_str()))->getPort(endPortNumber);

                    std::vector<QPointF> tempPointVector;
                    qreal tempX, tempY;
                    while(undoStream.good())
                    {
                        undoStream >> tempString;
                        tempX = QString(tempString.c_str()).toDouble();
                        undoStream >> tempString;
                        tempY = QString(tempString.c_str()).toDouble();
                        tempPointVector.push_back(QPointF(tempX, tempY));
                    }

                    //! @todo: Store useIso bool in model file and pick the correct line styles when loading
                    GUIConnector *pTempConnector;

                    QString type, style;
                    if((startPort->mpCorePort->getNodeType() == "NodeHydraulic") | (startPort->mpCorePort->getNodeType() == "NodeMechanic"))
                        type = "Power";
                    else if(startPort->mpCorePort->getNodeType() == "NodeSignal")
                        type = "Signal";
                    if(mpParentView->mpParentProjectTab->useIsoGraphics)
                        style = "Iso";
                    else
                        style = "User";
                    pTempConnector = new GUIConnector(startPort, endPort, tempPointVector, mpParentView->getPen("Primary", type, style),
                                                      mpParentView->getPen("Active", type, style), mpParentView->getPen("Hover", type, style), mpParentView);

                    mpParentView->scene()->addItem(pTempConnector);

                    //Hide connected ports
                    startPort->hide();
                    endPort->hide();

                    GUIObject::connect(startPort->getGuiObject(),SIGNAL(componentDeleted()),pTempConnector,SLOT(deleteMe()));
                    GUIObject::connect(endPort->getGuiObject(),SIGNAL(componentDeleted()),pTempConnector,SLOT(deleteMe()));

                    std::stringstream tempStream;
                    tempStream << startPort->getGuiObject()->getName().toStdString() << " " << startPort->getPortNumber() << " " <<
                                  endPort->getGuiObject()->getName().toStdString() << " " << endPort->getPortNumber();
                    mpParentView->mConnectorVector.append(pTempConnector);
                    bool success = mpParentView->getModelPointer()->connect(startPort->mpCorePort, endPort->mpCorePort);
                    if (!success)
                    {
                        qDebug() << "Unsuccessful connection try" << endl;
                        assert(false);
                    }
                }
                else if(undoWord == "DELETEOBJECT")
                {
                    string itemName;
                    undoStream >> itemName;
                    GUIObject* item = mpParentView->mGUIObjectMap.find(QString(itemName.c_str())).value();
                    mpParentView->mGUIObjectMap.erase(mpParentView->mGUIObjectMap.find(QString(itemName.c_str())));
                    item->deleteInHopsanCore();
                    mpParentView->scene()->removeItem(item);
                    delete(item);
                    mpParentView->setBackgroundBrush(mpParentView->mBackgroundColor);
                }
                else if(undoWord == "DISCONNECT")
                {
                    string startComponentName;
                    string startPortNumber;
                    string endComponentName;
                    string endPortNumber;
                    undoStream >> startComponentName;
                    undoStream >> startPortNumber;
                    undoStream >> endComponentName;
                    undoStream >> endPortNumber;

                        // Lookup the pointer to the connector to remove from the connector vector
                    GUIConnector *item;
                    for(int i = 0; i != mpParentView->mConnectorVector.size(); ++i)
                    {
                        if((mpParentView->mConnectorVector[i]->getStartPort()->getGuiObject()->getName() == QString(startComponentName.c_str())) and
                           (mpParentView->mConnectorVector[i]->getStartPort()->getPortNumber() == QString(startPortNumber.c_str()).toInt()) and
                           (mpParentView->mConnectorVector[i]->getEndPort()->getGuiObject()->getName() == QString(endComponentName.c_str())) and
                           (mpParentView->mConnectorVector[i]->getEndPort()->getPortNumber() == QString(endPortNumber.c_str()).toInt()))
                        {
                            item = mpParentView->mConnectorVector[i];
                        }
                        else if((mpParentView->mConnectorVector[i]->getStartPort()->getGuiObject()->getName() == QString(endComponentName.c_str())) and
                                (mpParentView->mConnectorVector[i]->getStartPort()->getPortNumber() == QString(endPortNumber.c_str()).toInt()) and
                                (mpParentView->mConnectorVector[i]->getEndPort()->getGuiObject()->getName() == QString(startComponentName.c_str())) and
                                (mpParentView->mConnectorVector[i]->getEndPort()->getPortNumber() == QString(startPortNumber.c_str()).toInt()))
                        {
                            item = mpParentView->mConnectorVector[i];
                        }
                    }
                    mpParentView->removeConnector(item);
                }
                else if(undoWord == "RENAMEOBJECT")
                {
                    string oldName;
                    string newName;
                    undoStream >> oldName;
                    undoStream >> newName;

                    GUIObject* obj_ptr = mpParentView->mGUIObjectMap.find(QString(newName.c_str())).value();
                    mpParentView->mGUIObjectMap.erase(mpParentView->mGUIObjectMap.find(QString(newName.c_str())));
                    obj_ptr->setName(QString(oldName.c_str()), true);
                    mpParentView->mGUIObjectMap.insert(obj_ptr->getName(), obj_ptr);
                }
                else if(undoWord == "MODIFIEDCONNECTOR")
                {
                    string tempString;
                    qreal oldX;
                    qreal oldY;
                    string startComponentName;
                    string startPortNumber;
                    string endComponentName;
                    string endPortNumber;
                    int lineNumber;

                    undoStream >> tempString;
                    oldX = QString(tempString.c_str()).toFloat();
                    undoStream >> tempString;
                    oldY = QString(tempString.c_str()).toFloat();
                    undoStream >> startComponentName;
                    undoStream >> startPortNumber;
                    undoStream >> endComponentName;
                    undoStream >> endPortNumber;
                    undoStream >> tempString;
                    lineNumber = QString(tempString.c_str()).toInt();

                        //Fetch the pointer to the connector
                    GUIConnector *item;
                    for(int i = 0; i != mpParentView->mConnectorVector.size(); ++i)
                    {
                        if((mpParentView->mConnectorVector[i]->getStartPort()->getGuiObject()->getName() == QString(startComponentName.c_str())) and
                           (mpParentView->mConnectorVector[i]->getStartPort()->getPortNumber() == QString(startPortNumber.c_str()).toInt()) and
                           (mpParentView->mConnectorVector[i]->getEndPort()->getGuiObject()->getName() == QString(endComponentName.c_str())) and
                           (mpParentView->mConnectorVector[i]->getEndPort()->getPortNumber() == QString(endPortNumber.c_str()).toInt()))
                        {
                            item = mpParentView->mConnectorVector[i];
                        }
                        else if((mpParentView->mConnectorVector[i]->getStartPort()->getGuiObject()->getName() == QString(endComponentName.c_str())) and
                                (mpParentView->mConnectorVector[i]->getStartPort()->getPortNumber() == QString(endPortNumber.c_str()).toInt()) and
                                (mpParentView->mConnectorVector[i]->getEndPort()->getGuiObject()->getName() == QString(startComponentName.c_str())) and
                                (mpParentView->mConnectorVector[i]->getEndPort()->getPortNumber() == QString(startPortNumber.c_str()).toInt()))
                        {
                            item = mpParentView->mConnectorVector[i];
                        }
                    }

                    item->getLine(lineNumber)->setPos(QPointF(oldX, oldY));
                    item->updateLine(lineNumber);
                }
                else if(undoWord == "MOVEDOBJECT")
                {
                    string tempString;
                    qreal oldX;
                    qreal oldY;
                    string objectName;
                    undoStream >> tempString;
                    oldX = QString(tempString.c_str()).toFloat();
                    undoStream >> tempString;
                    oldY = QString(tempString.c_str()).toFloat();
                    undoStream >> objectName;

                    mpParentView->getGUIObject(QString(objectName.c_str()))->setPos(QPointF(oldX, oldY));
                }
            }
        }
        mStack[mCurrentStackPosition] = QStringList();      //Empty curren stack position
        mpParentView->setBackgroundBrush(mpParentView->mBackgroundColor);
    }
}


//! Will redo the previously undone changes if they exist, and re-add the undo command to the undo stack.
void UndoStack::redoOneStep()
{
    //Not implemented yet
}


//! Register function for component
//! @param item is a pointer to the component about to be deleted.
void UndoStack::registerDeletedObject(GUIObject *item)
{
    QPointF pos = item->mapToScene(item->boundingRect().center());
    std::stringstream tempStringStream;
    //std::string tempString;
    tempStringStream << "COMPONENT " << item->getTypeName().toStdString() << " " << item->getName().toStdString()
            << " " << pos.x() << " " << pos.y() << " " << item->rotation() << " " << item->getNameTextPos();
    mStack[mCurrentStackPosition].insert(0,QString(tempStringStream.str().c_str()));

    //! @todo ugly quickhack for now dont save parameters for systemport or group
    //! @todo Group typename probably not correct
    //! @todo maybe the save functin should be part of every object (so it can write its own text)
    if ( (item->getTypeName() != "SystemPort") && (item->getTypeName() != "Group") )
    {
        Component *mpCoreComponent = item->getHopsanCoreComponentPtr();
        vector<CompParameter> paramVector = mpCoreComponent->getParameterVector();
        std::vector<CompParameter>::iterator itp;
        for ( itp=paramVector.begin() ; itp !=paramVector.end(); ++itp )
        {
            tempStringStream.str("");
            tempStringStream << "PARAMETER " << item->getName().toStdString() << " " << itp->getName().c_str() << " " << itp->getValue() << "\n";
            mStack[mCurrentStackPosition].insert(0,QString(tempStringStream.str().c_str()));
        }
    }
    qDebug() << "Adding " << QString(tempStringStream.str().c_str());
}


//! Register function for connectors
//! @param item is a pointer to the connector about to be deleted.
void UndoStack::registerDeletedConnector(GUIConnector *item)
{
    std::stringstream tempStringStream;

    int i;
    for(i = 0; i != mpParentView->mConnectorVector.size(); ++i)
    {
        if(mpParentView->mConnectorVector[i] == item)
        {
            break;
        }
    }

    QString startPortNumberString;
    QString endPortNumberString;
    startPortNumberString.setNum(mpParentView->mConnectorVector[i]->getStartPort()->getPortNumber());
    endPortNumberString.setNum(mpParentView->mConnectorVector[i]->getEndPort()->getPortNumber());
    tempStringStream << "CONNECT " << QString(mpParentView->mConnectorVector[i]->getStartPort()->getGuiObject()->getName() + " " +
                                              startPortNumberString + " " +
                                              mpParentView->mConnectorVector[i]->getEndPort()->getGuiObject()->getName() + " " +
                                              endPortNumberString).toStdString();
    for(size_t j = 0; j != mpParentView->mConnectorVector[i]->getPointsVector().size(); ++j)
    {
        tempStringStream << " " << mpParentView->mConnectorVector[i]->getPointsVector()[j].x() << " " << mpParentView->mConnectorVector[i]->getPointsVector()[j].y();
    }
    mStack[mCurrentStackPosition].insert(0,QString(tempStringStream.str().c_str()));
    qDebug() << "Adding " << QString(tempStringStream.str().c_str());
}


//! Register function for added objects.
//! @param itemName is the name of the added object
void UndoStack::registerAddedObject(QString itemName)
{
    mStack[mCurrentStackPosition].insert(0,QString("DELETEOBJECT " + itemName));
    qDebug() << "Adding " << QString("DELETEOBJECT " + itemName);
}


//! Register function for added connectors.
//! @param item is a pointer to the added connector.
void UndoStack::registerAddedConnector(GUIConnector *item)
{
    std::stringstream tempStringStream;
    int i;
    for(i = 0; i != mpParentView->mConnectorVector.size(); ++i)
    {
        if(mpParentView->mConnectorVector[i] == item)
        {
            break;
        }
    }
    QString startPortNumberString;
    QString endPortNumberString;
    startPortNumberString.setNum(mpParentView->mConnectorVector[i]->getStartPort()->getPortNumber());
    endPortNumberString.setNum(mpParentView->mConnectorVector[i]->getEndPort()->getPortNumber());
    tempStringStream << "DISCONNECT " << QString(mpParentView->mConnectorVector[i]->getStartPort()->getGuiObject()->getName() + " " +
                                                 startPortNumberString + " " +
                                                 mpParentView->mConnectorVector[i]->getEndPort()->getGuiObject()->getName() + " " +
                                                 endPortNumberString).toStdString();
    mStack[mCurrentStackPosition].insert(0,QString(tempStringStream.str().c_str()));
    qDebug() << "Adding " << QString(tempStringStream.str().c_str());
}


//! Registser function for renaming an object.
//! @param oldName is a string with the old name.
//! @param newName is a string with the new name.
void UndoStack::registerRenameObject(QString oldName, QString newName)
{
    //! @todo Spaces in names won't work with the streaming. Replace them with something here and then replace back when undoing.
    mStack[mCurrentStackPosition].insert(0,QString("RENAMEOBJECT " + oldName + " " + newName));
    qDebug() << "Adding " << QString("RENAMEOBJECT " + oldName + " " + newName);
}


//! Register function for moving a line in a connector.
//! @param oldPos is the position before the line was moved.
//! @param item is a pointer to the connector.
//! @param lineNumber is the number of the line that was moved.
void UndoStack::registerModifiedConnector(QPointF oldPos, GUIConnector *item, int lineNumber)
{
    int i;
    for(i = 0; i != mpParentView->mConnectorVector.size(); ++i)
    {
        if(mpParentView->mConnectorVector[i] == item)
        {
            break;
        }
    }

    QString oldXString;
    QString oldYString;
    //QString newXString;
    //QString newYString;
    QString lineNumberString;
    QString startPortNumberString;
    QString endPortNumberString;
    oldXString.setNum(oldPos.x());
    oldYString.setNum(oldPos.y());
    //newXString.setNum(newPos.x());
    //newYString.setNum(newPos.y());
    lineNumberString.setNum(lineNumber);
    startPortNumberString.setNum(mpParentView->mConnectorVector[i]->getStartPort()->getPortNumber());
    endPortNumberString.setNum(mpParentView->mConnectorVector[i]->getEndPort()->getPortNumber());

    mStack[mCurrentStackPosition].insert(0,QString("MODIFIEDCONNECTOR " + oldXString + " " + oldYString + " " +
                                                   mpParentView->mConnectorVector[i]->getStartPort()->getGuiObject()->getName() + " " + startPortNumberString + " " +
                                                   mpParentView->mConnectorVector[i]->getEndPort()->getGuiObject()->getName() + " " + endPortNumberString + " " +
                                                   lineNumberString));
    qDebug() << "Adding " << QString("MODIFIEDCONNECTOR " + oldXString + " " + oldYString + " " +
                                     mpParentView->mConnectorVector[i]->getStartPort()->getGuiObject()->getName() + " " + startPortNumberString + " " +
                                     mpParentView->mConnectorVector[i]->getEndPort()->getGuiObject()->getName() + " " + endPortNumberString + " " +
                                     lineNumberString);
}


//! Register function for moving an object.
//! @param oldPos is the position of the object before it was moved.
//! @param objectName is the name of the object.
void UndoStack::registerMovedObject(QPointF oldPos, QString objectName)
{
    QString oldXString;
    QString oldYString;
    oldXString.setNum(oldPos.x());
    oldYString.setNum(oldPos.y());
    mStack[mCurrentStackPosition].insert(0,QString("MOVEDOBJECT " + oldXString + " " + oldYString + " " + objectName));
    qDebug() << "Adding " << QString("MOVEDOBJECT " + oldXString + " " + oldYString + " " + objectName);
}
