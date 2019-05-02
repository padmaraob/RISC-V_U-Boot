# RISC-V_U-Boot
Development repository for RISC-V U-Boot on Microsemi FPGA

This repository contains the U-Boot created by Microsemi for RISC-V core on Smartfusion2 M2Sxxx devices. The Smartfusion2 M2S150 FPGA Advance Dev Kit is used for RISC-V U-Boot and the FPGA design available at [RISCV-on-Microsemi-FPGA](https://github.com/RISCV-on-Microsemi-FPGA/M2S150-Advanced-Dev-Kit). 

### RISC-V cross tools

Get the RISC-V cross tool from [riscv github](https://github.com/riscv/riscv-gnu-toolchain) and follow the README file for installation.

        $ git clone --recursive https://github.com/riscv/riscv-gnu-toolchain

After clone of riscv-gnu-toolchain

	$ cd riscv-gnu-toolchain

To build the 32-bit rv32 toolchain, use:

        $./configure --prefix={Installation directory} --with-arch=rv32imac --with-abi=ilp32
	$ make linux

### Getting U-Boot sources

        $ git clone https://github.com/Microsemi-SoC-IP/RISC-V_U-Boot
        $ cd RISC-V_U-Boot
    
### Build U-Boot

1. Set the PATH

        $ export PATH=$PATH:${installation directory}/bin
        
1. Create U-Boot configuration based on architecture defaults:

        $ make riscv-m2sxxx_defconfig

1. Optionally edit the configuration via an ncurses interface:

        $ make menuconfig

1. Build the U-Boot image:

        $ make

### Build and Debug U-Boot with Softconsole

The Softconsole available on [RISCV-on-Microsemi-FPGA](https://github.com/RISCV-on-Microsemi-FPGA/SoftConsole).

Follow the steps explained in `Debug_setup_with_SC.docx` document available in u-boot root directory.
