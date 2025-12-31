# compile
mips64-ultra-elf-gcc -G 0 -nostdinc -DNDEBUG -Iinclude/z64hdr/include -Iinclude/z64hdr/oot_mq_debug -Os -s --std=gnu99 -march=vr4300 -mfix4300 -mabi=32 -mno-abicalls -mdivide-breaks -fno-zero-initialized-in-bss -fno-toplevel-reorder -ffreestanding -fno-common -fno-merge-constants -mno-explicit-relocs -mno-split-addresses -funsigned-char -mno-memcpy -c init.c -o tmp.o

# link
touch entry.ld
mips64-ultra-elf-ld --emit-relocs -o tmp.elf tmp.o -defsym ENTRY_POINT=0x80800000 -Linclude/z64hdr/oot_mq_debug -L include/z64hdr/common/ -T z64hdr_no_bss.ld

# generate overlay
./novl -v -c -A 0x80800000 -o init.zovl tmp.elf

# get func address
mips64-ultra-elf-objdump --syms tmp.elf | grep -w SceneFunc > func.txt
