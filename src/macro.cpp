#include <QObject>
#include <QDebug>
bool  ExternV_IS_DEBUG_MODE=false;

uint ExternV_TargetCount; //see param.json

uint ExternV_SECONDS_CHECK_TARGET_COUNT= 20; //Output target count periodically
uint ExternV_Milliseconds_FetchData = 4000;

uint ExternV_SOGX10_LOWER_THRESH; //Only simulate targets with SOGX10 no less than this.see param.json
uint ExternV_SOGX10_UPPER_THRESH; //Only simulate targets with SOGX10 no larger than this. see param.json

double ExternV_LATITUDE_LOWER_THRESH_DEGREE; //see param.json
double ExternV_LATITUDE_UPPER_THRESH_DEGREE;//see param.json
double ExternV_LONGITUDE_LOWER_THRESH_DEGREE;//see param.json
double ExternV_LONGITUDE_UPPER_THRESH_DEGREE;//see param.json
