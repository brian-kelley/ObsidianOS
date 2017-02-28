echo "*** ORIGINAL ASSEMBLY ***"
cat test.asm
nasm test.asm -o test.bin
echo "*** HEX DUMP ***"
xxd test.bin

echo "*** BIN DUMP ***"
xxd -b test.bin
