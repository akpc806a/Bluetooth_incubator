//---------------------------------------------------------------------------

#ifndef MainH
#define MainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TMainForm : public TForm
{
__published:	// IDE-managed Components
        TPageControl *PageControl;
        TTabSheet *TabSheet_LiveData;
        TTabSheet *TabSheet_Connect;
        TComboBox *ComboBox_COMPorts;
        TButton *Button_Open;
        TEdit *Edt_TempC;
        TEdit *Edt_Hum;
        TEdit *Edt_Timer;
        TLabel *Label_Timer;
        TLabel *Label_Hum;
        TLabel *Label_Temp;
        TTabSheet *TabSheet_TempCtrl;
        TTabSheet *TabSheet_HumCtrl;
        TTabSheet *TabSheet_Output;
        TTabSheet *TabSheet_Timer;
        TLabel *Label1;
        TComboBox *ComboBox_Output1;
        TComboBox *ComboBox_Output2;
        TLabel *Label2;
        TLabel *Label3;
        TLabel *Label4;
        TComboBox *ComboBox_Output3;
        TGroupBox *GroupBox1;
        TButton *Btn_PI_Tune;
        TEdit *Edt_Ki;
        TEdit *Edt_Kp;
        TLabel *Label5;
        TLabel *Label6;
        TGroupBox *GroupBox2;
        TEdit *Edt_TempOnOff_Hysteresis;
        TLabel *Label7;
        TGroupBox *GroupBox3;
        TLabel *Label9;
        TEdit *Edt_Hum_UpperValue;
        TCheckBox *ChBx_InvertHumOutput;
        TEdit *Edt_Hum_Period;
        TLabel *Label12;
        TEdit *Edt_Timer_OffInterval;
        TLabel *Label13;
        TEdit *Edt_Timer_OnInterval;
        TLabel *Label14;
        TTimer *Timer;
        TGroupBox *GroupBox4;
        TCheckBox *ChBx_HumidityOn;
        TCheckBox *ChBx_TimerOn;
        TTrackBar *TrackBar_HeaterPower;
        TCheckBox *ChBx_ManualCtrlEn;
        TLabel *Label15;
        TLabel *Label16;
        TLabel *Label17;
        TEdit *Edt_Hum_LowerValue;
        TLabel *Label18;
        TGroupBox *GroupBox5;
        TEdit *Edt_TempCtrlInterval;
        TLabel *Label10;
        TLabel *Label8;
        TComboBox *ComboBox_TempCtrlType;
        TLabel *Label11;
        TEdit *Edt_TempCtrlSetpoint;
        void __fastcall ComboBox_COMPortsDropDown(TObject *Sender);
        void __fastcall FormShow(TObject *Sender);
        void __fastcall Button_OpenClick(TObject *Sender);
        void __fastcall TimerTimer(TObject *Sender);
        void __fastcall ChBx_ManualCtrlEnClick(TObject *Sender);
        void __fastcall TrackBar_HeaterPowerChange(TObject *Sender);
        void __fastcall ComboBox_Output1Change(TObject *Sender);
        void __fastcall ComboBox_Output2Change(TObject *Sender);
        void __fastcall ComboBox_Output3Change(TObject *Sender);
        void __fastcall ComboBox_TempCtrlTypeChange(TObject *Sender);
        void __fastcall Edt_TempCtrlIntervalChange(TObject *Sender);
        void __fastcall Edt_TempOnOff_HysteresisChange(TObject *Sender);
        void __fastcall Edt_TempCtrlSetpointChange(TObject *Sender);
        void __fastcall Edt_KpChange(TObject *Sender);
        void __fastcall Edt_KiChange(TObject *Sender);
        void __fastcall Edt_Hum_UpperValueChange(TObject *Sender);
        void __fastcall Edt_Hum_LowerValueChange(TObject *Sender);
        void __fastcall Edt_Hum_PeriodChange(TObject *Sender);
        void __fastcall ChBx_InvertHumOutputClick(TObject *Sender);
        void __fastcall TabSheet_LiveDataShow(TObject *Sender);
        void __fastcall TabSheet_LiveDataHide(TObject *Sender);
        void __fastcall TabSheet_OutputShow(TObject *Sender);
        void __fastcall TabSheet_TempCtrlShow(TObject *Sender);
        void __fastcall TabSheet_HumCtrlShow(TObject *Sender);
        void __fastcall TabSheet_TimerShow(TObject *Sender);
        void __fastcall Edt_Timer_OffIntervalChange(TObject *Sender);
        void __fastcall Edt_Timer_OnIntervalChange(TObject *Sender);
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
private:	// User declarations
        void UpdateManualControl();
        unsigned char COMPortOpened;

public:		// User declarations
        __fastcall TMainForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif
