Week 1：搭建 QEMU，编译内核

操作过程：

一、环境：

VMware Ubuntu 22.04 环境。配置为 2 个 vCPU、4 GB 内存、NAT 网络。ARM 交叉编译器为 arm-linux-gnueabihf-gcc 11.4.0，QEMU 为 6.2.0。

二、操作过程：

1. 准备源码和工具：通过 Ubuntu 软件仓库安装 QEMU 及编译依赖，从 kernel.org 下载 Linux 6.12，从 BusyBox 官方站点下载 BusyBox 1.36.1。

2. 编译内核：设置 ARCH=arm、CROSS_COMPILE=arm-linux-gnueabihf-，以 vexpress_defconfig 为基础，启用 ext4、devtmpfs、virtio 和 9p。生成 zImage 与 vexpress-v2p-ca9.dtb。编译目录与源码目录分开。

3. 编译 BusyBox：使用相同交叉工具链，启用静态链接，将 Shell 和基础命令安装到根目录树，准备 /dev、/proc、/sys、/etc、/tmp 等目录，并配置 init、串口 Shell 和启动脚本。

4. 制作文件系统：用 mkfs.ext4 -d 将根目录树写入 128 MiB 的 rootfs.ext4 文件镜像；通过 debugfs 添加 console 和 null 设备节点。

5. 启动：使用 QEMU vexpress-a9、Cortex-A9、2 核、512 MiB 内存，直接加载内核及设备树，以模拟 SD 卡上的 ext4 镜像作为根文件系统。通过 virtio-9p 将 Ubuntu 的 ~/pta-week1/shared 挂载到 ARM 系统的 /mnt/host。


三、验证：
通过以下命令启动QEMU：

```bash
cd ~/pta-week1
bash scripts/build.sh
python3 scripts/verify.py
bash scripts/run-qemu.sh
```


QEMU 内实际执行 uname -a，输出为：

```text
Linux pta-arm 6.12.0-pta-week1 #1 SMP Wed Nov 11 22:36:16 CST 2026 armv7l GNU/Linux
```

BusyBox 1.36.1 已在 ARM 系统中运行；ELF 检查显示 ELF32、ARM、静态链接。根文件系统挂载为 ext4，共享目录挂载为 9p。

