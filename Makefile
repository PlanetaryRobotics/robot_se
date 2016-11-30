all: compile

compile: src/utils.c src/se_model.c
	gcc -I$(TINY_EKF_INCL) -I./inc -c src/utils.c -o build/utils.o -lm
	gcc -I$(TINY_EKF_INCL) -I./inc -c src/se_model.c -o build/se_model.o -lm
	ar -rcs build/librobotse.a build/utils.o build/se_model.o

compile_test: compile src/test_se_model.c
	gcc -Wall -I$(TINY_EKF_INCL) -I./inc -o ./build/robotse_test ./src/test_se_model.c -static -L$(TINY_EKF_LIB) -Lbuild -lm -lrobotse -ltinyekf -lm

clean:
	rm -rf build/*

