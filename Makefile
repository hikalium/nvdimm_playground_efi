CLANGXX=/usr/local/opt/llvm/bin/clang++
LLD_LINK=/usr/local/opt/llvm/bin/lld-link
QEMU=qemu-system-x86_64

DST_DIR=mnt/EFI/BOOT

$(DST_DIR)/BOOTX64.EFI : main.cc asm.S efi.h guid.h
	mkdir -p $(DST_DIR)
	$(CLANGXX) \
		-target x86_64-pc-win32-coff \
		-fno-stack-protector -fshort-wchar \
		-mno-red-zone \
		-nostdlibinc \
		-Wall -std=c++17 -c \
		main.cc asm.S
	$(LLD_LINK) \
		-subsystem:efi_application -nodefaultlib \
		-entry:efi_main main.o asm.o -out:$@

run : mnt/EFI/BOOT/BOOTX64.EFI
	$(QEMU) \
		-bios ovmf/bios64.bin \
		-machine q35,nvdimm -cpu qemu64 -smp 4 \
		-monitor stdio \
		-m 8G,slots=2,maxmem=10G \
		-drive format=raw,file=fat:rw:mnt -net none \
		-object memory-backend-file,id=mem1,share=on,mem-path=pmem.img,size=2G \
		-device nvdimm,id=nvdimm1,memdev=mem1
