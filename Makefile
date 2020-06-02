all:
	cd ./node/ && make
	cd ./gateway/ && make
	cd ./command_center/ && make all

clean:
	cd ./node/ && make clean
	cd ./gateway/ && make clean
	cd ./command_center/ && make clean
