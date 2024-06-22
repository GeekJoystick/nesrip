# nesrip

This tool automatically rips graphics from a supplied NES rom into PNG sheets using a legibility palette of black, white, orange, and teal. It also has a built-in tile deduplicator that, by default, runs across sheets and removes duplicate tiles. People who create spritesheets, write PC ports, or recreate maps in TMX may find this program useful. This tool was commissioned by FitzRoyX.

If you want to help add entries to the database, familiarize yourself with the included `nes_gfxdb.txt` file and YYCHR.

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

#### Command-Line usage

```
nesrip.exe file [arguments]                                                                                      
Arguments:
 -S {start address} {end address}                 Directly rip graphics from specified memory in ROM.
 -o {filename}                                    Output filename (without file extension) when using -S.
 -d {filename}                                    Graphics database filename.
 -c {compression type, raw}                       Graphics decompression algorithm.
 -p {pattern size, 1/2/4/8/16} {direction, h/v}   Set or override tile block size and direction.
 -i {4 letter combination of b/o/t/w}             Set or override palette order for rendering.
 -b {bitplane type, 1/2}                          Set or override bitplane type
 -r {redundancy check enable, true/false}         Set or override enabling redundancy checks
```

### Drag-n-Drop usage

An included database file is used by **nesrip** to match the given ROM to a set of commands written for extracting its graphics.
The file contains a number of **description blocks**, each of them containing the unheadered SHA-256 hash of a ROM, and a list of commands that determine palette indices, pattern sizes, decompression algorithms and finally the ROM addresses of the graphics data. The commands sandwiched between "hash" and "end" mostly mirror the ones described in the command-line section.

### Compression types

* `raw`: Uncompressed graphics

#### Note

> No compression algorithm is supported at the moment

### Example database file

```
//Spacegulls
hash b69bd1809e26400336af288bc04403c00d77030b931bc31875594c9a0ae92f67
p 1 h
i botw
s bg 00000 ffff
end

//Micro Mages
hash a4b5b736a84b260314c18783381fe2dca7b803f7c29e78fb403a0f9087a7e570
p 1 v
i btow
s spr 8000 8fff
s bg 9000 9fff
end
```

#### Note

> Text written outside of a **description block** is ignored by the tool, though the latter will throw errors if you put text other than commands in a **description block**.

## Version History

* 0.3
	* Added bitplane types
	* Added 1bpp bitplane
	* Added tile redundancy checks
* 0.2
	* Made ROM headers be ignored from any operation.
	* Hashes are now case insensitive.
	* Made output files go into a folder named after the input ROM.
	* Added batch files for processing multiple ROMs.
	* Added pattern directionnality.
	* Sheets will now extend vertically instead of splitting in multiple sheets.
	* Added a pattern size of 16 to allow for fully vertical 128x128 sections.
* 0.1
	* Initial Release.

## License

MIT - © 2024 [Matys Guéroult](https://github.com/GeekJoystick)
