//---------------------------------------------------------------------------

#ifndef TabbedTemplateH
#define TabbedTemplateH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Gestures.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.TabControl.hpp>
#include <FMX.Types.hpp>
#include <FMX.ListBox.hpp>
#include <FMX.Controls.Presentation.hpp>
#include <FMX.Edit.hpp>

//---------------------------------------------------------------------------
class TTabbedForm : public TForm
{
__published:	// IDE-managed Components
	TTabControl *TabControl;
	TTabItem *TabItem_Connect;
	TTabItem *TabItem_Data;
	TTabItem *TabItem_Output;
	TTabItem *TabItem_TempCtrl;
	TTabItem *TabItem_HumCtrl;
	TTabItem *TabItem_Timer;
	TComboBox *ComboBoxPaired;
	TTimer *Timer;
    TEdit *Edt_TempC;
    TEdit *Edt_Hum;
    TEdit *Edt_Timer;
    TLabel *Label_Temp;
    TLabel *Label_Hum;
    TLabel *Label_Timer;
    TLabel *Label1;
    TLabel *Label2;
    TLabel *Label3;
    TLabel *Label4;
    TLabel *Label5;
    TComboBox *ComboBox_Output1;
    TComboBox *ComboBox_Output2;
    TComboBox *ComboBox_Output3;
    TCheckBox *ChBx_ManualCtrlEn;
	TTrackBar *TrackBar_HeaterPower;
    TLabel *Label6;
    TCheckBox *ChBx_HumidityOn;
    TCheckBox *ChBx_TimerOn;
	TLabel *Label7;
	TEdit *Edt_TempCtrlSetpoint;
	TLabel *Label8;
	TComboBox *ComboBox_TempCtrlType;
	TListBoxItem *ListBoxItem1;
	TListBoxItem *ListBoxItem2;
	TListBoxItem *ListBoxItem3;
	TEdit *Edt_TempCtrlInterval;
	TLabel *Label9;
	TLabel *Label10;
	TEdit *Edt_TempOnOff_Hysteresis;
	TLabel *Label11;
	TEdit *Edt_Kp;
	TEdit *Edt_Ki;
	TLabel *Label12;
	TLabel *Label13;
	TEdit *Edt_Hum_UpperValue;
	TEdit *Edt_Hum_LowerValue;
	TLabel *Label14;
	TEdit *Edt_Hum_Period;
	TLabel *Label15;
	TLabel *Label16;
	TEdit *Edt_Timer_OffInterval;
	TLabel *Label17;
	TEdit *Edt_Timer_OnInterval;
	TCheckBox *ChBx_InvertHumOutput;
	TTabItem *TabItem_Protect;
	TEdit *Edt_OTP_Value;
	TLabel *Label18;
	TLabel *Label19;
	TEdit *Edt_OTP_Time;
	TEdit *Edt_UTP_Time;
	TLabel *Label20;
	TLabel *Label21;
	TEdit *Edt_UTP_Value;
	TLabel *Label22;
	TEdit *Edt_OHP_Value;
	TEdit *Edt_OHP_Time;
	TLabel *Label23;
	TLabel *Label24;
	TEdit *Edt_UHP_Value;
	TEdit *Edt_UHP_Time;
	TLabel *Label25;
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall Button1Click(TObject *Sender);
    void __fastcall ComboBoxPairedChange(TObject *Sender);
    void __fastcall TimerTimer(TObject *Sender);
    void __fastcall TabControlChange(TObject *Sender);
	void __fastcall ChBx_ManualCtrlEnChange(TObject *Sender);
	void __fastcall TrackBar_HeaterPowerChange(TObject *Sender);
	void __fastcall ChBx_HumidityOnChange(TObject *Sender);
	void __fastcall ChBx_TimerOnChange(TObject *Sender);
	void __fastcall ComboBox_Output1Change(TObject *Sender);
	void __fastcall ComboBox_Output2Change(TObject *Sender);
	void __fastcall ComboBox_Output3Change(TObject *Sender);
	void __fastcall Edt_TempCtrlIntervalChange(TObject *Sender);
	void __fastcall ComboBox_TempCtrlTypeChange(TObject *Sender);
	void __fastcall Edt_TempCtrlSetpointChange(TObject *Sender);
	void __fastcall Edt_TempOnOff_HysteresisChange(TObject *Sender);
	void __fastcall Edt_KpChange(TObject *Sender);
	void __fastcall Edt_KiChange(TObject *Sender);
	void __fastcall Edt_Hum_UpperValueChange(TObject *Sender);
	void __fastcall Edt_Hum_LowerValueChange(TObject *Sender);
	void __fastcall Edt_Hum_PeriodChange(TObject *Sender);
	void __fastcall ChBx_InvertHumOutputChange(TObject *Sender);
	void __fastcall Edt_Timer_OffIntervalChange(TObject *Sender);
	void __fastcall Edt_Timer_OnIntervalChange(TObject *Sender);
	void __fastcall TabItem_TimerClick(TObject *Sender);
	void __fastcall TabItem_HumCtrlClick(TObject *Sender);
	void __fastcall TabItem_TempCtrlClick(TObject *Sender);
	void __fastcall TabItem_OutputClick(TObject *Sender);
	void __fastcall TabItem_ProtectClick(TObject *Sender);
	void __fastcall Edt_UHP_TimeChange(TObject *Sender);
	void __fastcall Edt_OHP_TimeChange(TObject *Sender);
	void __fastcall Edt_UHP_ValueChange(TObject *Sender);
	void __fastcall Edt_OHP_ValueChange(TObject *Sender);
	void __fastcall Edt_UTP_TimeChange(TObject *Sender);
	void __fastcall Edt_UTP_ValueChange(TObject *Sender);
	void __fastcall Edt_OTP_TimeChange(TObject *Sender);
	void __fastcall Edt_OTP_ValueChange(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
private:	// User declarations
	bool FConnected;
	void UpdateManualControl();
public:		// User declarations
	__fastcall TTabbedForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TTabbedForm *TabbedForm;
//---------------------------------------------------------------------------
#endif
