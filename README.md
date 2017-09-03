### 注意！该项目为开源项目，请勿将账号、消息队列信息、敏感信息等传入代码或附件！   
Notice! This project is open source, DO NOT PUSH personal or sensitive contents, such as accounts of rabbitmq.
  
# Product_MultiSource_TargetData_Simulator_OpenSource
## Dataset used for initialization  
- Dataset used for initialization  
ais_ship_realtime_valid_anonymous_randOrder.csv：https://drive.google.com/open?id=0B01TUPO5dae2VTZGblI3a1pCazA  
OR ais_ship_realtime_valid_randomOrder.csv: add more columns such as shipNames into ais_ship_realtime_valid_anonymous_randOrder.csv

- Dataset used for water and land detection  
Waters_1_Degree_2014.csv：https://drive.google.com/open?id=0B01TUPO5dae2U0ItUk5MaDRnSlU  
For details about the content of this file refer to： Wu L, Xu Y, Wang Q, et al. Mapping Global Shipping Density from AIS Data[J]. The Journal of Navigation, 2017, 70(1): 67-81.

## Environment  
Linux, g++ 4.7 or above is needed.  

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
Download libs needed from http://pan.baidu.com/share/link?shareid=1523880373&uk=3506494889   
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