# Lines of code counter

This is a simple program written in C to count the amount of lines inside each filetype in any given directory or file.
Example output for a single directory:
```
LOCC version 2025.5.28

Info for 'locc'
===============
        total:  3 files | lines: 529 code | 117 empty | (646 total)
        .c:     2 files | lines: 456 code | 104 empty | (560 total)
        .h:     1 files | lines: 73 code | 13 empty | (86 total)

```

Example output for multiple directories:
```
LOCC version 2025.5.28

Info for 'src'
==============
        total:  2 files | lines: 456 code | 104 empty | (560 total)
        .c:     2 files | lines: 456 code | 104 empty | (560 total)

Info for 'inc'
==============
        total:  1 files | lines: 73 code | 13 empty | (86 total)
        .h:     1 files | lines: 73 code | 13 empty | (86 total)

Info Summary
============
        3 files
        529 lines of code
        117 empty lines
        (646 total lines)

```

## Building
Create a build directory and run cmake inside that directory like this:
```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=<Release/Debug>
```
> Replace `<Release/Debug>` with whichever build type you want, `Release` would be recommended.

Now while still in the `build` directory, build the project like this:
```bash
cmake --build .
```

If you want you can also install it directly using make (assuming your operating system supports installing it with cmake).
To install just run:
```bash
cmake --build . --target install
```
> [!NOTE]
> `sudo` may required in order to install to the default prefix `/usr/local` on linux/mac systems.
> If you want a custom prefix (and possibly avoid using sudo),
> add `-DCMAKE_INSTALL_PREFIX=<your prefix here>` to the first cmake command.

## Usage
To use locc, just specify a path and let it handle the rest.
If you want to ignore files that are detected as miscellaneous files,
add the `-i`/`--ignore-misc` flag to the command line arguments.
This is how your command should look like: (`-h` displays a help message, arguments in `[]` are optional)
`locc [-i/--ignore-misc] [-h] <path1> <path2> ....`
