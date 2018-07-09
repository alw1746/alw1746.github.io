; The project measures temperature using a PICAXE-08M to read voltage output from an LM335 sensor.
; This program polls the PICAXE through the serial port for temperature readings and displays
; them in real-time on a GUI interface. Readings are also logged into a CSV file. See the CHM help
; file for more details on PICAXE code and schematic.
; Alex Wong, PIC Club, AUG 2006, Sydney.
; Build with Autoit V3.2.12.1
;
#include <GUIConstants.au3>
#include <ListviewConstants.au3>
#include <WindowsConstants.au3>
#include <ButtonConstants.au3>
#include <EditConstants.au3>
#Include <Date.au3>
#include <Array.au3>
Dim $NetComm
Dim $startTime = 0, $readTimer = 0, $secTimer = 0, $hh, $mm, $ss
Dim $run = False
Dim $firstPort = ""
Dim $ComList = ""
Dim $ComPorts
Dim $file
Dim $buf, $period
Dim $counter = 0, $sampval = 0
Dim $baudrate, $parityval
Dim $minTemp, $maxTemp, $avgTemp, $totTemp

$oMyError = ObjEvent("AutoIt.Error", "MyErrFunc")

_ComputerGetSerialPorts($ComPorts)
If $ComPorts[0] > 0 Then
  _ArraySort($ComPorts)
  $firstPort = $ComPorts[1]
  For $i = 1 To $ComPorts[0] Step 1
    $ComList = $ComList & $ComPorts[$i] & "|"
  Next
EndIf

; == GUI generated with Koda ==
If True Then
  $TempSenForm = GUICreate("Temperature Sensor", 644, 593, 205, 72, -1)
  $SettingsGroup = GUICtrlCreateGroup("Settings", 48, 40, 217, 201)
  $SerialPort = GUICtrlCreateCombo($firstPort, 152, 64, 81, 21)
  GUICtrlSetData($SerialPort, $ComList)
  GUICtrlCreateLabel("Serial Port", 80, 64, 52, 17)
  $Baud = GUICtrlCreateCombo("2400", 152, 88, 81, 21)
  GUICtrlSetData(-1, "1200|2400|4800|9600")
  GUICtrlCreateLabel("Baud Rate", 80, 88, 55, 17)
  $parity = GUICtrlCreateCombo("N,8,1", 152, 112, 81, 21)
  GUICtrlSetData(-1, "N,8,1|N,7,2|E,8,1|E,7,2|O,8,1|O,7,2")
  GUICtrlCreateLabel("Parity", 80, 112, 30, 17)
  $Centigrade = GUICtrlCreateRadio("Centigrade", 80, 144, 73, 17)
  GUICtrlSetState(-1, $GUI_CHECKED)
  $Farenheit = GUICtrlCreateRadio("Farenheit", 168, 144, 65, 17)
  GUICtrlCreateLabel("Sampling Interval", 80, 176, 85, 17)
  $SampInt = GUICtrlCreateInput("1", 184, 176, 41, 21, -1,$WS_EX_CLIENTEDGE )
  $Slider = GUICtrlCreateSlider(72, 200, 161, 20)
  GUICtrlSetCursor($Slider, 13)
  GUICtrlSetData($Slider, "1")
  GUICtrlCreateGroup("", -99, -99, 1, 1)
  $TempLV = GUICtrlCreateListView("          DateTime          | Temp " & Chr(176) & "C ", 48, 272, 217, 273, -1, BitOR($LVS_EX_GRIDLINES, $LVS_REPORT))
  $ConPanelGroup = GUICtrlCreateGroup("Control Panel", 312, 96, 281, 145)
  $StartBtn = GUICtrlCreateButton("Start", 344, 128, 89, 33, $BS_DEFPUSHBUTTON)
  $StopBtn = GUICtrlCreateButton("Stop", 344, 184, 89, 33)
  $HelpBtn = GUICtrlCreateButton("Help", 472, 128, 89, 33)
  $ExitBtn = GUICtrlCreateButton("Exit", 472, 184, 89, 33)
  GUICtrlCreateGroup("", -99, -99, 1, 1)
  $ReadingsGroup = GUICtrlCreateGroup("Readings", 312, 264, 281, 145)
  GUICtrlCreateLabel("Temp", 336, 296, 31, 17)
  $Temp = GUICtrlCreateInput("", 376, 296, 41, 21, BitOR($ES_RIGHT, $ES_READONLY), $WS_EX_CLIENTEDGE)
  GUICtrlSetBkColor(-1, 0xD8E4F8)
  GUICtrlCreateLabel("Min", 336, 320, 21, 17)
  $Min = GUICtrlCreateInput("", 376, 320, 41, 21, BitOR($ES_RIGHT, $ES_READONLY), $WS_EX_CLIENTEDGE)
  GUICtrlSetBkColor(-1, 0xD8E4F8)
  GUICtrlCreateLabel("Max", 336, 344, 24, 17)
  $Max = GUICtrlCreateInput("", 376, 344, 41, 21, BitOR($ES_RIGHT, $ES_READONLY), $WS_EX_CLIENTEDGE)
  GUICtrlSetBkColor(-1, 0xD8E4F8)
  GUICtrlCreateLabel("Avg", 336, 368, 23, 17)
  $Avg = GUICtrlCreateInput("", 376, 368, 41, 21, BitOR($ES_RIGHT, $ES_READONLY), $WS_EX_CLIENTEDGE)
  GUICtrlSetBkColor(-1, 0xD8E4F8)
  GUICtrlCreateLabel("Start", 456, 296, 27, 17)
  $Start = GUICtrlCreateInput("", 512, 296, 49, 21, BitOR($ES_RIGHT, $ES_READONLY), $WS_EX_CLIENTEDGE)
  GUICtrlSetBkColor(-1, 0xD8E4F8)
  GUICtrlCreateLabel("Time", 456, 320, 26, 17)
  $Time = GUICtrlCreateInput("", 512, 320, 49, 21, BitOR($ES_RIGHT, $ES_READONLY), $WS_EX_CLIENTEDGE)
  GUICtrlSetBkColor(-1, 0xD8E4F8)
  GUICtrlCreateLabel("Elapsed", 456, 344, 40, 17)
  $Elapsed = GUICtrlCreateInput("", 512, 344, 49, 21, BitOR($ES_RIGHT, $ES_READONLY), $WS_EX_CLIENTEDGE)
  GUICtrlSetBkColor(-1, 0xD8E4F8)
  GUICtrlCreateLabel("Samples", 456, 368, 44, 17)
  $Samples = GUICtrlCreateInput("", 512, 368, 49, 21, BitOR($ES_RIGHT, $ES_READONLY), $WS_EX_CLIENTEDGE)
  GUICtrlSetBkColor(-1, 0xD8E4F8)
  GUICtrlCreateGroup("", -99, -99, 1, 1)
  GUICtrlCreateLabel("Temperature Sensor", 352, 48, 245, 33)
  GUICtrlSetFont(-1, 18, 800, 0, "Microsoft Sans Serif")
  $TempSenIcon = GUICtrlCreateIcon("C:\autoit\TempSensor\TempSensorBig.ico", 0, 312, 48)
  GUISetState(@SW_SHOW)
