OUTPUT = nesrip.exe

all:
	tcc -o $(OUTPUT) src/*.c src/*/*.c

clean:
	del *.exe