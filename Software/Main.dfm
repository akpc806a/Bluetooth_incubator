object MainForm: TMainForm
  Left = 193
  Top = 124
  BorderStyle = bsDialog
  Caption = 'Incubator controller'
  ClientHeight = 248
  ClientWidth = 350
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnClose = FormClose
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Label3: TLabel
    Left = 16
    Top = 88
    Width = 44
    Height = 13
    Caption = 'Output 1:'
  end
  object PageControl: TPageControl
    Left = 0
    Top = 0
    Width = 350
    Height = 248
    ActivePage = TabSheet_Timer
    Align = alClient
    TabIndex = 5
    TabOrder = 0
    object TabSheet_Connect: TTabSheet
      Caption = 'Connect'
      ImageIndex = 1
      object ComboBox_COMPorts: TComboBox
        Left = 8
        Top = 16
        Width = 145
        Height = 21
        ItemHeight = 0
        TabOrder = 0
        Text = 'ComboBox_COMPorts'
        OnDropDown = ComboBox_COMPortsDropDown
      end
      object Button_Open: TButton
        Left = 160
        Top = 12
        Width = 75
        Height = 25
        Caption = 'Open'
        TabOrder = 1
        OnClick = Button_OpenClick
      end
    end
    object TabSheet_LiveData: TTabSheet
      Caption = 'Live data'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -19
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
      OnHide = TabSheet_LiveDataHide
      OnShow = TabSheet_LiveDataShow
      object Label_Timer: TLabel
        Left = 200
        Top = 120
        Width = 26
        Height = 24
        Caption = 'On'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clLime
        Font.Height = -19
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentFont = False
      end
      object Label_Hum: TLabel
        Left = 200
        Top = 72
        Width = 26
        Height = 24
        Caption = 'On'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clLime
        Font.Height = -19
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentFont = False
      end
      object Label_Temp: TLabel
        Left = 200
        Top = 24
        Width = 26
        Height = 24
        Caption = 'On'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clLime
        Font.Height = -19
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentFont = False
      end
      object Label16: TLabel
        Left = 144
        Top = 28
        Width = 24
        Height = 24
        Caption = 'oC'
      end
      object Label17: TLabel
        Left = 144
        Top = 76
        Width = 15
        Height = 24
        Caption = '%'
      end
      object Edt_TempC: TEdit
        Left = 16
        Top = 24
        Width = 121
        Height = 32
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -19
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentFont = False
        TabOrder = 0
        Text = 'Edt_TempC'
      end
      object Edt_Hum: TEdit
        Left = 16
        Top = 72
        Width = 121
        Height = 32
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -19
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentFont = False
        TabOrder = 1
        Text = 'Edt_Hum'
      end
      object Edt_Timer: TEdit
        Left = 16
        Top = 120
        Width = 121
        Height = 32
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -19
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentFont = False
        TabOrder = 2
        Text = 'Edt_Timer'
      end
    end
    object TabSheet_Output: TTabSheet
      Caption = 'Output'
      ImageIndex = 4
      OnShow = TabSheet_OutputShow
      object Label1: TLabel
        Left = 16
        Top = 16
        Width = 44
        Height = 13
        Caption = 'Output 1:'
      end
      object Label2: TLabel
        Left = 16
        Top = 40
        Width = 44
        Height = 13
        Caption = 'Output 2:'
      end
      object Label4: TLabel
        Left = 16
        Top = 64
        Width = 44
        Height = 13
        Caption = 'Output 3:'
      end
      object ComboBox_Output1: TComboBox
        Left = 72
        Top = 13
        Width = 105
        Height = 21
        ItemHeight = 13
        ItemIndex = 0
        TabOrder = 0
        Text = 'Temperature'
        OnChange = ComboBox_Output1Change
        Items.Strings = (
          'Temperature'
          'Humidity'
          'Timer')
      end
      object ComboBox_Output2: TComboBox
        Left = 72
        Top = 37
        Width = 105
        Height = 21
        ItemHeight = 13
        ItemIndex = 1
        TabOrder = 1
        Text = 'Humidity'
        OnChange = ComboBox_Output2Change
        Items.Strings = (
          'Temperature'
          'Humidity'
          'Timer')
      end
      object ComboBox_Output3: TComboBox
        Left = 72
        Top = 61
        Width = 105
        Height = 21
        ItemHeight = 13
        ItemIndex = 2
        TabOrder = 2
        Text = 'Timer'
        OnChange = ComboBox_Output3Change
        Items.Strings = (
          'Temperature'
          'Humidity'
          'Timer')
      end
      object GroupBox4: TGroupBox
        Left = 8
        Top = 88
        Width = 225
        Height = 97
        Caption = 'Manual control '
        TabOrder = 3
        object Label15: TLabel
          Left = 16
          Top = 35
          Width = 32
          Height = 13
          Caption = 'Heater'
        end
        object ChBx_HumidityOn: TCheckBox
          Left = 16
          Top = 56
          Width = 105
          Height = 17
          Caption = 'Humidity ouput on'
          Enabled = False
          TabOrder = 0
          OnClick = TrackBar_HeaterPowerChange
        end
        object ChBx_TimerOn: TCheckBox
          Left = 16
          Top = 72
          Width = 97
          Height = 17
          Caption = 'Timer output on'
          Enabled = False
          TabOrder = 1
          OnClick = TrackBar_HeaterPowerChange
        end
        object TrackBar_HeaterPower: TTrackBar
          Left = 49
          Top = 30
          Width = 168
          Height = 25
          Enabled = False
          Max = 100
          Orientation = trHorizontal
          Frequency = 5
          Position = 0
          SelEnd = 0
          SelStart = 0
          TabOrder = 2
          TickMarks = tmBottomRight
          TickStyle = tsAuto
          OnChange = TrackBar_HeaterPowerChange
        end
        object ChBx_ManualCtrlEn: TCheckBox
          Left = 8
          Top = 15
          Width = 129
          Height = 17
          Caption = 'Manual control enable'
          TabOrder = 3
          OnClick = ChBx_ManualCtrlEnClick
        end
      end
    end
    object TabSheet_TempCtrl: TTabSheet
      Caption = 'Temp. control'
      ImageIndex = 2
      OnShow = TabSheet_TempCtrlShow
      object Label11: TLabel
        Left = 16
        Top = 11
        Width = 76
        Height = 13
        Caption = 'Setpoint ref (oC)'
      end
      object GroupBox1: TGroupBox
        Left = 8
        Top = 144
        Width = 313
        Height = 73
        Caption = 'PI controller parameters '
        TabOrder = 0
        object Label5: TLabel
          Left = 8
          Top = 19
          Width = 65
          Height = 13
          Caption = 'Kp coefficient'
        end
        object Label6: TLabel
          Left = 8
          Top = 43
          Width = 61
          Height = 13
          Caption = 'Ki coefficient'
        end
        object Btn_PI_Tune: TButton
          Left = 216
          Top = 24
          Width = 75
          Height = 25
          Caption = 'Tune'
          TabOrder = 0
        end
        object Edt_Ki: TEdit
          Left = 88
          Top = 40
          Width = 121
          Height = 21
          TabOrder = 1
          Text = '0'
          OnChange = Edt_KiChange
        end
        object Edt_Kp: TEdit
          Left = 88
          Top = 16
          Width = 121
          Height = 21
          TabOrder = 2
          Text = '0'
          OnChange = Edt_KpChange
        end
      end
      object GroupBox2: TGroupBox
        Left = 8
        Top = 100
        Width = 313
        Height = 41
        Caption = 'On-Off controller parameters '
        TabOrder = 1
        object Label7: TLabel
          Left = 8
          Top = 19
          Width = 70
          Height = 13
          Caption = 'Hysteresis (oC)'
        end
        object Edt_TempOnOff_Hysteresis: TEdit
          Left = 88
          Top = 16
          Width = 121
          Height = 21
          TabOrder = 0
          Text = '0'
          OnChange = Edt_TempOnOff_HysteresisChange
        end
      end
      object GroupBox5: TGroupBox
        Left = 8
        Top = 32
        Width = 313
        Height = 65
        Caption = 'General parameters '
        TabOrder = 2
        object Label10: TLabel
          Left = 8
          Top = 43
          Width = 76
          Height = 13
          Caption = 'Ctrl. period (sec)'
        end
        object Label8: TLabel
          Left = 16
          Top = 20
          Width = 24
          Height = 13
          Caption = 'Type'
        end
        object Edt_TempCtrlInterval: TEdit
          Left = 88
          Top = 40
          Width = 121
          Height = 21
          TabOrder = 0
          Text = '0'
          OnChange = Edt_TempCtrlIntervalChange
        end
        object ComboBox_TempCtrlType: TComboBox
          Left = 88
          Top = 16
          Width = 121
          Height = 21
          ItemHeight = 13
          TabOrder = 1
          Text = 'PI controller'
          OnChange = ComboBox_TempCtrlTypeChange
          Items.Strings = (
            'On-Off controller'
            'PI controller'
            'Hybrid')
        end
      end
      object Edt_TempCtrlSetpoint: TEdit
        Left = 96
        Top = 8
        Width = 121
        Height = 21
        TabOrder = 3
        Text = '0'
        OnChange = Edt_TempCtrlSetpointChange
      end
    end
    object TabSheet_HumCtrl: TTabSheet
      Caption = 'Hum. control'
      ImageIndex = 3
      OnShow = TabSheet_HumCtrlShow
      object GroupBox3: TGroupBox
        Left = 8
        Top = 8
        Width = 313
        Height = 97
        Caption = 'On-Off controller parameters '
        TabOrder = 0
        object Label9: TLabel
          Left = 8
          Top = 19
          Width = 58
          Height = 13
          Caption = 'Upper value'
        end
        object Label12: TLabel
          Left = 8
          Top = 67
          Width = 76
          Height = 13
          Caption = 'Ctrl. period (sec)'
        end
        object Label18: TLabel
          Left = 8
          Top = 43
          Width = 58
          Height = 13
          Caption = 'Lower value'
        end
        object Edt_Hum_UpperValue: TEdit
          Left = 88
          Top = 16
          Width = 121
          Height = 21
          TabOrder = 0
          Text = '0'
          OnChange = Edt_Hum_UpperValueChange
        end
        object Edt_Hum_Period: TEdit
          Left = 88
          Top = 64
          Width = 121
          Height = 21
          TabOrder = 1
          Text = '0'
          OnChange = Edt_Hum_PeriodChange
        end
        object Edt_Hum_LowerValue: TEdit
          Left = 88
          Top = 40
          Width = 121
          Height = 21
          TabOrder = 2
          Text = '0'
          OnChange = Edt_Hum_LowerValueChange
        end
      end
      object ChBx_InvertHumOutput: TCheckBox
        Left = 8
        Top = 112
        Width = 97
        Height = 17
        Caption = 'Invert output'
        TabOrder = 1
        OnClick = ChBx_InvertHumOutputClick
      end
    end
    object TabSheet_Timer: TTabSheet
      Caption = 'Timer'
      ImageIndex = 5
      OnShow = TabSheet_TimerShow
      object Label13: TLabel
        Left = 8
        Top = 27
        Width = 97
        Height = 13
        Caption = 'Off interval (HH:MM)'
      end
      object Label14: TLabel
        Left = 8
        Top = 51
        Width = 77
        Height = 13
        Caption = 'On interval (sec)'
      end
      object Edt_Timer_OffInterval: TEdit
        Left = 128
        Top = 24
        Width = 121
        Height = 21
        TabOrder = 0
        Text = '0'
        OnChange = Edt_Timer_OffIntervalChange
      end
      object Edt_Timer_OnInterval: TEdit
        Left = 128
        Top = 48
        Width = 121
        Height = 21
        TabOrder = 1
        Text = '0'
        OnChange = Edt_Timer_OnIntervalChange
      end
    end
  end
  object Timer: TTimer
    Enabled = False
    OnTimer = TimerTimer
    Left = 296
    Top = 176
  end
end
