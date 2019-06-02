
file :=read.c
target_dir :=/mnt/share/uart_test/baseV2.0

all:
	cp       $(target_dir)/$(file)    ./
	arm-linux-gnueabihf-gcc        $(file)         -o          readapp      -pthread
	sync
	@cp     readapp    $(target_dir)/
	sync
	@echo    "======copy finish======"
clean:
	@rm  *rocom
	@clear
	@ls
