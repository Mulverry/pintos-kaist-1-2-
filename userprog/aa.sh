make clean
make
cd build
pintos -v -k -T 60 -m 20   --fs-disk=10 -p tests/userprog/fork-multiple:fork-multiple -- -q   -f run fork-multiple < /dev/null 2> tests/userprog/fork-multiple.errors > tests/userprog/fork-multiple.output
perl -I../.. ../../tests/userprog/fork-multiple.ck tests/userprog/fork-multiple tests/userprog/fork-multiple.result
pintos -v -k -T 60 -m 20   --fs-disk=10 -p tests/userprog/fork-recursive:fork-recursive -- -q   -f run fork-recursive < /dev/null 2> tests/userprog/fork-recursive.errors > tests/userprog/fork-recursive.output
perl -I../.. ../../tests/userprog/fork-recursive.ck tests/userprog/fork-recursive tests/userprog/fork-recursive.result
pintos -v -k -T 60 -m 20   --fs-disk=10 -p tests/userprog/rox-simple:rox-simple -- -q   -f run rox-simple < /dev/null 2> tests/userprog/rox-simple.errors > tests/userprog/rox-simple.output
perl -I../.. ../../tests/userprog/rox-simple.ck tests/userprog/rox-simple tests/userprog/rox-simple.result
pintos -v -k -T 60 -m 20   --fs-disk=10 -p tests/userprog/rox-child:rox-child -p tests/userprog/child-rox:child-rox -- -q   -f run rox-child < /dev/null 2> tests/userprog/rox-child.errors > tests/userprog/rox-child.output
perl -I../.. ../../tests/userprog/rox-child.ck tests/userprog/rox-child tests/userprog/rox-child.result
pintos -v -k -T 60 -m 20   --fs-disk=10 -p tests/userprog/rox-multichild:rox-multichild -p tests/userprog/child-rox:child-rox -- -q   -f run rox-multichild < /dev/null 2> tests/userprog/rox-multichild.errors > tests/userprog/rox-multichild.output
perl -I../.. ../../tests/userprog/rox-multichild.ck tests/userprog/rox-multichild tests/userprog/rox-multichild.result
pintos -v -k -T 600 -m 20 -m 20   --fs-disk=10 -p tests/userprog/no-vm/multi-oom:multi-oom -- -q   -f run multi-oom < /dev/null 2> tests/userprog/no-vm/multi-oom.errors > tests/userprog/no-vm/multi-oom.output
perl -I../.. ../../tests/userprog/no-vm/multi-oom.ck tests/userprog/no-vm/multi-oom tests/userprog/no-vm/multi-oom.result
pintos -v -k -T 60 -m 20   --fs-disk=10 -p tests/userprog/multi-recurse:multi-recurse -- -q   -f run 'multi-recurse 15' < /dev/null 2> tests/userprog/multi-recurse.errors > tests/userprog/multi-recurse.output
perl -I../.. ../../tests/userprog/multi-recurse.ck tests/userprog/multi-recurse tests/userprog/multi-recurse.result
