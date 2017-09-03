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