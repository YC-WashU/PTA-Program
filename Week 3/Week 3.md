Task 1

### Linux 系统启动流程

本次在 QEMU 模拟的双核 Cortex-A9 上启动 Linux 6.12。串口从第一条内核消息一直记录到 BusyBox 的命令提示符。

* 内核启动过程：

QEMU 把压缩内核 zImage 和设备树 DTB 放入内存。ARM 启动代码调用 decompress\_kernel() 解压内核，建立初始页表并打开内存管理单元（MMU），随后跳到通用入口 start\_kernel()。

* 内核启动的后续流程：

start\_kernel() 调用 setup\_arch() 识别板型、安排内存，随后初始化调度器、中断和控制台。最后，rest\_init() 创建 PID 1。此时 PID 1 执行的还是内核函数 kernel\_init()，它通过 kernel\_init\_freeable() 启动第二个 CPU，并运行内置驱动的初始化函数。

* 根目录：

存储驱动识别出模拟 SD 卡 mmcblk0。启动参数 root=/dev/mmcblk0 指定它作为根设备，prepare\_namespace() 等待设备就绪，再调用 mount\_root() 挂载 ext4。串口随后打印“VFS: Mounted root (ext4 filesystem) on device 179:0.”。

* 进入命令行：

根目录就绪后，kernel\_init() 通过 run\_init\_process() 和 kernel\_execve() 执行 /sbin/init。BusyBox 的 init 按 /etc/inittab 先运行启动脚本 rcS，再在串口 ttyAMA0 上启动 /bin/sh。/proc/1/exe 指向 /bin/busybox，uname -a 返回 armv7l，说明系统已经可以执行用户侧命令。





Task 2 （1）

### 以 I²C 为例，看 Linux 驱动模型框架

实验板用 I²C 控制显示发送器 SiI9022。要写入它的一个寄存器，系统需要明确：①这块芯片接在哪里；②应该怎样把数据传送过去。

* 设备和驱动的分工：

Linux 用 i2c\_adapter 表示一条总线，用 i2c\_client 表示这条总线上某个地址的设备。控制器驱动负责收发数据，芯片驱动负责解释寄存器。0 号总线上地址为 0x39 的设备，在 Linux 中记作 0-0039。

* 内核的工作：

设备树用 arm,versatile-i2c 标明控制器型号。平台总线匹配到控制器驱动后，i2c\_versatile\_probe() 映射寄存器并注册 adapter。接着，of\_i2c\_register\_devices() 遍历该总线的子节点，读出 reg 中的地址，为每个设备创建 client。

* 芯片驱动：

I²C 核心用子节点的 compatible 与驱动的匹配表比对。匹配 sil,sii9022 后，核心调用 sii902x\_probe()。驱动准备供电、访问寄存器并核对芯片 ID；整个 probe 成功返回后，设备与驱动完成绑定。此后，芯片驱动发起读写，由 adapter 的传输函数操作时钟线 SCL 和数据线 SDA。

* 串口记录：

日志中出现 i2c 0-0039，说明内核已为该地址创建设备。另一条 sii902x 0-0060 供电提示说明驱动已开始探测同一总线上的 0x60 接口。





Task 2 （2）

### DTS 的作用与 DTB 的解析过程

设备树用来描述一块板上有哪些硬件、它们接在哪里。DTS 是我们能读写的源文件，编译后得到 DTB。QEMU 在启动时把 DTB 交给 ARM 内核。



* 为什么引入设备树：

同一款 CPU 可以装在不同的板上，外设地址、中断和接线也会变化。以前这些差异常写在各块板的 C 初始化代码里。用 DTS 单独描述接线后，只要内核已有相应驱动，换板时就可以沿用内核和驱动，只更换 DTB。



* 怎样读一个设备节点：

以板上的 dvi-transmitter@39 为例：它位于 I²C 控制器节点下面，所以 reg = <0x39> 表示它在这条总线上的地址。compatible 列出芯片兼容型号，供内核选择驱动。同一个 reg 写在控制器节点上，则按上一级总线的规则描述寄存器窗口的位置和大小。



* DTB 在Kemel启动的哪个过程被解析：

在 Linux 6.12 的 ARM 启动路径中，start\_kernel() 调用 setup\_arch()，再进入 setup\_machine\_fdt()。其中 early\_init\_dt\_verify() 检查 DTB 头，of\_flat\_dt\_match\_machine() 按根节点的 compatible 选择板型，early\_init\_dt\_scan\_nodes() 读取启动参数和内存范围。此时内核直接读取扁平的二进制树。

稍后，setup\_arch() 调用 unflatten\_device\_tree()，把 DTB 展开为内核可遍历的节点树。到了驱动初始化阶段，内核才沿树创建控制器设备；I²C 控制器注册后，I²C 核心再创建它的子设备。串口先打印板型 V2P-CA9，随后才出现设备 0-0039，对应这两个阶段。



