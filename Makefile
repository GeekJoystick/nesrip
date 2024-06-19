OUTPUT = nesrip.exe

all:
	tcc -o $(OUTPUT) src/*.c src/*/*.c -g

clean:
	del *.exe