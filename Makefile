K=kernel
U=user

OBJS = \
  $K/entry.o \
  $K/start.o \
  $K/console.o \
  $K/printf.o \
  $K/uart.o \
  $K/kalloc.o \
  $K/spinlock.o \
  $K/string.o \
  $K/main.o \
  $K/vm.o \
  $K/cpu.o \
  $K/proc.o \
  $K/swtch.o \
  $K/trampoline.o \
  $K/trap.o \
  $K/syscall.o \
  $K/sysproc.o \
  $K/bio.o \
  $K/fs.o \
  $K/log.o \
  $K/sleeplock.o \
  $K/file.o \
  $K/pipe.o \
  $K/exec.o \
  $K/sysfile.o \
  $K/kernelvec.o \
  $K/plic.o \
  $K/virtio_disk.o

# Try to infer normal GCC version
ifndef GCC
GCC := $(shell if command -v gcc-11.2.0 > /dev/null 2>&1; \
	then echo 'gcc-11.2.0'; \
	elif command -v gcc-11.1.0 > /dev/null 2>&1; \
	then echo 'gcc-11.1.0'; \
	elif command -v gcc-11 > /dev/null 2>&1; \
	then echo 'gcc-11'; \
	elif command -v gcc-10 > /dev/null 2>&1; \
	then echo 'gcc-10'; \
	elif command -v gcc > /dev/null 2>&1; \
	then echo 'gcc'; \
	else echo "Could not find GCC, please enter the correct version here manually"; \
	fi)
endif

# riscv64-unknown-elf- or riscv64-linux-gnu-
# perhaps in /opt/riscv/bin
#TOOLPREFIX = 
# Try to infer the correct TOOLPREFIX if not set
ifndef TOOLPREFIX
TOOLPREFIX := $(shell if riscv64-unknown-elf-objdump -i 2>&1 | grep 'elf64-big' >/dev/null 2>&1; \
	then echo 'riscv64-unknown-elf-'; \
	elif riscv64-linux-gnu-objdump -i 2>&1 | grep 'elf64-big' >/dev/null 2>&1; \
	then echo 'riscv64-linux-gnu-'; \
	elif riscv64-unknown-linux-gnu-objdump -i 2>&1 | grep 'elf64-big' >/dev/null 2>&1; \
	then echo 'riscv64-unknown-linux-gnu-'; \
	else echo "***" 1>&2; \
	echo "*** Error: Couldn't find a riscv64 version of GCC/binutils." 1>&2; \
	echo "*** To turn off this error, run 'gmake TOOLPREFIX= ...'." 1>&2; \
	echo "***" 1>&2; exit 1; fi)
endif

# Try to infer RISC-V gcc version
ifndef RISCVGCCSUFFIX
RISCVGCCSUFFIX := $(shell if command -v $(TOOLPREFIX)gcc-11.2.0 > /dev/null 2>&1; \
	then echo 'gcc-11.2.0'; \
	elif command -v $(TOOLPREFIX)gcc-11.1.0 > /dev/null 2>&1; \
	then echo 'gcc-11.1.0'; \
	elif command -v $(TOOLPREFIX)gcc-11 > /dev/null 2>&1; \
	then echo 'gcc-11'; \
	elif command -v $(TOOLPREFIX)gcc-10 > /dev/null 2>&1; \
	then echo 'gcc-10'; \
	elif command -v $(TOOLPREFIX)gcc > /dev/null 2>&1; \
	then echo 'gcc'; \
	else echo "Could not find GCC, please enter the correct version here manually"; \
	fi)
endif

ifndef GDB
GDB := $(shell if command -v $(TOOLPREFIX)gdb > /dev/null 2>&1; \
        then echo '$(TOOLPREFIX)gdb'; \
        elif command -v gdb-multiarch > /dev/null 2>&1; \
        then echo 'gdb-multiarch'; \
        fi)
endif

