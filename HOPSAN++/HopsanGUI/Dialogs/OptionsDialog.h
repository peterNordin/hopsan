//!
//! @file   OptionsDialog.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-XX-XX
//!
//! @brief Contains a class for the options dialog
//!
//$Id: OptionsDialog.h 1195 2010-04-01 09:25:58Z robbr48 $

#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QGridLayout>
#include <QToolButton>
#include <QComboBox>

class MainWindow;

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    OptionsDialog(MainWindow *parent = 0);

    //MainWindow *mpParentMainWindow;

public slots:
    void updateValues();
    void colorDialog();
    void show();

private slots:
    void addValueUnit();
    void addPressureUnit();
    void addFlowUnit();
    void addForceUnit();
    void addPositionUnit();
    void addVelocityUnit();
    void addTorqueUnit();
    void addAngleUnit();
    void addAngularVelocityUnit();
    void addCustomUnitDialog(QString physicalQuantity);
    void addCustomUnit();
    void updateCustomUnits();

signals:
    void paletteChanged();

private:
    QColor mPickedBackgroundColor;

    QCheckBox *mpNativeStyleSheetCheckBox;
    QCheckBox *mpShowWelcomeDialogCheckBox;
    QCheckBox *mpInvertWheelCheckBox;
    QCheckBox *mpAntiAliasingCheckBox;
    QCheckBox *mpSnappingCheckBox;
    QLabel *mpBackgroundColorLabel;
    QToolButton *mpBackgroundColorButton;
    QGroupBox *mpInterfaceGroupBox;
    QGridLayout *mpInterfaceLayout;

    QCheckBox *mpUseMulticoreCheckBox;
    QCheckBox *mpEnableProgressBarCheckBox;
    QLabel *mpThreadsLabel;
    QSpinBox *mpThreadsSpinBox;
    QLabel *mpThreadsWarningLabel;
    QLabel *mpProgressBarLabel;
    QSpinBox *mpProgressBarSpinBox;
    QGroupBox *mpSimulationGroupBox;
    QGridLayout *mpSimulationLayout;

    QLabel *mpValueUnitLabel;
    QComboBox *mpValueUnitComboBox;
    QPushButton *mpAddValueUnitButton;
    QLabel *mpPressureUnitLabel;
    QComboBox *mpPressureUnitComboBox;
    QPushButton *mpAddPressureUnitButton;
    QLabel *mpFlowUnitLabel;
    QComboBox *mpFlowUnitComboBox;
    QPushButton *mpAddFlowUnitButton;
    QLabel *mpForceUnitLabel;
    QComboBox *mpForceUnitComboBox;
    QPushButton *mpAddForceUnitButton;
    QLabel *mpPositionUnitLabel;
    QComboBox *mpPositionUnitComboBox;
    QPushButton *mpAddPositionUnitButton;
    QLabel *mpVelocityUnitLabel;
    QComboBox *mpVelocityUnitComboBox;
    QPushButton *mpAddVelocityUnitButton;
    QLabel *mpTorqueUnitLabel;
    QComboBox *mpTorqueUnitComboBox;
    QPushButton *mpAddTorqueUnitButton;
    QLabel *mpAngleUnitLabel;
    QComboBox *mpAngleUnitComboBox;
    QPushButton *mpAddAngleUnitButton;
    QLabel *mpAngularVelocityUnitLabel;
    QComboBox *mpAngularVelocityUnitComboBox;
    QPushButton *mpAddAngularVelocityUnitButton;
    QGroupBox *mpPlottingGroupBox;
    QGridLayout *mpPlottingLayout;

    QPushButton *mpCancelButton;
    QPushButton *mpApplyButton;
    QPushButton *mpOkButton;
    QDialogButtonBox *mpButtonBox;

    QWidget *mpCentralwidget;

    QDialog *mpAddUnitDialog;
    QLabel *mpNameLabel;
    QLineEdit *mpUnitNameBox;
    QLabel *mpScaleLabel;
    QLineEdit *mpScaleBox;
    QPushButton *mpDoneInUnitDialogButton;
    QPushButton *mpCancelInUnitDialogButton;
    QString mPhysicalQuantityToModify;
};

#endif // OPTIONSDIALOG_H
