The sensor monitoring system:
It consists of sensor nodes measuring the room temperature, a
sensor gateway that acquires all sensor data from the sensor nodes, and an SQL database to
store all sensor data processed by the sensor gateway. 

Sensor gateway:
The sensor gateway consists of a main process and a log process.
The main process runs three threads: the connection, the data, and the storage
manager thread. A circular queue is used for shared communication between the connection (writer) and the data and storage (both
readers) manager. Data can only be dequeued when both the data and storage
manager have read the data. Notice that read/write/update-access to shared data
needs to be thread-safe!
The log process receives log-events from the main process using a FIFO