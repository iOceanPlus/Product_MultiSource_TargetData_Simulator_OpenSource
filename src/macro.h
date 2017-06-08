#ifndef MACRO
#define MACRO

#include "QObject"
#include "CommonEnum.pb.h"

extern uint ExternV_TargetCount;

const quint32 GRID_ARRAY_ROW_COUNT = 180; //How many rows the grid array has
extern bool ExternV_IS_DEBUG_MODE;
extern uint SOGX10_LOWER_THRESH;

const PB_Enum_Software SOFTWARE_NAME=EV_Software_NA;
const bool IS_INTLONGITUDE_0_VALID =false; //If 0 is a valid coordinate

const quint32 MAX_UINT32=4294967295;
const int MIN_INT32= -2147483648;
const int MAX_INT32= 2147483647;

//AIS消息中的经纬度需要除以这个数得到度数
const double AISPosDivider =600000.0;
const double  NM_In_Meter= 1852.0 ;//一海里多少米

/********************
 * routing keys of rabbitmq
 * ***************************/
const QString ROUTING_KEY_MONITOR_PROBEACK ="Monitor.ProbeAck";
const QString ROUTING_KEY_MONITOR_ALIVETARGETCOUNT ="Monitor.AliveTargetCount";
const QString ROUTING_KEY_FUSEDDATA_TARGETTRACKS = "OnLine.FusedData.TargetTracks";

const QString ROUTING_KEY_ONLINE_PREPROCESSEDDATA ="OnLine.PreprocessedData.#";
const QString ROUTING_KEY_MONITOR_RPOBE ="Monitor.Probe";

/*******定时检查的相关参数**************/
extern uint ExternV_SECONDS_CHECK_TARGET_COUNT; //定时检查目标总数，并输出

#endif // MACRO

