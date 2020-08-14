# DDS Documentation
-
This is the documentation of source code used for the NTUST Water Dispenser IoT Project. On this project we used a library package provided by [RTI Connext DDS](https://www.rti.com/products)

- Sample QOS => [USER_QOS_PROFILES.xml](./USER_QOS_PROFILES.xml)
- Makefile => [makefile_water_x64Linux3gcc5.4.0](./makefile_water_x64Linux3gcc5.4.0)
- rtiddsgen generated code => [water.c](./water.c)
- rtiddsgen generated code => [water.h](./water.h)
- rtiddsgen generated code => [water.idl](./water.idl)
- rtiddsgen generated code => [waterPlugin.c](./waterPlugin.c)
- rtiddsgen generated code => [waterPlugin.h](./waterPlugin.h)
- rtiddsgen generated code => [waterSupport.c](./waterSupport.c)
- rtiddsgen generated code => [waterSupport.h](./waterSupport.h)
- Sample Data => [DDS_Data.csv](./DDS_Data.csv)
- QOS Settings Folder => [QOS](./QOS)
- This Readme => README.md
- water publisher for all data => [water_publisher.c](./water_publisher.c)
- water publisher for always data => [water_publisher_classA.c](./water_publisher_classA.c)
- water publisher for regular data => [water_publisher_classB.c](./water_publisher_classB.c)
- water publisher for seldom data => [water_publisher_classC.c](./water_publisher_classC.c)
- water subscriber for all data => [water_subscriber.c](./water_subscriber.c)
- water subscriber for always data => [water_subscriber_classA.c](./water_subscriber_classA.c)
- water subscriber for regular data => [water_subscriber_classB.c](./water_subscriber_classB.c)
- water subscriber for seldom data => [water_subscriber_classC.c](./water_subscriber_classC.c)

###Documentation Comments are already attached on each file