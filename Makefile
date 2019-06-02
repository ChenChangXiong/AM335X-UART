
file :=uart_test.c
target_dir :=/mnt/share/uart_test/baseV2.0

all:
	cp       $(target_dir)/$(file)    ./
	arm-linux-gnueabihf-gcc        $(file)         -o          UartApp_V1       -pthread
	#arm-linux-gcc        $(file)         -o          UartApp_V1       -pthread
	sync
	@cp     *App_V1    $(target_dir)/
	@echo    "======copy finish======"
	sync
clean:
	@rm  *App*
	@clear
	@ls
