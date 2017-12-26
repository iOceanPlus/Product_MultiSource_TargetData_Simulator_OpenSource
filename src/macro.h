#ifndef MACRO
#define MACRO

#include "QObject"
#include "CommonEnum.pb.h"
#include "PosDevice.h"
#include "Monitor.pb.h"

//#define DEBUG_PERFORMANCE
//#define DEBUG_TargetCount
//#define DEBUG_TARGETTYPE_ANDNAME
#define DEBUG_MOTION

/**Each data source may recode targetIDOrigs, for example: mmsi of a target in data from different sources may be different**/
//#define DATA_SOURCE_RECODE_TARGETIDORIG  //not work, discard

extern uint ExternV_TargetCount;

const quint32 DegreesX10_ToTurn_WhenMeetLand=450;
extern quint16 ExternV_MIN_Sample_MSEC; // Min interval of targets' update of positions, overwriten by param.json
extern uint ExternV_Milliseconds_FetchData;

const quint32 GRID_ARRAY_ROW_COUNT = 180; //How many rows the grid array has
extern bool externVIsWater[GRID_ARRAY_ROW_COUNT][2*GRID_ARRAY_ROW_COUNT]; //water grids
extern QMap <PB_Enum_TargetInfo_Type, Struct_PosDeviceInfo> mapInfoTypePosDeviceInfo;
extern PBMonitor_ProbeAck monitor_ProbeAck;

extern bool ExternV_IS_DEBUG_MODE;
extern uint ExternV_SOGX10_LOWER_THRESH;
extern uint ExternV_SOGX10_UPPER_THRESH;

//extern double ExternV_LATITUDE_LOWER_THRESH_DEGREE;
//extern double ExternV_LATITUDE_UPPER_THRESH_DEGREE;
//extern double ExternV_LONGITUDE_LOWER_THRESH_DEGREE;
//extern double ExternV_LONGITUDE_UPPER_THRESH_DEGREE;

const PB_Enum_Software SOFTWARE_NAME=EV_Software_TargetDataSimulator ;
const bool IS_INTLONGITUDE_0_VALID =false; //If 0 is a valid coordinate

const quint32 MAX_UINT32=4294967295;
const int MIN_INT32= -2147483648;
const int MAX_INT32= 2147483647;

//AIS消息中的经纬度需要除以这个数得到度数
const double AISPosDivider =600000.0;
const double  NM_In_Meter= 1852.0 ;//一海里多少米
const double KNOT_IN_METER_PER_SECOND= NM_In_Meter/3600.0;

const QString ROUTING_KEY_MONITOR_RPOBE ="Monitor.Probe";

/*******定时检查的相关参数**************/
extern uint ExternV_SECONDS_CHECK_TARGET_COUNT; //定时检查目标总数，并输出

const double ACCEL_IN_METERS_PER_SECOND = 0.5/60.0 *KNOT_IN_METER_PER_SECOND; //accel: 0.5 knot per minute
const double APPROACHING__SPEED_IN_KNOTX10=8; //Approach boundary in this speed
const double APPROACHING_SPEED_IN_METERS_PER_SECOND=APPROACHING__SPEED_IN_KNOTX10/10.0*KNOT_IN_METER_PER_SECOND; //Approach boundary in this speed

#endif // MACRO

