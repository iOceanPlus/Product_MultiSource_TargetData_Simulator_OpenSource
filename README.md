### 注意！该项目为开源项目，请勿将账号、消息队列信息、敏感信息等传入代码或附件！   
Notice! This project is open source, DO NOT PUSH personal or sensitive contents, such as accounts of rabbitmq.
  
# ICTShips: Multisource Target Data Simulator

## Introduction  
This project simulates targets in a specified area, several devices are installed in each target. These devices get kinematic information about targets and send to channels. Data sources fetech data from channels periodically.  
Messages are encoded using Google's protobuf, and sent to a rabbitmq server.   
Targets move with constant speed in Great Circle Route until they are going to meed boundary (land or specified area). When they are going to meed boundary, targets move with constant acceleration until the speed drop to zero and then they change direction to find a way into water again. If targets fail to find navigable routes, they will stop.

Other softwares can fetch data generated by this software from rabbitmq. An introduction of rabbitmq can be found in:  
http://www.rabbitmq.com/#getstarted  
 
We have implemented some modules and demos:  
http://git.oschina.net/iOceanPlus/JavaRabbitMQ  
http://git.oschina.net/iOceanPlus/JavaProtoBufDemo  
http://git.oschina.net/iOceanPlus/PythonRabbitMQDemo  
http://git.oschina.net/iOceanPlus/PythonProtobufDemo  

If your program do not support rabbitmq (such as MatLab), use a MQ-UDP adapter as a bridge.  

The architecture of fusion systems based on rabbitmq is in the following figure.

