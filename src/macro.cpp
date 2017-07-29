#include <QObject>
#include <QDebug>
bool  ExternV_IS_DEBUG_MODE=false;

uint ExternV_TargetCount= 40000;

uint ExternV_SECONDS_CHECK_TARGET_COUNT= 20; //Output target count periodically
uint ExternV_Milliseconds_FetchData = 4000;

uint ExternV_SOGX10_LOWER_THRESH=30; //Only simulate targets with SOGX10 no less than this
uint ExternV_SOGX10_UPPER_THRESH=800; //Only simulate targets with SOGX10 no larger than this

double ExternV_LATITUDE_LOWER_THRESH_DEGREE=30.0;
double ExternV_LATITUDE_UPPER_THRESH_DEGREE=40.0;
double ExternV_LONGITUDE_LOWER_THRESH_DEGREE=110.0;
double ExternV_LONGITUDE_UPPER_THRESH_DEGREE=120.0;
