#  UART测试代码

该测试代码使用的是蓝牙4.0，因为蓝牙初始化的波特率是9600，需要用AT指令去改，这里没有改，所以为了避免使用monitor调试出现乱码，需要在 **make menuconfig** 里面将monitor的波特率改为9600即可。
