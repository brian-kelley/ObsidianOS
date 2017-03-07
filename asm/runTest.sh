./asm.exe
echo "ASSEMBLY:"
cat test.asm
echo "BINARY:"
./printbin.exe test.bin
echo "DISASSEMBLY:"
ndisasm -u test.bin

