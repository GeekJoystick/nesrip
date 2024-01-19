# nesrip

This tool takes in a ROM and a graphics database file *(described further down)*, and automatically extracts graphics from the ROM.

## Installation

### Pre-built binaries

Pre-built binaries can be downloaded from the [GitHub Releases section](https://github.com/GeekJoystick/nesrip/releases/latest)

### Compilation

#### Dependencies

* Windows 10
* [TCC](https://github.com/TinyCC/tinycc)
* [Make](https://www.gnu.org/software/make)

### Building

* Clone or download the repository
* Run `make` in the root folder of the project

### Executing program

#### Usage

```
nesrip.exe file [arguments]                                                                                      
Arguments:
-S {start address} {end address}        Directly rip graphics from specified memory in ROM.
-o {filename}                           Output filename (without file extension) when using -S.
-d {filename}                           Graphics database filename.
-c {compression type, raw}              Graphics decompression algorithm.
-p {pattern size, 1/2/4/8}              Set or override tile block size.   
-i {4 letter combination of b/o/t/w}    Set or override palette order for rendering.
```

### Graphics database file

The Graphics database file is used by **nesrip** to match the given ROM to a set of commands written for extracting its graphics.
The file contains a number of **description blocks**, each of them containing a SHA-256 hash of a ROM, and a list of commands that determine palette indices, pattern sizes, decompression algorithms and finally the ROM addresses of the graphics data.

#### Note

> By default the tool will look for a graphics database file called `nes_gfxdb.txt`.
> You can change this by using the `-d` argument when running **nesrip**

> You also must provide the database file yourself.

### Graphics data base commands

```
Hash {ROM SHA-256 hash}
EndHash
Section {Start address} {End address}
Pattern {Pattern size, 1/2/4/8}
Palette {4 letter combination of b/o/t/w}
Compression {compression type, raw}
```

### Compression types

* `raw`: Uncompressed graphics

#### Note

> No compression algorithm is supported at the moment

### Example database file

```
//Spacegulls
Hash B69BD1809E26400336AF288BC04403C00D77030B931BC31875594C9A0AE92F67
Pattern 1
Palette botw
Section bg 00010 1000F
EndHash

//Micro Mages
Hash A4B5B736A84B260314C18783381FE2DCA7B803F7C29E78FB403A0F9087A7E570
Pattern 1
Palette btow
Section spr 8010 900F
Section bg 9010 A00F
EndHash

...
```

#### Note

> Text written outside of a **description block** is ignored by the tool, though the latter will throw errors if you put text other than commands in a **description block**.

## Version History

* 0.1
	* Initial Release

## License

MIT - © 2024 [Matys Guéroult](https://github.com/GeekJoystick)