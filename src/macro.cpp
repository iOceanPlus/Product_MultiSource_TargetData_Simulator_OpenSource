#include <QObject>
#include <QDebug>
#include "macro.h"

bool  ExternV_IS_DEBUG_MODE; //see param.json

uint ExternV_TargetCount; //see param.json

uint ExternV_SECONDS_CHECK_TARGET_COUNT; //Output target count periodically, see param.json
uint ExternV_Milliseconds_FetchData = 1000; //overwriten by param.json, interval to fetch data from channels
quint16 ExternV_MIN_Sample_MSEC=1000; // Min interval of targets' update of positions, overwriten by param.json

uint ExternV_SOGX10_LOWER_THRESH; //Only simulate targets with SOGX10 no less than this.see param.json
uint ExternV_SOGX10_UPPER_THRESH; //Only simulate targets with SOGX10 no larger than this. see param.json

//double ExternV_LATITUDE_LOWER_THRESH_DEGREE; //see param.json
//double ExternV_LATITUDE_UPPER_THRESH_DEGREE;//see param.json
//double ExternV_LONGITUDE_LOWER_THRESH_DEGREE;//see param.json
//double ExternV_LONGITUDE_UPPER_THRESH_DEGREE;//see param.json

bool externVIsWater[GRID_ARRAY_ROW_COUNT][2*GRID_ARRAY_ROW_COUNT]; //water grids
QMap <PB_Enum_TargetInfo_Type, Struct_PosDeviceInfo> mapInfoTypePosDeviceInfo;
PBMonitor_ProbeAck monitor_ProbeAck;


