# Chromini - Image Dithering Tool

Chromini is a console-based C++ program designed to dither images. It supports only opaque PNG files and utilizes `libpng` and `zlib` for image processing. This project is part of my training in creating a finished, usable project.

## Table of Contents

- [Features](#features)
- [Installation](#installation)
- [Usage](#usage)
- [Dependencies](#dependencies)
- [Building the Project](#building-the-project)
- [Credits](#credits)
- [License](#license)

## Features

- Dithering of opaque PNG images.
- Customizable color palette size.
- Adjustable learning rate and thresholds for color differentiation.
- Easy-to-use command-line interface.

## Installation

To use Chromini, you need to have a C++ compiler installed on your system. The program has been tested with `g++`, but other compilers should work as well.

## Usage

Chromini can be run from the command line with the following syntax:

```sh
chromini <max_colors> <learning_portion> <difference_threshold> <sameness_threshold> <learning_rate> <input> <output>
```

### Parameters

- **max_colors**: Specifies the maximum number of colors in the palette. Range: [1; 256] for PLT, >256 for SRGB.
- **learning_portion**: Specifies the percentage of the image to learn from. Range: [1; 100].
- **difference_threshold**: Specifies how different two colors should be to be considered unique. Range: [1; 100].
- **sameness_threshold**: Specifies how different two colors should be to be considered the same for duplicate removal Range: [1; 100].
- **learning_rate**: Specifies the rate at which colors are learned. Range: [0; 1].
- **input**: Path to input file
- **output**: Path to output file

### Example

```sh
chromini 16 50 15 5 0.0001 input.png output.png
```

This command will dither the `input.png` image with a maximum of 16 colors, learning from 50% of the image, with a difference threshold of 15%, a sameness threshold of 5%, and a color learning rate of 0.0001. The output will be saved as `output.png`.

## Dependencies

Chromini relies on the following libraries:

- `libpng`: For reading and writing PNG files.
- `zlib`: For compression and decompression of PNG files.

They are included into the project as submodules

## Building the Project

Building must be done in a separate directory.
Assuming chromini/your_dir:

```sh
cmake ..
cmake --build .
```

## Credits

- `libpng`: [libpng](https://github.com/pnggroup/libpng)
- `zlib`: [zlib](https://github.com/madler/zlib)

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.