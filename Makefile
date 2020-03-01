CLANGXX=/usr/local/opt/llvm/bin/clang++
LLD_LINK=/usr/local/opt/llvm/bin/lld-link
QEMU=qemu-system-x86_64
QEMU_ARGS= \
		-bios ovmf/bios64.bin \
		-machine q35,nvdimm -cpu qemu64 -smp 4 \
		-monitor stdio \
		-m 8G,slots=2,maxmem=10G \
		-net none \
		-object memory-backend-file,id=mem1,share=on,mem-path=pmem.img,size=2G \
		-device nvdimm,id=nvdimm1,memdev=mem1

DST_DIR=mnt/EFI/BOOT
SRCS=main.cc assert.cc efi_stdio.cc
OBJS= $(addsuffix .o, $(basename $(SRCS)))

$(DST_DIR)/BOOTX64.EFI : $(SRCS) efi.h guid.h
	mkdir -p $(DST_DIR)
	$(CLANGXX) \
		-target x86_64-pc-win32-coff \
		-fno-stack-protector -fshort-wchar \
		-mno-red-zone \
		-nostdlibinc \
		-Wall -Wpedantic -std=c++17 -O3 -c \
		$(SRCS)
	$(LLD_LINK) \
		-subsystem:efi_application -nodefaultlib \
		-entry:efi_main $(OBJS) -out:$@

.FORCE :

run : $(DST_DIR)/BOOTX64.EFI
	$(QEMU) $(QEMU_ARGS) \
		-drive format=raw,file=fat:rw:mnt -net none

run_img : $(DST_DIR)/BOOTX64.EFI nvdimm_playground.img
	$(QEMU) $(QEMU_ARGS) \
		-hda nvdimm_playground.img

img : nvdimm_playground.img

nvdimm_playground.img : .FORCE
	# Make FAT partition image
	dd if=/dev/zero of=fat.img bs=12288 count=1024
	/usr/local/Cellar/dosfstools/4.1/sbin/mkfs.fat -F 12 fat.img
	mkdir -p mnt_img
	hdiutil attach -mountpoint mnt_img fat.img
	cp -r mnt/* mnt_img/
	hdiutil detach mnt_img
	# Make GPT partitioned image
	dd if=/dev/zero of=$@ bs=32768 count=1024
	sgdisk -o $@
	sgdisk -n 1:8192:32768 -c 1:"EFI System Partition" -t 1:ef00 $@
	sgdisk -p $@
	dd if=fat.img of=$@ bs=512 oseek=8192

clean:
	-rm $(DST_DIR)/BOOTX64.EFI

