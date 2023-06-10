make clean
make
cd build
pintos -v -k -T 60 -m 20   — -q   run priority-sema < /dev/null 2> tests/threads/priority-sema.errors > tests/threads/priority-sema.output
perl -I../.. ../../tests/threads/priority-sema.ck tests/threads/priority-sema tests/threads/priority-sema.result
