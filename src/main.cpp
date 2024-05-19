// distance sensor library
// meant to be called from Python
// using C/C++ tends to give more consistent values then using python interrupts or polling loops
//
// uses a posix thread which runs in a continuous loop triggering the sensor and polling the echo pin
//
// useage:-
// import SensorLib
// r=SensorLib.init(trigPin,echoPin, echoPulse) where echPulse is HIGH or LOW
// if r!=0:
//  print "WiringPi GPIO setup failed errno=",r
//  probably abort here

// to start the sensor sensing :-
// SensorLib.Start()
//
// to stop the sensor :-
// SensorLib.Stop()
//
// to get the last measured distance :-
// NOTE: measurements are made at a rate of 40Hz (limit of the sensor)
// d=SensorLib.Distance()



extern "C"
{
    #define FREQUENCY 40

    // A function adding two integers and returning the result
    #include <stdio.h>
    #include <wiringPi.h>
    #include <pthread.h>
    #include <unistd.h>


    int stop;
    int _echoPin;
    int _trigPin;
    int _echoPulse;
    float pulsePeriod=1000/FREQUENCY;  // millisec
    bool debug=false;
    float distance=0;

    void *sensor(void *arg)
    {
        int echoStart;
        int echoEnd;
        int timeout;
        int p1;
        int p2;
        bool skip;


        while(!stop)

        {
        // skip is set if the echo pulse times out
        // we live to scan another time

        skip=false;
        p1=millis(); // used to keep loop time as constant as possible
        // trigger the sensor (on first pass init sets this low
        //printf("sending trigger\n");
        digitalWrite(_trigPin,HIGH);
        delayMicroseconds(20);          // - see sensor spec, minimum of 10us
        digitalWrite(_trigPin,LOW);

        // wait for an echo
        timeout=millis();
        while(digitalRead(_echoPin)!=_echoPulse)
            {
                if ((millis()-timeout)>=pulsePeriod)  // 1 sec
                {
                    skip=true;
                    if (debug){printf("Echo start of pulse not seen on pin %d\n",_echoPin);}
                    break;
                }
            }

        if (!skip)
            {
            echoStart=micros();
            //printf("got echo");
            // now wait for the echo pulse to go away
            timeout=millis();
            while(digitalRead(_echoPin)==_echoPulse)
                {
                if ((millis()-timeout)>=pulsePeriod) //
                    {
                    if (debug) {printf("Echo end of pulse not seen on pin %d\n",_echoPin);}
                    skip=true;
                    break;
                    }
                }

            // echo pulse leading and trailing edges were seen
            // so calculate the distance
            if (!skip)
                {
                echoEnd=micros();
                // speed of sound in cm/s - pulse travels there and back hence divide by 2
                distance=34029*(echoEnd-echoStart)/2000000;

                if (debug) { printf("Distance= %f on pin%d\n",distance,_echoPin);}
                }


            }

        // we need to delay before sending the next pulse
        // allow for time taken in the loop up to this point
        p2=millis();
        delay(pulsePeriod-(p2-p1));

        }

        return(0);  // compiler complained
    }

    void setDebug(bool mode)
    {
        debug=mode;
    }

    // setupWiringPi()
    // generates an error is called twice so if the library is used
    // for multiple sensors this routing must be called once only before Init()
    bool setupWiringPi()
    {
       int r;

        r=wiringPiSetupGpio();

        if (r==-1)
        {
        if (debug) {printf("WiringPi GPIO setup failed\n");}
        return false;
        }

        return true;
    }
    // Init() initialises the GPIO returns errno if that fails
    // echoPulse sense 1=HIGH,0=LOW

    int Init(int trigPin, int echoPin,int echoPulse)
    {

        // this method configures the GPIO pins
        _echoPin=echoPin;
        _trigPin=trigPin;
        _echoPulse=echoPulse;

        if (debug) {printf("trigPin=%d echoPin=%d pulse=%d\n",trigPin,echoPin,echoPulse);}



        // assume library user has setup the GPIO
        pinMode(trigPin,OUTPUT);
        pinMode(echoPin,INPUT);
        digitalWrite(trigPin,LOW);  // sensor() takes this high then low again

        // echo pin is driven from the output of a schmitt
        // not sure if pullup/down is needed
        pullUpDnControl(echoPin,PUD_OFF);


        return 0;   // all ok
    }

    // Start gets the sensor() routine running
    void Start()
    {
        pthread_t pth;
        // insert code here
        stop=false;

        pthread_create(&pth,NULL,sensor,NULL);
    }

    // Stop() causes the sensor() routine to terminate
    void Stop()
    {
        // the sensor thread will terminateon the next pulse
        stop=true;
    }

    // get the last distance calculated - doesn't appear to work in the python caller
    void Distance(float* d)
    {
        if (debug){printf("Call to Distance returns %f for pin %d\n",distance,_echoPin);}
        *d=distance;
    }
}
