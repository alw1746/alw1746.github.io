'TempSensor.bas - Measure temperature by using a PICAXE-08M to read the voltage from
'an LM335 sensor. The results are converted to Centigrade(ASCII) and sent to
'PC through an RS232 connection for display. The PC polls for values by sending the
'string SSSS to the PICAXE at regular intervals.
'COM port settings - 2400,N,8,1

   Symbol ADVal=W0                        'voltage from LM335
   Symbol Sum=W1                          'total readings
   Symbol SensorOutput=W1                 'LM335 output voltage
   Symbol Fraction=W1                     'eg: 8 in 10.8
   Symbol Whole=W2                        'eg: 10 in 10.8 
   Symbol factor=W4                       'Kelvin to Centigrade conversion factor
   symbol loop=b10                        'loop counter

   factor=273                             '273k = 0c
main:
   serin 2,n2400,b1,b2,b3,b4              'Wait for poll string from PC
   if b1 = "S" then tempout               'got SSSS? send temperature
   if b1 = "C" then calibrate             'got Cnnn? calibrate factor
   goto main                              'ignore anything else

tempout:
   Sum = 0
   For loop = 1 to 64                     ' sum 64 readings
      ReadADC10 1, ADVal
      Sum = Sum + ADVal
   Next
   ADVal = Sum / 64                       ' calculate the average

'The ADC has a resolution of 10 bits ie. it will return a value between 0 and 1023 over
'the 5V power supply range ie. each value represents 5/1024 = 4.88 mV. The LM335 outputs
'10mV/Kelvin rise hence - temp in Kelvin=total mV/10.
'Note: PICAXE does not support floating point maths hence, in a multiply calculation, the
'integer and fractions must be separately added together.

   SensorOutput = ADVal * 4                       'total mV=4.88 * ADVal
   SensorOutput = ADval * 8 / 10 + SensorOutput   'add fraction
   SensorOutput = ADval * 8 / 100 + SensorOutput  'LM335 output in mV
   Whole = SensorOutput / 10                      'get Kelvin (10mV/Kelvin)
   Whole=Whole-factor                             'get Centigrade (273k = 0c)
   Fraction = SensorOutput % 10                   'fraction=10th of degree

   SEROUT 4,n2400,(#Whole, ".", #Fraction, 13,10)   'send ASCII temp to PC

   goto main                              'wait for next poll

calibrate:      'change value of factor. Useful if battery runs low.
   factor=b2-48 *100                      'eg: ascii 216 -> bin 200
   factor=b3-48 *10 +factor               '200 + bin 10
   factor=b4-48 +factor                   '210 + bin 6 = 216
   goto main 
