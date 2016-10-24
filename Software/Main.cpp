//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#include <cstdlib> 
#pragma hdrstop

#include "Main.h"
#include "Modbus.h"
#include "Interface.h"
#include <Registry.hpp>

#define round(x) int((x) + 0.5)
//---------------------------------------------------------------------------
#define MODBUS_ADDR_TEMPERATURE 0
#define MODBUS_ADDR_HUMIDITY 1
#define MODBUS_ADDR_TIMER 2
#define MODBUS_ADDR_OUTPUT 3
#define MODBUS_ADDR_CH0_MAPPING 4
#define MODBUS_ADDR_CH1_MAPPING 5
#define MODBUS_ADDR_CH2_MAPPING 6

#define MODBUS_ADDR_TEMP_CTRL_TYPE 7
#define MODBUS_ADDR_TEMP_CTRL_INTERVAL 8
#define MODBUS_ADDR_TEMP_CTRL_HYSTERESIS 9
#define MODBUS_ADDR_TEMP_CTRL_KP 10
#define MODBUS_ADDR_TEMP_CTRL_KI 11
#define MODBUS_ADDR_TEMP_CTRL_REF 12

#define MODBUS_ADDR_HUM_CTRL_INTERVAL 13
#define MODBUS_ADDR_HUM_CTRL_LOW  14
#define MODBUS_ADDR_HUM_CTRL_UP 15
#define MODBUS_ADDR_HUM_CTRL_INVERTOUT 16

#define MODBUS_ADDR_TIMER_ON 17
#define MODBUS_ADDR_TIMER_OFF 18

#define MODBUS_ADDR_PROGRAM 19

#define MODBUS_ID 6
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMainForm *MainForm;
//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner)
        : TForm(Owner)
{
  COMPortOpened = 0;
}


