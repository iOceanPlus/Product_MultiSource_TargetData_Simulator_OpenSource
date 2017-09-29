### 注意！该项目为开源项目，请勿将账号、消息队列信息、敏感信息等传入代码或附件！   
Notice! This project is open source, DO NOT PUSH personal or sensitive contents, such as accounts of rabbitmq.
  
# Product_MultiSource_TargetData_Simulator_OpenSource  

## Introduction  
This project simulates targets in a specified area, several devices are installed in each target. These devices get kinematic information about targets and send to channels. Data sources fetech data from channels periodically.  
Messages are encoded using Google's protobuf, and sent to a rabbitmq server.   
https://developers.google.com/protocol-buffers/  
http://www.rabbitmq.com/#getstarted  

Other softwares can fetch data generated by this software from rabbitmq. We have implemented some modules and demos:  
http://git.oschina.net/iOceanPlus/JavaRabbitMQ  
http://git.oschina.net/iOceanPlus/JavaProtoBufDemo  
http://git.oschina.net/iOceanPlus/PythonRabbitMQDemo  
http://git.oschina.net/iOceanPlus/PythonProtobufDemo  

If your program do not support rabbitmq (such as MatLab), use a MQ-UDP adapter as a bridge.  

The architecture of fusion systems based on rabbitmq is in the following figure.
![How to interact with simulator](https://git.oschina.net/uploads/images/2017/0903/173330_065b65ed_854788.jpeg "system.JPG")

The architecture of Data Simulator is in the figure below.
![Data Smulator Architecture](https://git.oschina.net/uploads/images/2017/0903/212725_101dcebf_854788.jpeg "Simulator.JPG")

## Messages  
Message trasmitted: PBTarget encoded by protobuf. See the Protobuf_Files directory.  
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
### HuangPu River in Shanghai, China  
This is one of the busiest water ways in the world, we filter ships with SOG from 3 knots to 80 knots, see the figure below:  
![Huangpu river](https://git.oschina.net/uploads/images/2017/0929/114421_a5e15049_854788.png "屏幕截图.png")  

We focus on the region with latitude [31.3733,31.3867] and longitude [121.4967,121.5067], see below:  

![42 targets in Huangpu river](https://git.oschina.net/uploads/images/2017/0929/115344_4942a03c_854788.png "屏幕截图.png")

This section is 1.3 kilometers long and about 200 meters wide on average. It can be calculated that there are one target on every 6190 square meters, average distance between ships is 78.7 meters on average.