QEMU = qemu-system-riscv64
CC = $(TOOLPREFIX)$(RISCVGCCSUFFIX)
AS = $(TOOLPREFIX)gas
LD = $(TOOLPREFIX)ld
OBJCOPY = $(TOOLPREFIX)objcopy
OBJDUMP = $(TOOLPREFIX)objdump

CFLAGS = -Wall -Werror -O -fno-omit-frame-pointer -ggdb -gdwarf-2
CFLAGS += -MD
CFLAGS += -mcmodel=medany
CFLAGS += -ffreestanding -fno-common -nostdlib -mno-relax
CFLAGS += -I.
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

# Disable PIE when possible (for Ubuntu 16.10 toolchain)
ifneq ($(shell $(CC) -dumpspecs 2>/dev/null | grep -e '[^f]no-pie'),)
CFLAGS += -fno-pie -no-pie
endif
ifneq ($(shell $(CC) -dumpspecs 2>/dev/null | grep -e '[^f]nopie'),)
CFLAGS += -fno-pie -nopie
endif

LDFLAGS = -z max-page-size=4096

$K/kernel: $(OBJS) $K/kernel.ld $U/initcode
	$(LD) $(LDFLAGS) -T $K/kernel.ld -o $K/kernel $(OBJS) 
	$(OBJDUMP) -S $K/kernel > $K/kernel.asm
	$(OBJDUMP) -t $K/kernel | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $K/kernel.sym

$U/initcode: $U/initcode.S
	$(CC) $(CFLAGS) -march=rv64g -nostdinc -I. -Ikernel -c $U/initcode.S -o $U/initcode.o
	$(LD) $(LDFLAGS) -N -e start -Ttext 0 -o $U/initcode.out $U/initcode.o
	$(OBJCOPY) -S -O binary $U/initcode.out $U/initcode
	$(OBJDUMP) -S $U/initcode.o > $U/initcode.asm

tags: $(OBJS) _init
	etags *.S *.c

ULIB = $U/ulib.o $U/usys.o $U/printf.o $U/umalloc.o $U/printf.o

$U/_sh: $U/sh.o $U/sh_lib.o $U/sh_util.o $(ULIB)
	# the shell needs to be linked with its implementation in sh_lib.c
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^
	$(OBJDUMP) -S $@ > $*.asm
	$(OBJDUMP) -t $@ | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $*.sym

$U/_tests: $U/tests.o $U/sh_lib.o $U/sh_util.o $(ULIB)
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^
	$(OBJDUMP) -S $@ > $*.asm
	$(OBJDUMP) -t $@ | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $*.sym

_%: %.o $(ULIB)
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^
	$(OBJDUMP) -S $@ > $*.asm
	$(OBJDUMP) -t $@ | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $*.sym

$U/usys.S : $U/usys.sh
	sh $U/usys.sh > $U/usys.S

$U/usys.o : $U/usys.S
	$(CC) $(CFLAGS) -c -o $U/usys.o $U/usys.S

$U/_forktest: $U/forktest.o $(ULIB)
	# forktest has less library code linked in - needs to be small
	# in order to be able to max out the proc table.
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $U/_forktest $U/forktest.o $U/ulib.o $U/usys.o
	$(OBJDUMP) -S $U/_forktest > $U/forktest.asm

mkfs/mkfs: mkfs/mkfs.c $K/fs.h $K/param.h
	$(GCC) -Werror -Wall -I. -o mkfs/mkfs mkfs/mkfs.c

# Prevent deletion of intermediate files, e.g. cat.o, after first build, so
# that disk image changes after first build are persistent until clean.  More
# details:
# http://www.gnu.org/software/make/manual/html_node/Chained-Rules.html
.PRECIOUS: %.o

UPROGS= \
	$U/_cat \
	$U/_echo \
	$U/_forktest \
	$U/_grep \
	$U/_init \
	$U/_kill \
	$U/_ln \
	$U/_ls \
	$U/_mkdir \
	$U/_rm \
	$U/_sh \
	$U/_tests \
	$U/_stressfs \
	$U/_usertests \
	$U/_grind \
	$U/_wc \
	$U/_zombie \
	$U/_malloc_agent \

