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

## How to run    
- Check if the rabbitmq server is runing  
- Download the executable file and param.json in the attachment of this project   
修改param.txt中的ExternV_DEFAULT_SECONDS_BEFORE_DELETE_AssociatedTARGET参数，来控制默认目标消批时间，默认为24小时  
param.txt中的ExternV_ShipCount变量是综合目标数量，DataAssoc假设同一个目标的不同类型编号除以ExternV_ShipCount的余数相等，结合该参数来判断程序的关联结果是否正确。当不确定综合目标数量时，DataAssoc日志中输出的关联错误统计等信息可以忽略。
- 从网盘下载并部署相关的动态链接库（一台服务器上只需要部署一次，可以在程序运行失败的时候再执行该步骤）
见：https://git.oschina.net/iOceanPlus/Development/blob/master/Linux/README.md ，搜索关键字“库”  

- 检查程序是否已经在运行  
执行```ps -ef |grep DataAs```
如果能看到该线程，则说明已经在运行，不需要再次启动。如果看不到该线程，或者要重启程序，可以kill该线程  
- 在可执行程序目录下运行项目生成的可执行文件： ```nohup ./Dataxxx   &```
使用tab自动补全，防止名称输入错误，如果无法执行成功，尝试给程序添加可执行权限，见development仓库
- 检查程序是否在正常运行  
命令行输入```top```，查看是否有该程序  
输入```vi nohup.txt```，查看日志输出有没有报错  
