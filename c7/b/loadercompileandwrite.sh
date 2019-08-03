nasm -I ./include/ -f bin loader.S -o loader.bin && dd if=./loader.bin of=/usr/local/bochs/hd60M.img bs=512 count=4 seek=2 conv=notrunc