fs.img: mkfs/mkfs GBS $(UPROGS)
	mkfs/mkfs fs.img GBS $(UPROGS)

-include kernel/*.d user/*.d

clean: 
	rm -f *.tex *.dvi *.idx *.aux *.log *.ind *.ilg \
	*/*.o */*.d */*.asm */*.sym .sym .asm \
	$U/initcode $U/initcode.out $K/kernel fs.img \
	mkfs/mkfs .gdbinit \
        $U/usys.S \
	$(UPROGS)
	rm -rf logs/


# try to generate a unique GDB port
GDBPORT = $(shell expr `id -u` % 5000 + 25000)
# QEMU's gdb stub command line changed in 0.11
QEMUGDB = $(shell if $(QEMU) -help | grep -q '^-gdb'; \
	then echo "-gdb tcp::$(GDBPORT)"; \
	else echo "-s -p $(GDBPORT)"; fi)

ifndef CPUS
CPUS := 3
endif

QEMUOPTS = -machine virt -bios none -kernel $K/kernel -m 128M -smp $(CPUS) -nographic
QEMUOPTS += -drive file=fs.img,if=none,format=raw,id=x0
QEMUOPTS += -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0

GDBOPTS = --nx --x .gdbinit

build: $K/kernel fs.img

qemu: $K/kernel fs.img
	$(QEMU) $(QEMUOPTS)

.gdbinit: .gdbinit.tmpl-riscv
	sed "s/:1234/:$(GDBPORT)/" < $^ > $@

qemu-gdb: $K/kernel .gdbinit fs.img
	@echo "*** Now run 'make gdb' in another window." 1>&2
	$(QEMU) $(QEMUOPTS) -S $(QEMUGDB)

gdb:
	$(GDB) $(GDBOPTS)

# Appended K means that only the kernel is compiled with the additional flag
# This is necessary since sometimes problems only occur in one of them

# Compile with kernel + userspace with GCC static analysis enabled
# -fanalyzer:			GCC static analysis (for GCC >= 10.0 only)
# (https://developers.redhat.com/blog/2020/03/26/static-analysis-in-gcc-10/)
staticAnalysisK: CFLAGS += -fanalyzer -Wno-analyzer-malloc-leak 
staticAnalysisK: $K/kernel 

staticAnalysis: CFLAGS += -fanalyzer
staticAnalysis: $K/kernel fs.img

# target test
QEMUOPTSTEST = -machine virt -bios none -kernel $K/kernel -m 128M -smp $(CPUS) #-nographic
QEMUOPTSTEST += -drive file=fs.img,if=none,format=raw,id=x0
QEMUOPTSTEST += -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0
# disable display (we only interact with the guest via serial/uart)
QEMUOPTSTEST += -display none
# redirect the guest serial device to pipes on host
QEMUOPTSTEST += -chardev pipe,id=vmpipe,path=/tmp/gbs_test/vm -serial chardev:vmpipe
# redirect the qemu monitor to pipes on host
QEMUOPTSTEST += -chardev pipe,id=qemupipe,path=/tmp/gbs_test/qemu -mon chardev=qemupipe,mode=control

GDBOPTSTEST = --nx --interpreter=mi3

# Note: This target is for automated testing
test-qemu: $K/kernel fs.img
	$(QEMU) $(QEMUOPTSTEST)

# Note: This target is for automated testing with gdb support
test-qemu-gdb: $K/kernel .gdbinit fs.img
	$(QEMU) $(QEMUOPTSTEST) -S $(QEMUGDB)

# Note: This target is for automated testing in conjunction with test-qemu-gdb
#       stdbuf is used to encapsulate gdb's input/output and disable buffering.
#       This is required to trick gdb into using named pipes.
test-gdb:
	stdbuf -i0 -o0 -e0 $(GDB) --nx --interpreter=mi3 < /tmp/gbs_test/gdb.in > /tmp/gbs_test/gdb.out