EndIf

While True
  $msg = GUIGetMsg()
  Select
    Case $msg = $Slider
      $period = Int(GUICtrlRead($Slider))
      If $period == 0 Then
        $period = 1
      EndIf
      GUICtrlSetData($SampInt, String($period))
      $period = $period * 1000
    Case $msg = $StartBtn
      If Not $run Then
        GUICtrlSetData($Temp, "")
        GUICtrlSetData($Min, "")
        GUICtrlSetData($Max, "")
        GUICtrlSetData($Avg, "")
        GUICtrlSetData($Samples, "")
        $sampval = 0
        $totTemp = 0.0
        $minTemp = 9999.9
        $maxTemp = 0.0
        $avgTemp = 0.0
        $period = Int(GUICtrlRead($SampInt)) * 1000
        $baudrate = GUICtrlRead($Baud)
        $parityval = GUICtrlRead($parity)
        $port = _GetPort()
        If _InitPort($port, $baudrate, $parityval) Then
          $file = FileOpen("TempSensor.csv", 1)
          $run = True
          $startTime = TimerInit()
          $readTimer = TimerInit()
          GUICtrlSetData($Start, _NowTime(5))
          $secTimer = TimerInit() + 2000
          _ShowTimer($secTimer, $startTime)
          $buf = _ReadPort()
          _ShowTemp($buf)
        EndIf
      EndIf
    Case $msg = $StopBtn
      If $run Then
        $run = False
        _ClosePort()
        FileClose($file)
      EndIf
    Case $msg = $SerialPort
      $port = _GetPort()
  Case $msg = $HelpBtn
	  Run('hh.exe ' & @WorkingDir & '\TempSensor.chm')
    Case $msg = $GUI_EVENT_CLOSE
      If $run Then
        $run = False
        _ClosePort()
        FileClose($file)
      EndIf
      ExitLoop
	Case $msg = $ExitBtn
      If $run Then
        $run = False
        _ClosePort()
        FileClose($file)
      EndIf
      ExitLoop
  Case Else
      If $run Then
        _ShowTimer($secTimer, $startTime)
        If TimerDiff($readTimer) >= $period Then
          $buf = _ReadPort()
          _ShowTemp($buf)
          $readTimer = TimerInit()
        EndIf
      EndIf
  EndSelect
WEnd
Exit

; == Start User Code ==
Func OnAutoItExit()
  ConsoleWrite("Close port now" & @CRLF)
  If $run Then
    $NetComm.PortOpen = "False"
    FileClose($file)
  EndIf
EndFunc   ;==>OnAutoItExit