![How to interact with simulator](https://gitee.com/uploads/images/2017/1121/120341_2ccad847_854788.png "system.png")

There are multiple threads in simulator, whose count can be configured in param.json. In each thread, there are multiple data sources as in the figure below.
![Data Smulator Architecture](https://gitee.com/uploads/images/2017/1121/153109_4ae7d048_854788.png "屏幕截图.png")

The motion model of targets are in the figure below:
![输入图片说明](https://gitee.com/uploads/images/2017/1225/171943_e843a7d3_854788.jpeg "target motion model.jpg")

## Messages  
Message trasmitted: PBTarget encoded by protobuf. See the Protobuf_Files directory. Details about protobuf can be found in:    
https://developers.google.com/protocol-buffers/  

Use the topic mode of rabbitmq (see http://www.rabbitmq.com/tutorials/tutorial-five-python.html). The routing keys published are:  
OnLine.PreprocessedData.AIS  
OnLine.PreprocessedData.BeiDou  
OnLine.PreprocessedData.LRIT  
OnLine.PreprocessedData.HaiJian  
OnLine.PreprocessedData.Argos  

You can subscribe OnLine.PreprocessedData.# to receive all of them.

The settings of queue:  
```
arguments["x-message-ttl"]=300000;
arguments["x-expires"]=300000;
channel->declareQueue(AMQP::durable,arguments);
channel->consume(name,AMQP::exclusive+AMQP::noack+AMQP::nolocal);
```


## Dataset used for initialization  
- Dataset used for initialization  
ais_ship_realtime_valid_anonymous_randOrder.csv：https://drive.google.com/open?id=0B01TUPO5dae2VTZGblI3a1pCazA  
OR ais_ship_realtime_valid_randomOrder.csv: add more columns such as shipNames into ais_ship_realtime_valid_anonymous_randOrder.csv

- Dataset used for water and land detection  
Waters_1_Degree_2014.csv：https://drive.google.com/open?id=0B01TUPO5dae2U0ItUk5MaDRnSlU  
For details about the content of this file refer to： Wu L, Xu Y, Wang Q, et al. Mapping Global Shipping Density from AIS Data[J]. The Journal of Navigation, 2017, 70(1): 67-81.

Put these datasets with the executable file.

## Environment  
Linux, g++ 4.7 or above is needed.  
The program needs to connect to a rabbitmq server, make sure that rabbitmq is installed in a server that this program can connect to.

## Libs needed  
- libev  

- protobuf：Google's data interchange format  
https://github.com/google/protobuf  

- AMQP-CPP：C++ library for asynchronous non-blocking communication with RabbitMQ  
https://github.com/CopernicaMarketingSoftware/AMQP-CPP  

- Qt5  
Module: core  positioning 

## How to install libs and run the software  
- Install libs needed  
Download libs needed from https://www.jianguoyun.com/p/DWhKo_YQwZOqBhi1ojQ   
cd the CentOSLibs directory  
cd each directory under CentOSLibs and run the scripts: ```sudo sh ./DeployLibs.sh ```  
If error occured, modify DeployLibs.sh according to comments in the file.  
Create a directory sqldrivers under the directory that contains the executable file, put the file libqsqlmysql.so into sqldrivers, which is contained in the CentOSLibs directory.  

- Check if the rabbitmq server is runing  
- Download the executable file, param_mq.txt and param.json in the attachments of this project   
- Update the parameters in the param_mq.txt and param.json  
- Grant rights: ``` sudo chmod +rx ./TargetDataSimulator  ```
- check if the software is already runing:```ps -ef |grep Simu```  
- Run the software： ```nohup ./TargetDataSimulator &```  
- check the status  
```top```，see the resource consumed by TargetDataSimulator  
```vi nohup.txt```，inspect the output log  

## Typical Scenes  
Under these scences, only ships with SOG from 3 knots to 80 knots are present.  
Three types of data from three sources are used:  
- AIS from Maritime Administration  
TargetInfo_Type id is 4, targetInfo_Source id is 2  
- Beidou from Agriculture Ministry   
TargetInfo_Type id is 2, targetInfo_Source id is 1  
- Radar from Radar networks    
TargetInfo_Type id is 7, targetInfo_Source id is 3  

### DenseShip42: HuangPu River in Shanghai, China  
This is one of the busiest water ways in the world, see below:  

![42 targets in Huangpu river](https://git.oschina.net/uploads/images/2017/0929/115344_4942a03c_854788.png "屏幕截图.png")

Ships are in a polygon: (31.3870 121.5021,31.3742 121.4960,31.3733 121.5005,31.3857 121.5065,31.3870 121.5021)
This polygon is 1.3 kilometers long and about 200 meters wide on average. It can be calculated that there are one target on every 6190 square meters, average distance between ships is 78.7 meters on average.  

To configure DenseShip42, copy below to param.json:  
```JSON
{
    "ISDebugMode":"TRUE",
    "Comment of TargetCount":"The count of targets trying to simulate, if there are not so many targets in the region, the number actually created will be less than this, see the output log to check target count",
    "TargetCount": 20000,
    "World_Threads_Count":1,

    "Comment of ExternV_MIN_Sample_MSEC":"A target will not update its kinematic states if time elapsed since last update is less than this value",
    "ExternV_MIN_Sample_MSEC":1000,

    "Comment of ExternV_Milliseconds_FetchData":"How often positioning devices fetch data from targets.",
    "ExternV_Milliseconds_FetchData":1000,

    "SECONDS_CHECK_TARGET_COUNT" : 15,
    "SOGX10_LOWER_THRESH":30,
    "SOGX10_UPPER_THRESH":800,
    "Comment of Bounding_Region":"Bound all the targets in this polygon region, format is: lat1 lng1, lat2 lng2, lat3 lng3, ..., latk lngk, lat1 lng1. If Bounding is not required, delete Bounding_Region",
    "Bounding_Region":"31.3870 121.5021,31.3742 121.4960,31.3733 121.5005,31.3857 121.5065,31.3870 121.5021",
    "WaterGridsFileName":"Waters_1_Degree_2014.csv",
    "Comment of Ship_FileName":"Alternative of Ship_FileName is ais_ship_realtime_valid_anonymous_randOrder.csv: ",
    "Ship_FileName":"ais_ship_realtime_valid_anonymous_randomOrder.csv",
    "Comment of MC2_FileName":"If no file exits, delete the MC2_FileName Line.",
    "MC2_FileName":"mc2_mmsiToCountryCNENAndCode.csv",
    "Language":"EN",
    "Comment of PosDevice":"Define features of devices like AIS, Beidou, etc.",
    "PosDevice":
    [
        {
            "TargetInfo_Type":2,
            "Comment of PositioningDevInMeters":"Std dev of the gaussian distribution",
            "PositioningDevInMeters":15.0,
            "SOGDevInKnots":0.5,
            "COGDevInDegrees":5.0,
            "SampleMilliSeconds":20000
        },
        {
            "TargetInfo_Type":4,
            "PositioningDevInMeters":15.0,
            "SOGDevInKnots":0.5,
            "COGDevInDegrees":5.0,
            "SampleMilliSeconds":12000
        },
        {
            "TargetInfo_Type":7,
            "PositioningDevInMeters":100,
            "SOGDevInKnots":0.5,
            "COGDevInDegrees":5.0,
            "SampleMilliSeconds":2000
        }
    ],

    "Comment":"One data source may contain multiple data channels, each channel fetch data from one type of data.",
    "CommentOfSourceAndInfoType":"Refer to CommonEnum for the encoding",
    "DataSources":
    [
        {
            "TargetInfo_Source":2,
            "SourceInfo":
            [
                {
                    "TargetInfo_Type":4,
                    "ObservePercentage":90,
                    "meanTransmissionLatencyInMiliSeconds":0,
                    "stdDevTransmissionLatencyInMiliSeconds":0,
                    "meanTimestampErrorInMiliSeconds":0,
                    "stdDevTimestampErrorInMiliSeconds":0,
                    "packetLossPercentage":10
                }
            ]
        },


        {
            "TargetInfo_Source":1,
            "SourceInfo":
            [
                {
                    "TargetInfo_Type":2,
                    "ObservePercentage":55,
                    "meanTransmissionLatencyInMiliSeconds":0,
                    "stdDevTransmissionLatencyInMiliSeconds":0,
                    "meanTimestampErrorInMiliSeconds":0,
                    "stdDevTimestampErrorInMiliSeconds":0,
                    "packetLossPercentage":10
                }
            ]
        },

        {
            "TargetInfo_Source":3,
            "SourceInfo":
            [
                {
                    "TargetInfo_Type":7,
                    "ObservePercentage":100,
                    "meanTransmissionLatencyInMiliSeconds":0,
                    "stdDevTransmissionLatencyInMiliSeconds":0,
                    "meanTimestampErrorInMiliSeconds":0,
                    "stdDevTimestampErrorInMiliSeconds":0,
                    "packetLossPercentage":0
                }
            ]
        }
    ]
}
```


### SparseShip45: Outside Shanghai, China  
See the figure below:  
![sparse scene](https://git.oschina.net/uploads/images/2017/0929/121205_bbfc5624_854788.png "屏幕截图.png") 

Ships are in a polygon: (31.0017 123.4317, 31.0017 124.7267,31.8283 124.7267,31.8283 123.4317,31.0017 123.4317)
 This section is 120 kilometers long and 92 kilometers wide. There is one target on every 245 square kilometers, average distance between ships is 15.66 kilometers.  

To configure SparseShip45, copy below to param.json:  
```JSON
{
    "ISDebugMode":"TRUE",
    "Comment of TargetCount":"The count of targets trying to simulate, if there are not so many targets in the region, the number actually created will be less than this, see the output log to check target count",
    "TargetCount": 20000,
    "World_Threads_Count":1,

    "Comment of ExternV_MIN_Sample_MSEC":"A target will not update its kinematic states if time elapsed since last update is less than this value",
    "ExternV_MIN_Sample_MSEC":1000,

    "Comment of ExternV_Milliseconds_FetchData":"How often positioning devices fetch data from targets.",
    "ExternV_Milliseconds_FetchData":1000,

    "SECONDS_CHECK_TARGET_COUNT" : 15,
    "SOGX10_LOWER_THRESH":30,
    "SOGX10_UPPER_THRESH":800,
    "Comment of Bounding_Region":"Bound all the targets in this polygon region, format is: lat1 lng1, lat2 lng2, lat3 lng3, ..., latk lngk, lat1 lng1. If Bounding is not required, delete Bounding_Region",
    "Bounding_Region":"31.0017 123.4317, 31.0017 124.7267,31.8283 124.7267,31.8283 123.4317,31.0017 123.4317",
    "WaterGridsFileName":"Waters_1_Degree_2014.csv",
    "Comment of Ship_FileName":"Alternative of Ship_FileName is ais_ship_realtime_valid_anonymous_randOrder.csv: ",
    "Ship_FileName":"ais_ship_realtime_valid_anonymous_randomOrder.csv",
    "Comment of MC2_FileName":"If no file exits, delete the MC2_FileName Line.",
    "MC2_FileName":"mc2_mmsiToCountryCNENAndCode.csv",
    "Language":"EN",
    "Comment of PosDevice":"Define features of devices like AIS, Beidou, etc.",
    "PosDevice":
    [
        {
            "TargetInfo_Type":2,
            "Comment of PositioningDevInMeters":"Std dev of the gaussian distribution",
            "PositioningDevInMeters":15.0,
            "SOGDevInKnots":0.5,
            "COGDevInDegrees":5.0,
            "SampleMilliSeconds":20000
        },
        {
            "TargetInfo_Type":4,
            "PositioningDevInMeters":15.0,
            "SOGDevInKnots":0.5,
            "COGDevInDegrees":5.0,
            "SampleMilliSeconds":12000
        },
        {
            "TargetInfo_Type":7,
            "PositioningDevInMeters":500,
            "SOGDevInKnots":0.5,
            "COGDevInDegrees":5.0,
            "SampleMilliSeconds":2000
        }
    ],

    "Comment":"One data source may contain multiple data channels, each channel fetch data from one type of data.",
    "CommentOfSourceAndInfoType":"Refer to CommonEnum for the encoding",
    "DataSources":
    [
        {
            "TargetInfo_Source":2,
            "SourceInfo":
            [
                {
                    "TargetInfo_Type":4,
                    "ObservePercentage":90,
                    "meanTransmissionLatencyInMiliSeconds":0,
                    "stdDevTransmissionLatencyInMiliSeconds":0,
                    "meanTimestampErrorInMiliSeconds":0,
                    "stdDevTimestampErrorInMiliSeconds":0,
                    "packetLossPercentage":10
                }
            ]
        },


        {
            "TargetInfo_Source":1,
            "SourceInfo":
            [
                {
                    "TargetInfo_Type":2,
                    "ObservePercentage":55,
                    "meanTransmissionLatencyInMiliSeconds":0,
                    "stdDevTransmissionLatencyInMiliSeconds":0,
                    "meanTimestampErrorInMiliSeconds":0,
                    "stdDevTimestampErrorInMiliSeconds":0,
                    "packetLossPercentage":10
                }
            ]
        },

        {
            "TargetInfo_Source":3,
            "SourceInfo":
            [
                {
                    "TargetInfo_Type":7,
                    "ObservePercentage":100,
                    "meanTransmissionLatencyInMiliSeconds":0,
                    "stdDevTransmissionLatencyInMiliSeconds":0,
                    "meanTimestampErrorInMiliSeconds":0,
                    "stdDevTimestampErrorInMiliSeconds":0,
                    "packetLossPercentage":0
                }
            ]
        }
    ]
}
```


### RegularShip32:  TangShan, China

32 Ships are under this scence.

![RegularShip32](https://git.oschina.net/uploads/images/2017/0929/123032_c1f1fa4b_854788.png "屏幕截图.png")

Ships are in a polygon: (38.92 118.3367,38.92 118.4183,38.9633 118.4183,38.9633 118.3367,38.92 118.3367)
This scene is 7.07 kilometer long and 4.82 kilometers wide. There is one target on every 1.06 square kilometers, average distance between ships is 1 kilometer.  

To configure RegularShip32, copy below to param.json:  
```JSON
{
    "ISDebugMode":"TRUE",
    "Comment of TargetCount":"The count of targets trying to simulate, if there are not so many targets in the region, the number actually created will be less than this, see the output log to check target count",
    "TargetCount": 20000,
    "World_Threads_Count":1,

    "Comment of ExternV_MIN_Sample_MSEC":"A target will not update its kinematic states if time elapsed since last update is less than this value",
    "ExternV_MIN_Sample_MSEC":1000,

    "Comment of ExternV_Milliseconds_FetchData":"How often positioning devices fetch data from targets.",
    "ExternV_Milliseconds_FetchData":1000,

    "SECONDS_CHECK_TARGET_COUNT" : 15,
    "SOGX10_LOWER_THRESH":30,
    "SOGX10_UPPER_THRESH":800,
    "Comment of Bounding_Region":"Bound all the targets in this polygon region, format is: lat1 lng1, lat2 lng2, lat3 lng3, ..., latk lngk, lat1 lng1. If Bounding is not required, delete Bounding_Region",
    "Bounding_Region":"38.92 118.3367,38.92 118.4183,38.9633 118.4183,38.9633 118.3367,38.92 118.3367",
    "WaterGridsFileName":"Waters_1_Degree_2014.csv",
    "Comment of Ship_FileName":"Alternative of Ship_FileName is ais_ship_realtime_valid_anonymous_randOrder.csv: ",
    "Ship_FileName":"ais_ship_realtime_valid_anonymous_randomOrder.csv",
    "Comment of MC2_FileName":"If no file exits, delete the MC2_FileName Line.",
    "MC2_FileName":"mc2_mmsiToCountryCNENAndCode.csv",
    "Language":"EN",
    "Comment of PosDevice":"Define features of devices like AIS, Beidou, etc.",
    "PosDevice":
    [
        {
            "TargetInfo_Type":2,
            "Comment of PositioningDevInMeters":"Std dev of the gaussian distribution",
            "PositioningDevInMeters":15.0,
            "SOGDevInKnots":0.5,
            "COGDevInDegrees":5.0,
            "SampleMilliSeconds":20000
        },
        {
            "TargetInfo_Type":4,
            "PositioningDevInMeters":15.0,
            "SOGDevInKnots":0.5,
            "COGDevInDegrees":5.0,
            "SampleMilliSeconds":12000
        },
        {
            "TargetInfo_Type":7,
            "PositioningDevInMeters":500,
            "SOGDevInKnots":0.5,
            "COGDevInDegrees":5.0,
            "SampleMilliSeconds":2000
        }
    ],

    "Comment":"One data source may contain multiple data channels, each channel fetch data from one type of data.",
    "CommentOfSourceAndInfoType":"Refer to CommonEnum for the encoding",
    "DataSources":
    [
        {
            "TargetInfo_Source":2,
            "SourceInfo":
            [
                {
                    "TargetInfo_Type":4,
                    "ObservePercentage":90,
                    "meanTransmissionLatencyInMiliSeconds":0,
                    "stdDevTransmissionLatencyInMiliSeconds":0,
                    "meanTimestampErrorInMiliSeconds":0,
                    "stdDevTimestampErrorInMiliSeconds":0,
                    "packetLossPercentage":10
                }
            ]
        },


        {
            "TargetInfo_Source":1,
            "SourceInfo":
            [
                {
                    "TargetInfo_Type":2,
                    "ObservePercentage":55,
                    "meanTransmissionLatencyInMiliSeconds":0,
                    "stdDevTransmissionLatencyInMiliSeconds":0,
                    "meanTimestampErrorInMiliSeconds":0,
                    "stdDevTimestampErrorInMiliSeconds":0,
                    "packetLossPercentage":10
                }
            ]
        },

        {
            "TargetInfo_Source":3,
            "SourceInfo":
            [
                {
                    "TargetInfo_Type":7,
                    "ObservePercentage":100,
                    "meanTransmissionLatencyInMiliSeconds":0,
                    "stdDevTransmissionLatencyInMiliSeconds":0,
                    "meanTimestampErrorInMiliSeconds":0,
                    "stdDevTimestampErrorInMiliSeconds":0,
                    "packetLossPercentage":0
                }
            ]
        }
    ]
}
```


### GlobalShip110K: Global ships  
There are 110415 ships in total under this scene, 102231 of them are observed by sensors.    
![GlobalShip110K](https://git.oschina.net/uploads/images/2017/0929/122329_e3b9dec1_854788.png "屏幕截图.png")  

To configure GlobalShip110K, copy below to param.json:  
```JSON
{
    "ISDebugMode":"TRUE",
    "Comment of TargetCount":"The count of targets trying to simulate, if there are not so many targets in the region, the number actually created will be less than this, see the output log to check target count",
    "TargetCount": 200000,
    "World_Threads_Count":8,

    "Comment of ExternV_MIN_Sample_MSEC":"A target will not update its kinematic states if time elapsed since last update is less than this value",
    "ExternV_MIN_Sample_MSEC":1000,

    "Comment of ExternV_Milliseconds_FetchData":"How often positioning devices fetch data from targets.",
    "ExternV_Milliseconds_FetchData":1000,

    "SECONDS_CHECK_TARGET_COUNT" : 15,
    "SOGX10_LOWER_THRESH":30,
    "SOGX10_UPPER_THRESH":800,
    "Comment of Bounding_Region":"Bound all the targets in this polygon region, format is: lat1 lng1, lat2 lng2, lat3 lng3, ..., latk lngk, lat1 lng1. If Bounding is not required, delete Bounding_Region",
    "WaterGridsFileName":"Waters_1_Degree_2014.csv",
    "Comment of Ship_FileName":"Alternative of Ship_FileName is ais_ship_realtime_valid_anonymous_randOrder.csv: ",
    "Ship_FileName":"ais_ship_realtime_valid_anonymous_randomOrder.csv",
    "Comment of MC2_FileName":"If no file exits, delete the MC2_FileName Line.",
    "MC2_FileName":"mc2_mmsiToCountryCNENAndCode.csv",
    "Language":"EN",
    "Comment of PosDevice":"Define features of devices like AIS, Beidou, etc.",
    "PosDevice":
    [
        {
            "TargetInfo_Type":2,
            "Comment of PositioningDevInMeters":"Std dev of the gaussian distribution",
            "PositioningDevInMeters":15.0,
            "SOGDevInKnots":0.5,
            "COGDevInDegrees":5.0,
            "SampleMilliSeconds":20000
        },
        {
            "TargetInfo_Type":4,
            "PositioningDevInMeters":15.0,
            "SOGDevInKnots":0.5,
            "COGDevInDegrees":5.0,
            "SampleMilliSeconds":12000
        },
        {
            "TargetInfo_Type":7,
            "PositioningDevInMeters":50,
            "SOGDevInKnots":0.5,
            "COGDevInDegrees":5.0,
            "SampleMilliSeconds":2000
        }
    ],

    "Comment":"One data source may contain multiple data channels, each channel fetch data from one type of data.",
    "CommentOfSourceAndInfoType":"Refer to CommonEnum for the encoding",
    "DataSources":
    [
        {
            "TargetInfo_Source":2,
            "SourceInfo":
            [
                {
                    "TargetInfo_Type":4,
                    "ObservePercentage":90,
                    "meanTransmissionLatencyInMiliSeconds":0,
                    "stdDevTransmissionLatencyInMiliSeconds":0,
                    "meanTimestampErrorInMiliSeconds":0,
                    "stdDevTimestampErrorInMiliSeconds":0,
                    "packetLossPercentage":60
                }
            ]
        },


        {
            "TargetInfo_Source":1,
            "SourceInfo":
            [
                {
                    "TargetInfo_Type":2,
                    "ObservePercentage":20,
                    "meanTransmissionLatencyInMiliSeconds":0,
                    "stdDevTransmissionLatencyInMiliSeconds":0,
                    "meanTimestampErrorInMiliSeconds":0,
                    "stdDevTimestampErrorInMiliSeconds":0,
                    "packetLossPercentage":30
                }
            ]
        },

        {
            "TargetInfo_Source":3,
            "SourceInfo":
            [
                {
                    "TargetInfo_Type":7,
                    "ObservePercentage":10,
                    "meanTransmissionLatencyInMiliSeconds":0,
                    "stdDevTransmissionLatencyInMiliSeconds":0,
                    "meanTimestampErrorInMiliSeconds":0,
                    "stdDevTimestampErrorInMiliSeconds":0,
                    "packetLossPercentage":10
                }
            ]
        }
    ]
}

```