void GetSerialPortsList()
{
    AnsiString KeyName = "\\Hardware\\DeviceMap\\SerialComm";
    TStringList *SerialCommValues = new TStringList();
    MainForm->ComboBox_COMPorts->Items->Clear();
    TRegistry *Registry = new TRegistry;
    try
    {
        Registry->RootKey = HKEY_LOCAL_MACHINE;
        Registry->OpenKeyReadOnly( KeyName );
        Registry->GetValueNames( SerialCommValues );
        for(int i=0; i<SerialCommValues->Count; i++)
        {
            MainForm->ComboBox_COMPorts->Items->Add(Registry->ReadString(SerialCommValues->Strings[i]));
        }
    }
    __finally
    {
        delete Registry;
        delete SerialCommValues;
        if (MainForm->ComboBox_COMPorts->ItemIndex<0)
        {
            MainForm->ComboBox_COMPorts->ItemIndex=0;
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ComboBox_COMPortsDropDown(TObject *Sender)
{
  GetSerialPortsList();        
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormShow(TObject *Sender)
{
  GetSerialPortsList();      
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::Button_OpenClick(TObject *Sender)
{
  AnsiString sName = ComboBox_COMPorts->Text;
  sName = sName.SubString(4, sName.Length()-3);
  int iPort = sName.ToIntDef(-1);
  if (iPort < 0)
  {
    ShowMessage("Incorrect port name");
    return;
  }
  // opening serial port
  int iRes = HAL_Init(iPort, 9600);
  if (iRes != HAL_ERROR_OK)
  {
    ShowMessage("Failed to open port(");
    return;
  }

  COMPortOpened = 1;
}
//---------------------------------------------------------------------------
void SecondsToStr(int num_seconds, char* str)
{
  int hours, minutes;
  hours = num_seconds / (60 * 60);
  num_seconds -= hours * (60 * 60);
  minutes = num_seconds / 60;
  num_seconds -= minutes * 60;

  if (hours == 0)
    sprintf(str, "%02d:%02d", minutes, num_seconds);
  else
    sprintf(str, "%02d:%02d:%02d", hours, minutes, num_seconds);

}
//---------------------------------------------------------------------------
void __fastcall TMainForm::TimerTimer(TObject *Sender)
{
  char sTmp[64];

  short int iData[4];


  if (ModBus_ReadRegs(MODBUS_ID, MODBUS_ADDR_TEMPERATURE, 4, iData) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
  // temperature
  sprintf(sTmp, "%.2f", (iData[0] / 100.0));
  Edt_TempC->Text = sTmp;
  // humidity
  sprintf(sTmp, "%.2f", (iData[1] / 100.0));
  Edt_Hum->Text = sTmp;

  // timer
  SecondsToStr(iData[2] & 0xFFFF, sTmp);
  Edt_Timer->Text = sTmp;

  // output state
  sprintf(sTmp, "%d", int((iData[3] & 0x7FF)/10.0 + 0.5));
  Label_Temp->Caption = sTmp;
  if (iData[3] & 0x2000)
    Label_Hum->Caption = "On";
  else
    Label_Hum->Caption = "Off";
  if (iData[3] & 0x1000)
    Label_Timer->Caption = "On";
  else
    Label_Timer->Caption = "Off";
  /*
  short int iTemperature;
  short int iHumidity;
  if (ModBus_ReadReg(MODBUS_ID, MODBUS_ADDR_TEMPERATURE, &iTemperature) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
  sprintf(sTmp, "%.2f", (iTemperature / 100.0));
  Edt_TempC->Text = sTmp;

  if (ModBus_ReadReg(MODBUS_ID, MODBUS_ADDR_HUMIDITY, &iHumidity) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
  sprintf(sTmp, "%.2f", (iHumidity / 100.0));
  Edt_Hum->Text = sTmp;
  */

/*
  for (int i = 0; i < 16; i++) sTmp[i] = 'A'+i;
  HAL_OutData(sTmp, 16);
  char sIn[64];
  HAL_InData(sIn, 16);
*/
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ChBx_ManualCtrlEnClick(TObject *Sender)
{
  if (ChBx_ManualCtrlEn->Checked)
  {
    TrackBar_HeaterPower->Enabled = true;
    ChBx_HumidityOn->Enabled = true;
    ChBx_TimerOn->Enabled = true;
  }
  else
  {
    TrackBar_HeaterPower->Enabled = false;
    ChBx_HumidityOn->Enabled = false;
    ChBx_TimerOn->Enabled = false;
  }
  UpdateManualControl();
}

void TMainForm::UpdateManualControl()
{
  short int iData;
  
  if (ChBx_ManualCtrlEn->Checked)
  {
    iData = 0x800; // enable manual control
    iData |= TrackBar_HeaterPower->Position*10; // heater control
    if (ChBx_HumidityOn->Checked)
      iData |= 0x1000;
    if (ChBx_TimerOn->Checked)
      iData |= 0x2000;
  }
  else
  {
    iData = 0; // disable manual control
  }

  if (ModBus_WriteReg(MODBUS_ID, MODBUS_ADDR_OUTPUT, iData) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::TrackBar_HeaterPowerChange(TObject *Sender)
{
  UpdateManualControl();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ComboBox_Output1Change(TObject *Sender)
{
  int iData = ComboBox_Output1->ItemIndex;
  
  if (ModBus_WriteReg(MODBUS_ID, MODBUS_ADDR_CH0_MAPPING, iData) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }   
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ComboBox_Output2Change(TObject *Sender)
{
  int iData = ComboBox_Output2->ItemIndex;

  if (ModBus_WriteReg(MODBUS_ID, MODBUS_ADDR_CH1_MAPPING, iData) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ComboBox_Output3Change(TObject *Sender)
{
  int iData = ComboBox_Output3->ItemIndex;

  if (ModBus_WriteReg(MODBUS_ID, MODBUS_ADDR_CH2_MAPPING, iData) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ComboBox_TempCtrlTypeChange(TObject *Sender)
{
  int iData = ComboBox_TempCtrlType->ItemIndex;

  if (ModBus_WriteReg(MODBUS_ID, MODBUS_ADDR_TEMP_CTRL_TYPE, iData) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Edt_TempCtrlIntervalChange(TObject *Sender)
{
  double fData;
  try
  {
    fData = Edt_TempCtrlInterval->Text.ToDouble();
  }
  catch(EConvertError *err)
  {
    return;
  }

  if (ModBus_WriteReg(MODBUS_ID, MODBUS_ADDR_TEMP_CTRL_INTERVAL, fData*10) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Edt_TempOnOff_HysteresisChange(TObject *Sender)
{
  double fData;
  try
  {
    fData = Edt_TempOnOff_Hysteresis->Text.ToDouble();
  }
  catch(EConvertError *err)
  {
    return;
  }

  if (ModBus_WriteReg(MODBUS_ID, MODBUS_ADDR_TEMP_CTRL_HYSTERESIS, round(fData*100)) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Edt_TempCtrlSetpointChange(TObject *Sender)
{
  double fData;
  try
  {
    fData = Edt_TempCtrlSetpoint->Text.ToDouble();
  }
  catch(EConvertError *err)
  {
    return;
  }

  if (ModBus_WriteReg(MODBUS_ID, MODBUS_ADDR_TEMP_CTRL_REF, round(fData*100)) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::Edt_KpChange(TObject *Sender)
{
  double fData;
  try
  {
    fData = Edt_Kp->Text.ToDouble();
  }
  catch(EConvertError *err)
  {
    return;
  }

  if (ModBus_WriteReg(MODBUS_ID, MODBUS_ADDR_TEMP_CTRL_KP, round(fData*100)) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::Edt_KiChange(TObject *Sender)
{
  double fData;
  try
  {
    fData = Edt_Ki->Text.ToDouble();
  }
  catch(EConvertError *err)
  {
    return;
  }

  if (ModBus_WriteReg(MODBUS_ID, MODBUS_ADDR_TEMP_CTRL_KI, round(fData*1000)) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Edt_Hum_UpperValueChange(TObject *Sender)
{
  double fData;
  try
  {
    fData = Edt_Hum_UpperValue->Text.ToDouble();
  }
  catch(EConvertError *err)
  {
    return;
  }

  if (ModBus_WriteReg(MODBUS_ID, MODBUS_ADDR_HUM_CTRL_UP, round(fData*100)) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Edt_Hum_LowerValueChange(TObject *Sender)
{
  double fData;
  try
  {
    fData = Edt_Hum_LowerValue->Text.ToDouble();
  }
  catch(EConvertError *err)
  {
    return;
  }

  if (ModBus_WriteReg(MODBUS_ID, MODBUS_ADDR_HUM_CTRL_LOW, round(fData*100)) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Edt_Hum_PeriodChange(TObject *Sender)
{
  double fData;
  try
  {
    fData = Edt_Hum_Period->Text.ToDouble();
  }
  catch(EConvertError *err)
  {
    return;
  }

  if (ModBus_WriteReg(MODBUS_ID, MODBUS_ADDR_HUM_CTRL_INTERVAL, round(fData*10)) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ChBx_InvertHumOutputClick(TObject *Sender)
{
  int iData = ChBx_InvertHumOutput->Checked;

  if (ModBus_WriteReg(MODBUS_ID, MODBUS_ADDR_HUM_CTRL_INVERTOUT, iData) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::TabSheet_LiveDataShow(TObject *Sender)
{
  if (COMPortOpened)
    Timer->Enabled = true;        
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::TabSheet_LiveDataHide(TObject *Sender)
{
  Timer->Enabled = false;        
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::TabSheet_OutputShow(TObject *Sender)
{
  short int iData[4];

  if (!COMPortOpened) return;

  // read from MODBUS_ADDR_OUTPUT to MODBUS_ADDR_CH2_MAPPING
  if (ModBus_ReadRegs(MODBUS_ID, MODBUS_ADDR_OUTPUT, 4, iData) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }

  // output state
  if (iData[0] & 0x800)
  {
    ChBx_ManualCtrlEn->Checked = 1;
    // manual control enabled
    if (iData[0] & 0x2000)
      ChBx_HumidityOn->Checked = 1;
    else
      ChBx_HumidityOn->Checked = 0;
    if (iData[0] & 0x1000)
      ChBx_TimerOn->Checked = 1;
    else
      ChBx_TimerOn->Checked = 0;

    TrackBar_HeaterPower->Position = (iData[0] & 0x7FF)/10;
  }
  else
  {
    ChBx_ManualCtrlEn->Checked = 0;
    ChBx_HumidityOn->Checked = 0;
    ChBx_TimerOn->Checked = 0;
  }

  ComboBox_Output1->ItemIndex = iData[1];
  ComboBox_Output2->ItemIndex = iData[2];
  ComboBox_Output3->ItemIndex = iData[3];
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::TabSheet_TempCtrlShow(TObject *Sender)
{
  short int iData[6];

  if (!COMPortOpened) return;

  // read from MODBUS_ADDR_OUTPUT to MODBUS_ADDR_CH2_MAPPING
  if (ModBus_ReadRegs(MODBUS_ID, MODBUS_ADDR_TEMP_CTRL_TYPE, 6, iData) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }

  ComboBox_TempCtrlType->ItemIndex = iData[0];
  Edt_TempCtrlInterval->Text = FloatToStr(iData[1] / 10.0);
  Edt_TempOnOff_Hysteresis->Text = FloatToStr(iData[2] / 100.0);

  Edt_Kp->Text = FloatToStr(iData[3] / 100.0);
  Edt_Ki->Text = FloatToStr(iData[4] / 1000.0);

  Edt_TempCtrlSetpoint->Text = FloatToStr(iData[5] / 100.0);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::TabSheet_HumCtrlShow(TObject *Sender)
{
  short int iData[6];

  if (!COMPortOpened) return;

  // read from MODBUS_ADDR_OUTPUT to MODBUS_ADDR_CH2_MAPPING
  if (ModBus_ReadRegs(MODBUS_ID, MODBUS_ADDR_HUM_CTRL_INTERVAL, 4, iData) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }

  Edt_Hum_Period->Text = FloatToStr(iData[0] / 10.0);
  Edt_Hum_LowerValue->Text = FloatToStr(iData[1] / 100.0);
  Edt_Hum_UpperValue->Text = FloatToStr(iData[2] / 100.0);
  ChBx_InvertHumOutput->Checked = iData[3];
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::TabSheet_TimerShow(TObject *Sender)
{
  short int iData[6];
  char sTmp[128];

  if (!COMPortOpened) return;

  // read from MODBUS_ADDR_OUTPUT to MODBUS_ADDR_CH2_MAPPING
  if (ModBus_ReadRegs(MODBUS_ID, MODBUS_ADDR_TIMER_ON, 2, iData) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }

  int iOff = iData[1];

  if (iOff >= 60)
    sprintf(sTmp, "%02d:%02d", iOff/60, iOff % 60);
  else
    sprintf(sTmp, "%02d", iOff);

  Edt_Timer_OffInterval->Text = sTmp;
  Edt_Timer_OnInterval->Text = iData[0];
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::Edt_Timer_OffIntervalChange(TObject *Sender)
{
  int iOff;
  try
  {
    AnsiString s = Edt_Timer_OffInterval->Text;

    int iPos = s.Pos(":");
    if (iPos == 0)
    {
      // seconds only in period
      iOff = s.ToInt();
    }
    else
    {
      // seconds only in period
      AnsiString sHour = s.SubString(1,iPos-1);
      AnsiString sMin = s.SubString(iPos+1,s.Length()-iPos+1);
      iOff = sHour.ToInt()*60 + sMin.ToInt();
    }
  }
  catch(EConvertError *err)
  {
    return;
  }

  if (ModBus_WriteReg(MODBUS_ID, MODBUS_ADDR_TIMER_OFF, iOff) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Edt_Timer_OnIntervalChange(TObject *Sender)
{
  int iOn;
  try
  {
    iOn = Edt_Timer_OnInterval->Text.ToInt();
  }
  catch(EConvertError *err)
  {
    return;
  }

  if (ModBus_WriteReg(MODBUS_ID, MODBUS_ADDR_TIMER_ON, iOn) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
  short int iData;
  // writing enything for initiation of flash programming
  if (ModBus_WriteReg(MODBUS_ID, MODBUS_ADDR_PROGRAM, iData) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
}
//---------------------------------------------------------------------------

