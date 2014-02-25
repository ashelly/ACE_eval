




handcomp:   gcc -s -Os -o so_handcomp so_handcomp.c

microeval:
	gcc -s -Os -o microeval ace_microeval.c

test_golf:
	gcc -s -O3 -o test_golf accuracy_test.c ace_eval_golf.c
time_golf:
	gcc -lrt -s -O3 -o time_golf speed_test.c ace_eval_golf.c

test_base:
	gcc -s -O3 -o test_base accuracy_test.c ace_eval_base.c
time_base:
	gcc -lrt -s -O3 -o time_base speed_test.c ace_eval_base.c

test_unroll:
	gcc -s -O3 -o test_unroll accuracy_test.c ace_eval_unroll.c
time_unroll:
	gcc -lrt -s -O3 -o time_unroll speed_test.c ace_eval_unroll.c

test_flushtable:
	gcc -s -O3 -o test_flushtable accuracy_test.c ace_eval_flushtable.c
time_flushtable:
	gcc -lrt -s -O3 -o time_flushtable speed_test.c ace_eval_flushtable.c

test_decompress:
	gcc -s -O3 -o test_decompress accuracy_test.c ace_eval_decompress.c
time_decompress:
	gcc -lrt -s -O3 -o time_decompress speed_test.c ace_eval_decompress.c
test_decompress3:
	gcc -g -O3 -o test_decompress accuracy_test.c ace_eval5_decompress.c
time_decompress3:
	gcc -lrt -s -O3 -o time_decompress speed_test.c ace_eval5_decompress.c

test_all:	test_decompress test_flushtable test_unroll test_base test_golf
time_all:	time_decompress time_flushtable time_unroll time_base time_golf
all:  test_all time_all microeval