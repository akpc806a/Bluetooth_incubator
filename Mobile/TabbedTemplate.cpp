//---------------------------------------------------------------------------

#include <fmx.h>
#include <System.UITypes.hpp>
#include <stdio.h>
#include <cstdlib>
#pragma hdrstop

#include "TabbedTemplate.h"
#include "Interface.h"
#include "Modbus.h"

#pragma package(smart_init)
#pragma resource "*.fmx"
#pragma resource ("*.NmXhdpiPh.fmx", _PLAT_ANDROID)
#pragma resource ("*.SmXhdpiPh.fmx", _PLAT_ANDROID)

TTabbedForm *TabbedForm;


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

#define MODBUS_ADDR_OTP_VALUE 19
#define MODBUS_ADDR_OTP_TIME 20
#define MODBUS_ADDR_UTP_VALUE 21
#define MODBUS_ADDR_UTP_TIME 22
#define MODBUS_ADDR_OHP_VALUE 23
#define MODBUS_ADDR_OHP_TIME 24
#define MODBUS_ADDR_UHP_VALUE 25
#define MODBUS_ADDR_UHP_TIME 26

#define MODBUS_ADDR_PROGRAM 27

#define MODBUS_ID 6

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
__fastcall TTabbedForm::TTabbedForm(TComponent* Owner)
  : TForm(Owner)
{
    FConnected = 0;
    FTunning = 0;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void __fastcall TTabbedForm::FormCreate(TObject *Sender)
{
  // This defines the default active tab at runtime
  TabControl->ActiveTab = TabItem_Connect;
  //PairedDevices();
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void __fastcall TTabbedForm::FormShow(TObject *Sender)
{
    if (HAL_Init() == HAL_ERROR_OK)
    {
      TStringList* device_list = new TStringList();
      HAL_GetDeviceList(device_list);
      ComboBoxPaired->Items->Assign(device_list);
    }
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
void __fastcall TTabbedForm::ComboBoxPairedChange(TObject *Sender)
{
  if (HAL_Connect(ComboBoxPaired->ItemIndex) == HAL_ERROR_OK)
  {
    FConnected = 1;
  }
}
//---------------------------------------------------------------------------

void __fastcall TTabbedForm::TimerTimer(TObject *Sender)
{
  char sTmp[64];

  short int iData[4];
  int iRes = 0;

  if (!FConnected) return;

  iRes = ModBus_ReadRegs(MODBUS_ID, MODBUS_ADDR_TEMPERATURE, 4, iData);
  if (iRes != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
  /*
  {   #define MODBUS_ERROR_OK 0
#define MODBUS_ERROR_IO 1
#define MODBUS_ERROR_TIMEOUT 2
#define MODBUS_ERROR_CRC 3
#define MODBUS_ERROR_FORMAT 4
    Caption = "BAD";
  }
  */
  // temperature
  float fTemperature = (iData[0] / 100.0);
  sprintf(sTmp, "%.2f", fTemperature);
  Edt_TempC->Text = sTmp;

  if (FTunning)
  {
    iTuningTimer++;
    float fTime = iTuningTimer*0.5; // time in seconds (0.5 is TTimer period)
    Tuning_AddPoint(fTime, fTemperature);
  }

  // humidity
  sprintf(sTmp, "%.2f", (iData[1] / 100.0));
  Edt_Hum->Text = sTmp;

  // timer
  SecondsToStr(iData[2] & 0xFFFF, sTmp);
  Edt_Timer->Text = sTmp;

  // output state
  sprintf(sTmp, "%d", int((iData[3] & 0x7FF)/10.0 + 0.5));
  Label_Temp->Text = sTmp;
  if (iData[3] & 0x2000)
    Label_Hum->Text = "On";
  else
    Label_Hum->Text = "Off";
  if (iData[3] & 0x1000)
    Label_Timer->Text = "On";
  else
    Label_Timer->Text = "Off";
}
//---------------------------------------------------------------------------

void __fastcall TTabbedForm::TabControlChange(TObject *Sender)
{
  if (TabControl->TabIndex == 1 && FConnected)
  {
    Timer->Enabled = 1;
  }
  else
  if (Timer->Enabled) 
  {
    Timer->Enabled = 0;
  }
}
//---------------------------------------------------------------------------
void TTabbedForm::UpdateManualControl()
{
  short int iData;

  if (!FConnected) return;

  if (ChBx_ManualCtrlEn->IsChecked)
  {
    iData = 0x800; // enable manual control
    iData |= int(TrackBar_HeaterPower->Value)*10; // heater control
    if (ChBx_HumidityOn->IsChecked)
      iData |= 0x1000;
    if (ChBx_TimerOn->IsChecked)
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

void __fastcall TTabbedForm::ChBx_ManualCtrlEnChange(TObject *Sender)
{
  if (ChBx_ManualCtrlEn->IsChecked)
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
//---------------------------------------------------------------------------

void __fastcall TTabbedForm::TrackBar_HeaterPowerChange(TObject *Sender)
{
  UpdateManualControl();
}
//---------------------------------------------------------------------------

void __fastcall TTabbedForm::ChBx_HumidityOnChange(TObject *Sender)
{
  UpdateManualControl();
}
//---------------------------------------------------------------------------

void __fastcall TTabbedForm::ChBx_TimerOnChange(TObject *Sender)
{
  UpdateManualControl();
}
//---------------------------------------------------------------------------

void __fastcall TTabbedForm::ComboBox_Output1Change(TObject *Sender)
{
  if (!FConnected) return;

  int iData = ComboBox_Output1->ItemIndex;

  if (ModBus_WriteReg(MODBUS_ID, MODBUS_ADDR_CH0_MAPPING, iData) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
}
//---------------------------------------------------------------------------

void __fastcall TTabbedForm::ComboBox_Output2Change(TObject *Sender)
{
  if (!FConnected) return;

  int iData = ComboBox_Output2->ItemIndex;

  if (ModBus_WriteReg(MODBUS_ID, MODBUS_ADDR_CH1_MAPPING, iData) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
}
//---------------------------------------------------------------------------

void __fastcall TTabbedForm::ComboBox_Output3Change(TObject *Sender)
{
  if (!FConnected) return;

  int iData = ComboBox_Output3->ItemIndex;

  if (ModBus_WriteReg(MODBUS_ID, MODBUS_ADDR_CH2_MAPPING, iData) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
}
//---------------------------------------------------------------------------

void __fastcall TTabbedForm::Edt_TempCtrlIntervalChange(TObject *Sender)
{
  if (!FConnected) return;

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

void __fastcall TTabbedForm::ComboBox_TempCtrlTypeChange(TObject *Sender)
{
  if (!FConnected) return;

  int iData = ComboBox_TempCtrlType->ItemIndex;

  if (ModBus_WriteReg(MODBUS_ID, MODBUS_ADDR_TEMP_CTRL_TYPE, iData) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
}
//---------------------------------------------------------------------------

void __fastcall TTabbedForm::Edt_TempCtrlSetpointChange(TObject *Sender)
{
  if (!FConnected) return;

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

void __fastcall TTabbedForm::Edt_TempOnOff_HysteresisChange(TObject *Sender)
{
  if (!FConnected) return;

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

void __fastcall TTabbedForm::Edt_KpChange(TObject *Sender)
{
  if (!FConnected) return;

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

void __fastcall TTabbedForm::Edt_KiChange(TObject *Sender)
{
  if (!FConnected) return;

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

void __fastcall TTabbedForm::Edt_Hum_UpperValueChange(TObject *Sender)
{
  if (!FConnected) return;

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

void __fastcall TTabbedForm::Edt_Hum_LowerValueChange(TObject *Sender)
{
  if (!FConnected) return;

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

void __fastcall TTabbedForm::Edt_Hum_PeriodChange(TObject *Sender)
{
  if (!FConnected) return;

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

void __fastcall TTabbedForm::ChBx_InvertHumOutputChange(TObject *Sender)
{
  if (!FConnected) return;

  int iData = ChBx_InvertHumOutput->IsChecked;

  if (ModBus_WriteReg(MODBUS_ID, MODBUS_ADDR_HUM_CTRL_INVERTOUT, iData) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
}
//---------------------------------------------------------------------------

void __fastcall TTabbedForm::Edt_Timer_OffIntervalChange(TObject *Sender)
{
  if (!FConnected) return;

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

void __fastcall TTabbedForm::Edt_Timer_OnIntervalChange(TObject *Sender)
{
  if (!FConnected) return;

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

void __fastcall TTabbedForm::TabItem_TimerClick(TObject *Sender)
{
  short int iData[6];
  AnsiString sTmp;

  if (!FConnected) return;

  // read from MODBUS_ADDR_OUTPUT to MODBUS_ADDR_CH2_MAPPING
  if (ModBus_ReadRegs(MODBUS_ID, MODBUS_ADDR_TIMER_ON, 2, iData) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }

  int iOff = iData[1];

  if (iOff >= 60)
  sTmp = sTmp.sprintf("%02d:%02d", iOff/60, iOff % 60);
  else
    sTmp = sTmp.sprintf("%02d", iOff);

  Edt_Timer_OffInterval->Text = sTmp;
  Edt_Timer_OnInterval->Text = AnsiString(iData[0]);
}
//---------------------------------------------------------------------------

void __fastcall TTabbedForm::TabItem_HumCtrlClick(TObject *Sender)
{
  short int iData[6];

  if (!FConnected) return;

  // read from MODBUS_ADDR_OUTPUT to MODBUS_ADDR_CH2_MAPPING
  if (ModBus_ReadRegs(MODBUS_ID, MODBUS_ADDR_HUM_CTRL_INTERVAL, 4, iData) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }

  Edt_Hum_Period->Text = FloatToStr(iData[0] / 10.0);
  Edt_Hum_LowerValue->Text = FloatToStr(iData[1] / 100.0);
  Edt_Hum_UpperValue->Text = FloatToStr(iData[2] / 100.0);
  ChBx_InvertHumOutput->IsChecked = iData[3];
}
//---------------------------------------------------------------------------

void __fastcall TTabbedForm::TabItem_TempCtrlClick(TObject *Sender)
{
  short int iData[6];

  if (!FConnected) return;

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

void __fastcall TTabbedForm::TabItem_OutputClick(TObject *Sender)
{
  short int iData[4];

  if (!FConnected) return;

  // read from MODBUS_ADDR_OUTPUT to MODBUS_ADDR_CH2_MAPPING
  if (ModBus_ReadRegs(MODBUS_ID, MODBUS_ADDR_OUTPUT, 4, iData) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }

  // output state
  if (iData[0] & 0x800)
  {
    ChBx_ManualCtrlEn->IsChecked = 1;
    // manual control enabled
    if (iData[0] & 0x2000)
    ChBx_HumidityOn->IsChecked = 1;
    else
    ChBx_HumidityOn->IsChecked = 0;
    if (iData[0] & 0x1000)
    ChBx_TimerOn->IsChecked = 1;
    else
      ChBx_TimerOn->IsChecked = 0;

    TrackBar_HeaterPower->Value = (iData[0] & 0x7FF)/10;
  }
  else
  {
    ChBx_ManualCtrlEn->IsChecked = 0;
    ChBx_HumidityOn->IsChecked = 0;
    ChBx_TimerOn->IsChecked = 0;
  }

  ComboBox_Output1->ItemIndex = iData[1];
  ComboBox_Output2->ItemIndex = iData[2];
  ComboBox_Output3->ItemIndex = iData[3];
}
//---------------------------------------------------------------------------

void __fastcall TTabbedForm::TabItem_ProtectClick(TObject *Sender)
{
  short int iData[8];

  if (!FConnected) return;

  // read from MODBUS_ADDR_OTP_VALUE to MODBUS_ADDR_UHP_TIME
  if (ModBus_ReadRegs(MODBUS_ID, MODBUS_ADDR_OTP_VALUE, 8, iData) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }

  Edt_OTP_Value->Text = FloatToStr(iData[0] / 100.0);
  Edt_OTP_Time->Text = FloatToStr(iData[1] / 60.0);
  Edt_UTP_Value->Text = FloatToStr(iData[2] / 100.0);
  Edt_UTP_Time->Text = FloatToStr(iData[3] / 60.0);
  Edt_OHP_Value->Text = FloatToStr(iData[4] / 100.0);
  Edt_OHP_Time->Text = FloatToStr(iData[5] / 60.0);
  Edt_UHP_Value->Text = FloatToStr(iData[6] / 100.0);
  Edt_UHP_Time->Text = FloatToStr(iData[7] / 60.0);
}
//---------------------------------------------------------------------------

void __fastcall TTabbedForm::Edt_UHP_TimeChange(TObject *Sender)
{
  if (!FConnected) return;

  int iValue;
  try
  {
    iValue = Edt_UHP_Time->Text.ToInt();
  }
  catch(EConvertError *err)
  {
    return;
  }

  if (ModBus_WriteReg(MODBUS_ID, MODBUS_ADDR_UHP_TIME, iValue*60) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
}
//---------------------------------------------------------------------------

void __fastcall TTabbedForm::Edt_OHP_TimeChange(TObject *Sender)
{
  if (!FConnected) return;

  int iValue;
  try
  {
    iValue = Edt_OHP_Time->Text.ToInt();
  }
  catch(EConvertError *err)
  {
    return;
  }

  if (ModBus_WriteReg(MODBUS_ID, MODBUS_ADDR_OHP_TIME, iValue*60) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
}
//---------------------------------------------------------------------------

void __fastcall TTabbedForm::Edt_UHP_ValueChange(TObject *Sender)
{
  if (!FConnected) return;

  double fData;
  try
  {
    fData = Edt_UHP_Value->Text.ToDouble();
  }
  catch(EConvertError *err)
  {
    return;
  }

  if (ModBus_WriteReg(MODBUS_ID, MODBUS_ADDR_UHP_VALUE, round(fData*100)) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
}
//---------------------------------------------------------------------------

void __fastcall TTabbedForm::Edt_OHP_ValueChange(TObject *Sender)
{
  if (!FConnected) return;

  double fData;
  try
  {
    fData = Edt_OHP_Value->Text.ToDouble();
  }
  catch(EConvertError *err)
  {
    return;
  }

  if (ModBus_WriteReg(MODBUS_ID, MODBUS_ADDR_OHP_VALUE, round(fData*100)) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
}
//---------------------------------------------------------------------------

void __fastcall TTabbedForm::Edt_UTP_TimeChange(TObject *Sender)
{
  if (!FConnected) return;

  int iValue;
  try
  {
    iValue = Edt_UTP_Time->Text.ToInt();
  }
  catch(EConvertError *err)
  {
    return;
  }

  if (ModBus_WriteReg(MODBUS_ID, MODBUS_ADDR_UTP_TIME, iValue*60) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
}
//---------------------------------------------------------------------------

void __fastcall TTabbedForm::Edt_UTP_ValueChange(TObject *Sender)
{
  if (!FConnected) return;

  double fData;
  try
  {
    fData = Edt_UTP_Value->Text.ToDouble();
  }
  catch(EConvertError *err)
  {
    return;
  }

  if (ModBus_WriteReg(MODBUS_ID, MODBUS_ADDR_UTP_VALUE, round(fData*100)) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
}
//---------------------------------------------------------------------------

void __fastcall TTabbedForm::Edt_OTP_TimeChange(TObject *Sender)
{
  if (!FConnected) return;

  int iValue;
  try
  {
    iValue = Edt_OTP_Time->Text.ToInt();
  }
  catch(EConvertError *err)
  {
    return;
  }

  if (ModBus_WriteReg(MODBUS_ID, MODBUS_ADDR_OTP_TIME, iValue*60) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
}
//---------------------------------------------------------------------------

void __fastcall TTabbedForm::Edt_OTP_ValueChange(TObject *Sender)
{
  if (!FConnected) return;

  double fData;
  try
  {
    fData = Edt_OTP_Value->Text.ToDouble();
  }
  catch(EConvertError *err)
  {
    return;
  }

  if (ModBus_WriteReg(MODBUS_ID, MODBUS_ADDR_OTP_VALUE, round(fData*100)) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
}
//---------------------------------------------------------------------------

void __fastcall TTabbedForm::FormClose(TObject *Sender, TCloseAction &Action)
{
  if (!FConnected) return;

  short int iData;
  // writing enything for initiation of flash programming
  if (ModBus_WriteReg(MODBUS_ID, MODBUS_ADDR_PROGRAM, iData) != MODBUS_ERROR_OK)
  {
    Caption = "BAD";
  }
  Sleep(100);
}
//---------------------------------------------------------------------------
// start auto-tuning, InitialValue -- process output in idle steady-state
void TTabbedForm::Tuning_Start(float InitialValue)
{
  iPointCount = 0;
  fY_0 = InitialValue;

  fY_start = ceil(InitialValue/TUNE_TEMP_DELTA)*TUNE_TEMP_DELTA;
}

// add new measurement to recorded response
void TTabbedForm::Tuning_AddPoint(float Time, float Value)
{
  if (iPointCount >= TUNE_POINT_COUNT) return;

  if (Value >= (fY_start + TUNE_TEMP_DELTA*iPointCount))
  {
    fPoints_T[iPointCount] = Time;
    fPoints_Y[iPointCount] = Value;
    iPointCount++;
  }
}

// error codes for auto-tuning
#define TUNING_RESULT_OK 0
#define TUNING_RESULT_NOT_ENOUGH_DATA 1
#define TUNING_RESULT_WRONG_SHAPE 2
#define TUNING_RESULT_REGR_FAULT 3
// finilize auto-tuning and calculate coefficients, EndValue -- steady state value
// returns error code described above
unsigned char TTabbedForm::Tuning_Finilize(float EndValue)
{
  int i;
  float fMean_T = 0;
  float fMean_Y = 0;
  double fNum = 0;
  double fDen = 0;
  double fB;

  fY_ss = EndValue;

  // shape response to match steady state value
  i = iPointCount-1;
  while (i >= 0)
  {
    if (fPoints_Y[i] >= fY_ss)
      iPointCount--;
    else
      break;

    i--;
  }

  if (iPointCount < 2) return TUNING_RESULT_NOT_ENOUGH_DATA;

  // calculate logarithms -- should be a line in result
  for (i = 0; i < iPointCount; i++)
  {
    if (fY_ss <= fPoints_Y[i]) return TUNING_RESULT_WRONG_SHAPE;
    if (fY_ss <= fY_0) return TUNING_RESULT_WRONG_SHAPE;

    fPoints_Y[i] = -log((fY_ss - fPoints_Y[i])/(fY_ss - fY_0));
  }

  // linear regression for slope calculation

  for (i = 0; i < iPointCount; i++)
    fMean_T = fMean_T + fPoints_T[i];
  fMean_T = fMean_T / iPointCount;

  for (i = 0; i < iPointCount; i++)
    fMean_Y = fMean_Y + fPoints_Y[i];
  fMean_Y = fMean_Y / iPointCount;

  for (i = 0; i < iPointCount; i++)
  {
    fNum = fNum + (fPoints_T[i] - fMean_T)*(fPoints_Y[i] - fMean_Y);
    fDen = fDen + (fPoints_T[i] - fMean_T)*(fPoints_T[i] - fMean_T);
  }
  if (fDen == 0) return TUNING_RESULT_REGR_FAULT;
  fB = fNum / fDen;

  // first order system parameters
  fTau = 1/fB;
  fK = (fY_ss - fY_0)/TUNE_INPUT_VALUE;

  // PI controller parameters
  fK_i = w_0_cl_factor*w_0_cl_factor*fB/fK;
  fK_p = (2*zeta_cl*w_0_cl_factor - 1) / fK;

  return TUNING_RESULT_OK;
}

// function calculates maximal relative error between actual data and first-order approximation
float TTabbedForm::Tuning_CalculateMaxError()
{
  float fRange = fY_ss - fY_0;
  float fError = 0;
  int i;
  float fModelY; // output of model for given input

  // convert back to temperature values
  for (i = 0; i < iPointCount; i++)
    fPoints_Y[i] = fY_ss - exp(-fPoints_Y[i])*(fY_ss - fY_0);

  for (i = 0; i < iPointCount; i++)
  {
    fModelY = fY_0 + fK*TUNE_INPUT_VALUE*(1 - exp(-fPoints_T[i]/fTau));
    if (fabs(fModelY - fPoints_Y[i]) > fError)
      fError = fabs(fModelY - fPoints_Y[i]);
  }

  return (fError/fRange); // relative error
}


void __fastcall TTabbedForm::Btn_TuneClick(TObject *Sender)
{
  if (!FConnected) return;

  if (!FTunning)
  {
    //ShowMessage("PI tuning will start");

    FTunning = 1;
    Label_Tuning->Visible = 1;
    Btn_Tune->Text = "Stop";

    // get temperature
    TimerTimer(Sender);
    float fTemperature = Edt_TempC->Text.ToDouble();

    // enable heater
    ChBx_ManualCtrlEn->IsChecked = 1;
    TrackBar_HeaterPower->Value = 100;
    UpdateManualControl();

    Tuning_Start(fTemperature);

    iTuningTimer = 0;

    Timer->Enabled = 1;
  }
  else
  {
    //ShowMessage("Stop PI tuning");

    FTunning = 0;
    Label_Tuning->Visible = 0;
    Btn_Tune->Text = "PI Tune";

    // get temperature
    float fTemperature = Edt_TempC->Text.ToDouble();
    Tuning_Finilize(fTemperature);

    // set calculated coefficients
    if (ModBus_WriteReg(MODBUS_ID, MODBUS_ADDR_TEMP_CTRL_KP, round(fK_p*100)) != MODBUS_ERROR_OK)
    {
      Caption = "BAD";
    }
    Edt_Kp->Text = FloatToStr(round(fK_p*100)/100.0);


    if (ModBus_WriteReg(MODBUS_ID, MODBUS_ADDR_TEMP_CTRL_KI, round(fK_i*1000)) != MODBUS_ERROR_OK)
    {
      Caption = "BAD";
    }
    Edt_Ki->Text = FloatToStr(round(fK_i*1000)/1000.0);

    Timer->Enabled = 0;
  }




}
//---------------------------------------------------------------------------

