//
// Created by aryan on 23-01-2021.
//

/* A simplified one dimensional Kalman filter implementation
*/

#ifndef _Kalman_h
#define _Kalman_h

class Kalman {
private:
    /* Kalman filter variables */
    double q; //process noise covariance
    double r; //measurement noise covariance
    double x; //value
    double p; //estimation error covariance
    double k; //kalman gain

public:
    Kalman(double process_noise, double sensor_noise, double estimated_error, double intial_value) {
        /* The variables are x for the filtered value, q for the process noise,
           r for the sensor noise, p for the estimated error and k for the Kalman Gain.
           The state of the filter is defined by the values of these variables.

           The initial values for p is not very important since it is adjusted
           during the process. It must be just high enough to narrow down.
           The initial value for the readout is also not very important, since
           it is updated during the process.
           But tweaking the values for the process noise and sensor noise
           is essential to get clear readouts.

           For large noise reduction, we can try to start from
           q = 0.125
           r = 32
           p = 1023 //"large enough to narrow down"
           e.g.
           myVar = Kalman(0.125,32,1023,0);
        */
        this->q = process_noise;
        this->r = sensor_noise;
        this->p = estimated_error;
        this->x = intial_value; //x will hold the iterated filtered value
    }

    double getFilteredValue(double measurement) {
        /* Updates and gets the current measurement value */
        //prediction update
        //omit x = x
        this->p = this->p + this->q;

        //measurement update
        this->k = this->p / (this->p + this->r);
        this->x = this->x + this->k * (measurement - this->x);
        this->p = (1 - this->k) * this->p;

        return this->x;
    }

    void setParameters(double process_noise, double sensor_noise, double estimated_error) {
        this->q = process_noise;
        this->r = sensor_noise;
        this->p = estimated_error;
    }

    void setParameters(double process_noise, double sensor_noise) {
        this->q = process_noise;
        this->r = sensor_noise;
    }

    double getProcessNoise() {
        return this->q;
    }

    double getSensorNoise() {
        return this->r;
    }

    double getEstimatedError() {
        return this->p;
    }
};
/*
How can we merge with the other files:

#include <Kalman.h>

//Creates the variables:
double measurement, filteredMeasurement;
Kalman myFilter(0.125,32,1023,0); //suggested initial values for high noise filtering

void loop() {

  //...

  //reads measurement and filter it
  measurement = (double) analogRead(A1); //read new value from sensor in analog pin A1, for example
  filteredMeasurement = myFilter.getFilteredValue(measurement);

  //...
}

Some other useful stuff:
//Reset filter parameters:
void setParameters(double process_noise, double sensor_noise, double estimated_error)

//Get filter parameters:
double getProcessNoise()
double getSensorNoise()
double getEstimatedError()
*/
#endif



