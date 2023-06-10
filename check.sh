make clean
source ./activate
cd userprog
make clean
make
cd build
sleep 2
pintos-mkdisk filesys.dsk 10
pintos --fs-disk filesys.dsk -p tests/userprog/args-multiple:args-multiple -- -q -f run 'args-multiple onearg twoarg threearg'