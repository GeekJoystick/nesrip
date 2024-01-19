OUTPUT = nesrip.exe

all:
	tcc -o $(OUTPUT) src/*.c src/*/*.c

clean:
	del *.exe

run:
	./$(OUTPUT) './roms/Spacegulls-1.1.nes'
	pause