#ifndef IMAGE_IO_HPP
#define IMAGE_IO_HPP

#include <vector>

namespace ImageIO {
    std::vector<ColourSpaces::RGB> readImageRGB(const char* filename, int& width, int& height){
        FILE* fp = nullptr;
        errno_t err;
        fp = fopen(filename, "rb");
        if (!fp) {
            throw std::runtime_error("File \"" + std::string(filename) + "\" could not be found");
        }
        png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        png_infop info = png_create_info_struct(png);
        if(setjmp(png_jmpbuf(png))){
            throw std::runtime_error("File reading error");
        }
        png_init_io(png, fp);
        png_read_info(png, info);
        width = png_get_image_width(png, info);
        height = png_get_image_height(png, info);
        png_byte color_type = png_get_color_type(png, info);
        png_byte bit_depth = png_get_bit_depth(png, info);
        if(color_type == PNG_COLOR_TYPE_PALETTE){
            png_set_palette_to_rgb(png);
        }
        else if(color_type == PNG_COLOR_TYPE_GRAY){
            png_set_gray_to_rgb(png);
        }
        else if(color_type != PNG_COLOR_TYPE_RGB){
            throw std::runtime_error("Supports only rgb, gray and palette");
        }
        png_read_update_info(png, info);
        std::vector<ColourSpaces::RGB> rgbData(height * width);
        for (int y = 0; y < height; y++) {
            std::vector<png_byte> buffer(width * 3);
            png_read_row(png, reinterpret_cast<png_bytep>(buffer.data()), NULL);
            for (int x = 0; x < width; x++) {
                rgbData[y * width + x].r = buffer[x * 3];
                rgbData[y * width + x].g = buffer[x * 3 + 1];
                rgbData[y * width + x].b = buffer[x * 3 + 2];
            }
        }
        fclose(fp);
        if (png && info) {
            png_destroy_read_struct(&png, &info, (png_infopp)NULL);
        }
        return rgbData;
    } 

    void writeImageRgb(const char* filename, const std::vector<ColourSpaces::RGB>& rgbData, const int& width, const int& height) {
        FILE* fp = nullptr;
        errno_t err;
        fp = fopen(filename, "wb");
        if (!fp) {
            throw std::runtime_error("File \"" + std::string(filename) + "\" could not be made");
        }
        png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        png_infop info = png_create_info_struct(png);
        if(setjmp(png_jmpbuf(png))){
            throw std::runtime_error("Png writing error");
        };
        png_init_io(png, fp);
        png_set_IHDR(
            png,
            info,
            width, height,
            8,
            PNG_COLOR_TYPE_RGB,
            PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_BASE,
            PNG_FILTER_TYPE_BASE
        );
        png_set_compression_mem_level(png, MAX_MEM_LEVEL);
        png_set_compression_level(png, Z_BEST_COMPRESSION);
        png_set_compression_strategy(png, Z_DEFAULT_STRATEGY);
        png_set_compression_window_bits(png, 15);
        png_set_compression_buffer_size(png, rgbData.size()*3);
        png_write_info(png, info);
        for (int y = 0; y < height; y++) {
            std::vector<png_byte> buffer(width * 3);
            for (int x = 0; x < width; x++) {
                buffer[x * 3] = rgbData[y * width + x].r;
                buffer[x * 3 + 1] = rgbData[y * width + x].g;
                buffer[x * 3 + 2] = rgbData[y * width + x].b;
            }
            png_write_row(png, buffer.data());   
        }
        png_write_end(png, NULL);
        fclose(fp);
        if (png && info) {
            png_destroy_read_struct(&png, &info, (png_infopp)NULL);
        }
    }

    void writeImagePLT(const char* filename, const std::vector<unsigned char>& pltIndexes, const std::vector<ColourSpaces::RGB>& pallete, const int& width, const int& height){
        png_colorp bytePlt = new png_color[pallete.size()];
        for(int i = 0; i < pallete.size(); i++){
            bytePlt[i].red = pallete[i].r;
            bytePlt[i].green = pallete[i].g;
            bytePlt[i].blue = pallete[i].b;
        }
        FILE* fp = nullptr;
        errno_t err;
        fp = fopen(filename, "wb");
        if (!fp) {
            throw std::runtime_error("File \"" + std::string(filename) + "\" could not be made");
        }
        png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        png_infop info = png_create_info_struct(png);
        if(setjmp(png_jmpbuf(png))){
            throw std::runtime_error("Png writing error");
        };
        png_init_io(png, fp);
        png_set_IHDR(
            png,
            info,
            width, height,
            8,
            PNG_COLOR_TYPE_PALETTE,
            PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_BASE,
            PNG_FILTER_TYPE_BASE
        );
        png_set_compression_mem_level(png, MAX_MEM_LEVEL);
        png_set_compression_level(png, Z_BEST_COMPRESSION);
        png_set_compression_strategy(png, Z_DEFAULT_STRATEGY);
        png_set_compression_window_bits(png, 15);
        png_set_compression_buffer_size(png, pltIndexes.size());
        png_set_PLTE(png, info, bytePlt, pallete.size());
        png_write_info(png, info);
        for(int y = 0; y < height; y++){
            png_write_row(png, &pltIndexes[y * width]);
        }
        png_write_end(png, info);
        delete bytePlt;
    }
}

#endif