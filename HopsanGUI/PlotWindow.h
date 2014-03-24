/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   PlotWindow.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the PlotWindow class
//!
//$Id$

#ifndef PlotWindow_H
#define PlotWindow_H

#include <QObject>
#include <QDockWidget>
#include <QStackedWidget>
#include <QToolBar>
#include <QToolButton>
#include <QString>
#include <QMainWindow>

#include "PlotTab.h"
#include "qwt_plot.h"
#include "LogVariable.h"

// Forward Declaration
class PlotCurve;
class PlotWindow;
class HelpPopUpWidget;

//! @brief Tab widget for plots in plot window
class PlotTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    PlotTabWidget(PlotWindow *pParentPlotWindow);
    PlotTab *getCurrentTab();
    PlotTab *getTab(const int i);
    void closeAllTabs();

public slots:
    void closePlotTab(int index);
};

class PlotWindow : public QMainWindow
{
    Q_OBJECT
    friend class PlotTab;

public:
    PlotWindow(const QString name, QWidget *parent);
    ~PlotWindow();

    QString getName() const;

    PlotTab *addPlotTab(const QString &rName, PlotTabTypeT type=XYPlotType);
    PlotTab *getCurrentPlotTab();
    PlotTabWidget *getPlotTabWidget(); //!< @todo should this realy be needed

    void createBodePlot(SharedVectorVariableT var1, SharedVectorVariableT var2, int Fmax);

    void showHelpPopupMessage(const QString &rMessage);

    PlotCurve* addPlotCurve(HopsanVariable data, const QwtPlot::Axis axisY=QwtPlot::yLeft, QColor desiredColor=QColor());
    PlotCurve* addPlotCurve(HopsanVariable xdata, HopsanVariable ydata, const QwtPlot::Axis axisY=QwtPlot::yLeft, QColor desiredColor=QColor());

signals:
    void windowClosed(PlotWindow *pWindow);

public slots:
    PlotTab *addPlotTab();
    void createPlotWindowFromTab();

    //! @todo these two should not be used
    void setCustomXVector(QVector<double> xarray, const VariableDescription &rVarDesc);
    void setCustomXVector(SharedVectorVariableT pData);

    void saveToXml();
    void loadFromXml();

    void importPlo();
    void importCsv();

    void updatePalette();
    void hidePlotCurveControls();
    void setLegendsVisible(bool value);

    void closeIfEmpty();
    void closeAllTabs();

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void closeEvent(QCloseEvent *event);

protected slots:
    void changedTab();
    void showToolBarHelpPopup();

private:
    void refreshWindowTitle();
    void setModelPaths(const QStringList &rPaths);

    QString mName;
    QStringList mModelPaths;
    QPointF dragStartPosition;

    PlotTabWidget *mpPlotTabWidget;
    QDockWidget *mpPlotCurveControlsDock;
    QStackedWidget *mpPlotCurveControlsStack;
    HelpPopUpWidget *mpHelpPopup;

    QToolBar *mpToolBar;
    QAction *mpNewPlotButton;
    QAction *mpArrowButton;
    QAction *mpLegendButton;
    QAction *mpZoomButton;
    QAction *mpOriginalZoomButton;
    QAction *mpPanButton;
    QAction *mpSaveButton;
    QToolButton *mpExportButton;
    QToolButton *mpImportButton;
    QAction *mpLoadFromXmlButton;
    QAction *mpGridButton;
    QAction *mpBackgroundColorButton;
    QAction *mpNewWindowFromTabButton;
    QAction *mpResetXVectorButton;
    QAction *mpAllGenerationsDown;
    QAction *mpAllGenerationsUp;
    QAction *mpBodePlotButton;
    QMenu *mpImportMenu;
    QMenu *mpExportMenu;
    QAction *mpExportToXmlAction;
    QAction *mpImportPloAction;
    QAction *mpImportCsvAction;
    QAction *mpExportToCsvAction;
    QAction *mpExportToHvcAction;
    QAction *mpExportToMatlabAction;
    QAction *mpExportToGnuplotAction;
    QAction *mpExportToOldHopAction;
    QAction *mpExportToGraphicsAction;
    QAction *mpLocktheAxis;
    QAction *mpToggleAxisLockButton;
    QAction *mpOpentimeScaleDialog;
};

#endif // PLOTWINDOW_H