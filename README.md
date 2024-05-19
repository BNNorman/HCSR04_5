# HCSR04_5
A C shared library for driving HSR04/5 Ultrasonic distance sensors

The library loops triggering the sensor and measuring the echo in a posix thread.

If you have more than one sensor then just copy and rename the library. This allows you to specify the Trigger and Echo pins for each.

For example, after building I renamed the library as libDistanceSensor.so. My robot had a front and rear sensor so I made two copies called Sonar1.so and Sonar2.so

To use them in Python:-

```
frontSonar=cdll.LoadLibrary("./Sonar1.so")
rearSonar=cdll.LoadLibrary("./Sonar2.so")
frontSonar.setupWiringPi() # only call this once as it is system wide
frontSonar.Init(frontTrig,frontEcho,echoSense)
rearSonar.Init(rearTrig,rearEcho,echoSense)
frontSonar.Start()
rearSonar.Start()
```

to read the sensor distances (cm):-

```
frontDist=c_float()
frontSonar.Distance(byref(frontDist))

rearDist=c_float()
rearSonar.Distance(byref(rearDist))

```
