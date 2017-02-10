# RISC-V_U-Boot
Development repository for RISC-V U-Boot on Microsemi FPGA

This repository contains the U-Boot created by Microsemi for RISC-V core on Smartfusion2 M2Sxxx devices. The Smartfusion2 M2S150 FPGA Advance Dev Kit is used for RISC-V U-Boot and the FPGA design available at [RISCV-on-Microsemi-FPGA](https://github.com/RISCV-on-Microsemi-FPGA/M2S150-Advanced-Dev-Kit). 

### RISC-V cross tools

Get the RISC-V cross tool from [sifive github](https://github.com/sifive/freedom-e-sdk.git) and follow the README file for installation.

        git clone --recursive https://github.com/sifive/freedom-e-sdk.git

After clone of freedom-e-sdk, edit the `make` file for `riscv32-unknown-linux-gnu-gcc` like below

        $(toolchain_dest)/bin/$(target32)-gcc: $(toolchain_srcdir)
		      mkdir -p $(toolchain32_wrkdir)
		      cd $(toolchain32_wrkdir); $(toolchain_srcdir)/configure --prefix=$(toolchain_dest) --with-arch=RV32IMA
		      $(MAKE) linux -C $(toolchain32_wrkdir)

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