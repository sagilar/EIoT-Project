bool readFireDetector();
void readCurrentSensor();
void calculateCurrent();
float readMeanCurrent();
float readMeanPower();
float readMaxPower();
double filter2o(double u, double &v1, double &v2, const double a0, const double a1, const double a2, const double b0, const double b1, const double b2);