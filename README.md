# Image Processor

## How to run the main program

Firsly, you need to build your code%

```bash
cmake -S . -B build
cmake --build build
```

The main console application is called **`image_processor`**.
After building the project, run it from the terminal like this:

```bash
./build/image_processor <input_file> <output_file> [filters]
```

### Simple example

```bash
./build/image_processor input.bmp output.bmp -gs
```

This command:
1. reads `input.bmp`
2. applies the grayscale filter
3. saves the result to `output.bmp`

### Example with several filters

```bash
./build/image_processor input.jpg output.png -crop 800 600 -gs -blur 1.5 -contrast 1.2
```

This command:
1. loads `input.jpg`
2. crops the image to `800x600`
3. converts it to grayscale
4. applies Gaussian blur with sigma `1.5`
5. increases contrast with coefficient `1.2`
6. saves the result to `output.png`

### Important note about the entry point

In the current `CMakeLists.txt`, the console executable is built from:

- `src/main.cpp`

So when you launch `image_processor`, this is the file that is actually used as the entry point.

There is also a file named `image_processor.cpp` in the project root, but **it is not used by the current build configuration unless you explicitly change `CMakeLists.txt`**.

---

## Project overview

This project is an image processing application written in **C++17**.  
It contains:

- a **console application** for applying filters from the command line;
- a **Qt GUI application** for interactive image processing;
- a small set of **unit tests** for core components.

The project supports loading, processing, and saving images in several formats.

---

## Supported image formats

The current implementation supports:

- **BMP**
- **PNG**
- **JPEG / JPG**
- **HEIC**

Input and output format are determined by the file extension.

Examples:
- `input.bmp -> output.bmp`
- `input.jpg -> output.png`
- `input.heic -> output.jpeg`

---

## Available filters

Filters are passed in the command line in the order they should be applied.

### 1. Crop

```bash
-crop <width> <height>
```

Cuts the image to the specified width and height using the top-left part of the image.

Example:

```bash
./build/image_processor input.png output.png -crop 500 300
```

---

### 2. Grayscale

```bash
-gs
```

Converts the image to grayscale.

Example:

```bash
./build/image_processor input.png output.png -gs
```

---

### 3. Negative

```bash
-neg
```

Creates a negative of the image.

Example:

```bash
./build/image_processor input.png output.png -neg
```

---

### 4. Sharpening

```bash
-sharp
```

Sharpens the image.

Example:

```bash
./build/image_processor input.png output.png -sharp
```

---

### 5. Edge Detection

```bash
-edge <threshold>
```

Detects edges in the image. The image is converted to grayscale first.

Example:

```bash
./build/image_processor input.png output.png -edge 0.2
```

---

### 6. Gaussian Blur

```bash
-blur <sigma>
```

Applies Gaussian blur.

Example:

```bash
./build/image_processor input.png output.png -blur 2.0
```

---

### 7. Glow

```bash
-glow <threshold> <radius> <intensity>
```

Applies a glow effect.

Parameters:
- `threshold` — brightness threshold for glowing areas
- `radius` — glow radius
- `intensity` — glow strength

Example:

```bash
./build/image_processor input.png output.png -glow 0.7 6 1.3
```

---

### 8. Contrast

```bash
-contrast <coefficient>
```

Adjusts image contrast.

Example:

```bash
./build/image_processor input.png output.png -contrast 1.2
```

---

## Command line format

General syntax:

```bash
./build/image_processor <input_file> <output_file> [-filter_name [args...]] ...
```

Example:

```bash
./build/image_processor input.bmp output.bmp -crop 800 600 -gs -blur 0.5
```

Filters are applied **from left to right**, in the same order as they appear in the command.

If no filters are passed, the image is simply loaded and saved unchanged.

---

## Building the project

### Requirements

You need:

- **C++17 compiler**
- **CMake 3.16+**
- **Qt6** (`Widgets`, `Gui`) for the GUI application
- **libpng**
- **libjpeg**
- **libheif**
- **pkg-config**

### Build commands

```bash
cmake -S . -B build
cmake --build build
```

After that, the build directory will contain executables such as:

- `build/image_processor` — console application
- `build/image_processor_gui` — GUI application
- `build/unit_tests` — tests

---

## Running the GUI application

To start the graphical interface:

```bash
./build/image_processor_gui
```

The GUI allows you to:
- open an image from disk;
- drag and drop an image into the window;
- choose filters from a dropdown list;
- set filter parameters;
- apply processing;
- save the result.

---

## Running unit tests

If tests were built successfully, run:

```bash
./build/unit_tests
```

---

## Project structure

A simplified structure of the project:

```text
.
├── CMakeLists.txt
├── image_processor.cpp
├── readme.md
├── tests.cpp
├── include/
└── src/
    ├── main.cpp
    ├── gui_main.cpp
    ├── main_window.cpp
    ├── parser.cpp
    ├── processor.cpp
    ├── image.cpp
    ├── image_io.cpp
    ├── bmp.cpp
    ├── png_codec.cpp
    ├── jpeg_codec.cpp
    ├── heic_codec.cpp
    ├── filters.cpp
    ├── filter_factory.cpp
    ├── pipeline.cpp
    └── pipeline_builder.cpp
```

---

## Error handling

The program uses exceptions for invalid arguments, unsupported formats, and file I/O errors.

Examples of handled errors:
- wrong number of filter parameters;
- unknown filter name;
- unsupported file format;
- broken or unreadable input file;
- failure to save output file.

---

## Current limitations

A few practical notes:

1. The console executable is currently launched from `src/main.cpp`, not from the root `image_processor.cpp`.
2. `Qt6` is required by the current `CMakeLists.txt`, even if you only want the CLI build.
3. Help output on empty launch may require a small improvement depending on which version of `main.cpp` is currently in your project.

---

## Example workflow

```bash
cmake -S . -B build
cmake --build build
./build/image_processor input.jpg output.png -crop 1024 768 -sharp -contrast 1.15
```

This is the easiest way to build and run the main console program.
