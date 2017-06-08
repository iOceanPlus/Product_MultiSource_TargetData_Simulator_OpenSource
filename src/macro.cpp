#include <QObject>
#include <QDebug>
bool  ExternV_IS_DEBUG_MODE=false;

uint ExternV_TargetCount= 30000;

uint ExternV_SECONDS_CHECK_TARGET_COUNT= 20; //Output target count periodically

uint SOGX10_LOWER_THRESH=30; //Only simulate targets with SOGX10 no less than this

