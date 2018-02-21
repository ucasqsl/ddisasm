# ./reassemble_and_test.sh project_directory binary_path [compiler_flags]
# Take a project directory where the main makefile is located
# and the relative path of the main executable within the directory and:
# -Rebuild the project
# -Disassemble the binary
# -Reassemble the binary (substituting the old one)
# -Run the makefile's tests

# The reassembly uses the 'compiler_flags' arguments

# Example:
# ./reasemble_and_test.sh ../real_world_examples/grep-2.5.4 src/grep -lpcre
#



if [[ $# -eq 0 || $1 == "-h" || $1 == "--help" ]]; then
    printf "USAGE: ./reassemble_and_test.sh project_directory binary_path [compiler_flags]
 Take a project directory where the main makefile is located
 and the relative path of the main executable within the directory and:
 -Rebuild the project
 -Disassemble the binary
 -Reassemble the binary (substituting the old one)
 -Run the makefile's tests

 The reassembly uses the 'compiler_flags' arguments

 Example:
 ./reasemble_and_test.sh ../real_world_examples/grep-2.5.4 src/grep -lpcre
"   
    exit
fi
dir=$1
exe=$2
shift
shift

printf "\n Rebuilding project $dir\n"
if !(make clean -C $dir &>/dev/null && make -C $dir &>/dev/null); then
    printf "\n Initial compilation failed\n"
    exit 1
fi

printf "\n\n Disasembling $dir/$exe into $dir/$exe.s"
if !(./disasm "$dir/$exe" -asm > "$dir/$exe.s"); then
    printf "\n Disassembly failed\n"
    exit 1
fi

printf "  OK\n"
printf "\n Copying old binary to $dir/$exe.old\n"
cp $dir/$exe $dir/$exe.old
printf "\n Reassembling"

if !(gcc "$dir/$exe.s" $@ -o  "$dir/$exe"); then
    echo "Reassembly failed"
    exit 1
fi

printf "  OK\n"
printf "\n Testing\n"
make check -C $dir


