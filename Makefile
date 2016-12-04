all: compile

compile: src/utils.c src/se_model.c
	gcc -I$(TINY_EKF_INCL) -I./inc -c -fPIC src/utils.c -o build/utils.o -lm
	gcc -I$(TINY_EKF_INCL) -I./inc -c  -fPIC src/se_model.c -o build/se_model.o -lm
	gcc -shared -o build/librobotse.so build/utils.o build/se_model.o -fPIC -Wl,--whole-archive -L$(TINY_EKF_LIB) -ltinyekf -Wl,--no-whole-archive

compile_test: compile src/test_se_model.c
	gcc -Wall -I$(TINY_EKF_INCL) -I./inc -o ./build/robotse_test ./src/test_se_model.c -Lbuild -lrobotse -lm

clean:
	rm -rf build/*

