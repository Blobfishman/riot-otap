all:
	cd ./Client/ && make
	cd ./Server/ && make
	cd ./Command_center/ && make all

clean:
	cd ./Client/ && make clean
	cd ./Server/ && make clean
	cd ./Command_center/ && make clean
