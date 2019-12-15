#!/bin/sh

valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./hpcc > valgrind_result.txt