Func _InitPort($comport, $bdrate, $parval)
  Dim $status
  $NetComm = ObjCreate("NETCommOCX.NETComm") ;Create NETComm.ocx object
  If Not @error Then
    With $NetComm
      .CommPort = Int($comport) ;Set port number
      .Settings = $bdrate & "," & $parval      ;Set port settings
      .InputLen = 0                 ;reads entire buffer
      .InputMode = 0                ;reads in text mode
      .HandShaking = 0              ;no handshaking
      .PortOpen = "True"            ;opens specified COM port
    EndWith
    If Not @error Then
      $status = True
    Else
      MsgBox(0, "Open Serial Port", "Failed to open serial port COM" & $comport & ", error code: " & Hex(@error, 8))
    EndIf
  Else
    $status = False
    MsgBox(0, "Open Serial Port", "Failed to create NetComm object, error code: " & Hex(@error, 8))
  EndIf
  Return $status
EndFunc   ;==>_InitPort

; custom error handler
Func MyErrFunc()
  
  $HexNumber = Hex($oMyError.number, 8)
  
  MsgBox(0, "AutoItCOM Test", "We intercepted a COM Error !" & @CRLF & @CRLF & _
      "err.description is: " & @TAB & $oMyError.description & @CRLF & _
      "err.windescription:" & @TAB & $oMyError.windescription & @CRLF & _
      "err.number is: " & @TAB & $HexNumber & @CRLF & _
      "err.lastdllerror is: " & @TAB & $oMyError.lastdllerror & @CRLF & _
      "err.scriptline is: " & @TAB & $oMyError.scriptline & @CRLF & _
      "err.source is: " & @TAB & $oMyError.source & @CRLF & _
      "err.helpfile is: " & @TAB & $oMyError.helpfile & @CRLF & _
      "err.helpcontext is: " & @TAB & $oMyError.helpcontext _
      )
  SetError(1) ; to check for after this function returns
EndFunc   ;==>MyErrFunc

Func _ReadPort()
  $buf = ""
  $NetComm.Output = "SSSS"
  If $NetComm.InBufferCount > 0 Then
    $buf = $NetComm.InputData
    $buf = StringTrimRight($buf, 2)
  EndIf
  Return $buf
EndFunc   ;==>_ReadPort

Func _ClosePort()
  $NetComm.PortOpen = "False"
  $NetComm = 0
EndFunc   ;==>_ClosePort

Func _GetPort()
  $comport = GUICtrlRead($SerialPort)
  $comport = StringTrimLeft($comport, 3)
  Return $comport
EndFunc   ;==>_GetPort

Func _ShowTemp($tempval)
  If $tempval == "" Then
    Return
  EndIf
  If $tempval < $minTemp Then
    $minTemp = $tempval
  EndIf
  If $tempval > $maxTemp Then
    $maxTemp = $tempval
  EndIf
  $minTemp = StringFormat("%.1f", $minTemp)
  $maxTemp = StringFormat("%.1f", $maxTemp)
  GUICtrlSetData($Min, $minTemp)
  GUICtrlSetData($Max, $maxTemp)
  FileWriteLine($file, _NowTime(5) & "," & $buf)
  GUICtrlCreateListViewItem(_NowCalc() & " |  " & $tempval, $TempLV)
  GUICtrlSendMsg($TempLV, $LVM_ENSUREVISIBLE, $counter, 0)
  $counter += 1
  GUICtrlSetData($Temp, $tempval)
  $sampval += 1
  GUICtrlSetData($Samples, $sampval)
  $totTemp += $tempval
  $avgTemp = $totTemp / $sampval
  $avgTemp = StringFormat("%.1f", $avgTemp)
  GUICtrlSetData($Avg, $avgTemp)
EndFunc   ;==>_ShowTemp

Func _ShowTimer(ByRef $timer, $stime)
  Dim $elapsedTime
  
  If TimerDiff($timer) >= 1000 Then
    GUICtrlSetData($Time, _NowTime(5))
    _TicksToTime(Int(TimerDiff($stime)), $hh, $mm, $ss)
    $elapsedTime = StringFormat("%02i:%02i:%02i", $hh, $mm, $ss)
    GUICtrlSetData($Elapsed, $elapsedTime)
    $timer = TimerInit()
  EndIf
EndFunc   ;==>_ShowTimer

Func _ComputerGetSerialPorts(ByRef $aSerialPorts)
  Local $HKLMSerialPorts = "HKLM\HARDWARE\DEVICEMAP\SERIALCOMM"
  Local $RegistryKey
  Local $RegistryVal
  Dim $aSerialPorts[1], $i = 1
  
  While 1
    $RegistryKey = RegEnumVal($HKLMSerialPorts, $i)
    $RegistryVal = RegRead($HKLMSerialPorts, $RegistryKey)
    If @error <> 0 Then ExitLoop
    ReDim $aSerialPorts[UBound($aSerialPorts) + 1]
    $aSerialPorts[$i] = $RegistryVal
    $i += 1
  WEnd
  
  $aSerialPorts[0] = UBound($aSerialPorts) - 1
  If $aSerialPorts[0] < 1 Then
    SetError(1, 1, 0)
  EndIf
EndFunc   ;==>_ComputerGetSerialPorts

; == End User Code ==