#include <set>
#include <math.h>
#include <atomic>
#include <thread>
#include <sstream>

#include "png.h"
#include "zlib.h"

#include "DKohonen.hpp"
#include "ColourSpaces.hpp"
#include "imageIO.hpp"

class ColourCmprs{
    private:
size_t _numMaxColours = 0;
double _maxDiffPercent = 0;
double _minDiffPercent = 0;
uint8_t _percentage = 100;
double _learningRate = 0;
std::mt19937 _randEng = std::mt19937(std::chrono::high_resolution_clock::now().time_since_epoch().count());
DKohonen<ColourSpaces::XYZ> _colourKohonen;

double _clampLinRGB(const double& val){
    return std::min(std::max(0.0, val), 1.0);
}

ColourSpaces::LinRGB _propogateDithErrorLIN(ColourSpaces::LinRGB pixel, const double& errR, const double& errG, const double& errB, const double& coeff){
    pixel.r = _clampLinRGB(pixel.r + errR * coeff);
    pixel.g = _clampLinRGB(pixel.g + errG * coeff);
    pixel.b = _clampLinRGB(pixel.b + errB * coeff);
    return pixel;
}

void _ditheringBodyLIN(
    std::vector<ColourSpaces::LinRGB>& imgData, 
    const int& width, 
    const int& height, 
    const int& x, 
    const int& y, 
    const ColourSpaces::LinRGB& oldColour, 
    const ColourSpaces::LinRGB& newColour,
    const char& step){
    double errR = oldColour.r - newColour.r;
    double errG = oldColour.g - newColour.g;
    double errB = oldColour.b - newColour.b;
    if((0 <= (x + step)) && ((x + step) < width)){
        size_t newLoc = y * width + x + step;
        imgData[newLoc] = _propogateDithErrorLIN(imgData[newLoc], errR, errG, errB, 7.0/48.0);
        if((0 <= (x + 2 * step)) && ((x + 2 * step) < width)){
            newLoc = y * width + x + 2 * step;
            imgData[newLoc] = _propogateDithErrorLIN(imgData[newLoc], errR, errG, errB, 5.0/48.0);
        }
    }
    if(y + 1 < height){
        size_t newLoc = (y + 1) * width + x;
        imgData[newLoc] = _propogateDithErrorLIN(imgData[newLoc], errR, errG, errB, 7.0/48.0);
        if(x - 1 >= 0){
            imgData[newLoc - 1] = _propogateDithErrorLIN(imgData[newLoc - 1], errR, errG, errB, 5.0/48.0);
            if(x - 2 >= 0){
                imgData[newLoc - 2] = _propogateDithErrorLIN(imgData[newLoc - 2], errR, errG, errB, 3.0/48.0);
            }
        }
        if(x + 1 < width){
            imgData[newLoc + 1] = _propogateDithErrorLIN(imgData[newLoc + 1], errR, errG, errB, 5.0/48.0);
            if(x + 2 < width){
                imgData[newLoc + 2] = _propogateDithErrorLIN(imgData[newLoc + 2], errR, errG, errB, 3.0/48.0);
            }
        }
        if(y + 2 < height){
            newLoc = (y + 2) * width + x;
            imgData[newLoc] = _propogateDithErrorLIN(imgData[newLoc], errR, errG, errB, 5.0/48.0);
            if(x - 1 >= 0){
                imgData[newLoc - 1] = _propogateDithErrorLIN(imgData[newLoc - 1], errR, errG, errB, 3.0/48.0);
                if(x - 2 >= 0){
                    imgData[newLoc - 2] = _propogateDithErrorLIN(imgData[newLoc - 2], errR, errG, errB, 1.0/48.0);
                }
            }
            if(x + 1 < width){
                imgData[newLoc + 1] = _propogateDithErrorLIN(imgData[newLoc + 1], errR, errG, errB, 3.0/48.0);
                if(x + 2 < width){
                    imgData[newLoc + 2] = _propogateDithErrorLIN(imgData[newLoc + 2], errR, errG, errB, 1.0/48.0);
                }
            }
        }
    }
}

void _ditherDataPrepThread(const std::vector<ColourSpaces::RGB>& imgData, int y, const int& width, std::vector<ColourSpaces::LinRGB>& errorBuffer){
    for(int x = 0; x < width; x++){
        errorBuffer.push_back(imgData[y*width + x].toLinRGB());
    }
}

std::vector<ColourSpaces::RGB> _applyDithering(const std::vector<ColourSpaces::RGB>& imgData, const int& width, const int& height){
    std::vector<ColourSpaces::RGB> newImgData = imgData;
    std::vector<ColourSpaces::LinRGB> errorBuffer = std::vector<ColourSpaces::LinRGB>();
    errorBuffer.reserve(width*4);
    for(int i = 0; i < width * 3; i++){
        errorBuffer.push_back(imgData[i].toLinRGB());
    }
    int lineStart = 0;
    int lineStop = width;
    char step = 1;
    std::thread linearizer;
    for(int y = 0; y < height; y++){
        if(y + 3 < height){
            linearizer = std::thread(&ColourCmprs::_ditherDataPrepThread, this, std::cref(imgData), y + 3, std::cref(width), std::ref(errorBuffer));
        }
        for(int x = lineStart; x != lineStop; x += step){
            size_t loc = y * width + x;
            const ColourSpaces::LinRGB& oldColour = errorBuffer[x];
            ColourSpaces::LinRGB newColour = _colourKohonen.getClosestGroup(oldColour.toXYZ()).toLinRGB();
            _ditheringBodyLIN(errorBuffer, width, ((y + 2) < height)?(3):(height - y), x, 0, oldColour, newColour, step);
            newImgData[loc] = newColour.toRGB();
        }
        if(y + 3 < height){
            linearizer.join();
        }
        step = -step;
        lineStart = (lineStart == 0)?(width - 1):(0);
        lineStop = (lineStop == width)?(-1):(width);
        errorBuffer.erase(errorBuffer.begin(), errorBuffer.begin() + width);
    }
    return newImgData;
}

std::vector<unsigned char> _applyDitheringPLT(std::vector<ColourSpaces::RGB> imgData, const int& width, const int& height){
    std::vector<ColourSpaces::XYZ> palette = _colourKohonen.getGroups();
    std::vector<unsigned char> pltIndexes(imgData.size());
    std::vector<ColourSpaces::LinRGB> errorBuffer = std::vector<ColourSpaces::LinRGB>();
    errorBuffer.reserve(width*4);
    for(int i = 0; i < width * 3; i++){
        errorBuffer.push_back(imgData[i].toLinRGB());
    }
    int lineStart = 0;
    int lineStop = width;
    char step = 1;
    std::thread linearizer;
    for(int y = 0; y < height; y++){
        if(y + 3 < height){
            linearizer = std::thread(&ColourCmprs::_ditherDataPrepThread, this, std::cref(imgData), y + 3, std::cref(width), std::ref(errorBuffer));
        }
        for(int x = lineStart; x != lineStop; x += step){
            size_t loc = y * width + x;
            const ColourSpaces::LinRGB& oldColour = errorBuffer[x];
            unsigned char colourIndex = _colourKohonen.closestGroupInd(oldColour.toXYZ());
            _ditheringBodyLIN(errorBuffer, width, ((y + 2) < height)?(3):(height - y), x, 0, oldColour, palette[colourIndex].toLinRGB(), step);
            pltIndexes[loc] = colourIndex;
        }
        if(y + 3 < height){
            linearizer.join();
        }
        step = -step;
        lineStart = (lineStart == 0)?(width - 1):(0);
        lineStop = (lineStop == width)?(-1):(width);
        errorBuffer.erase(errorBuffer.begin(), errorBuffer.begin() + width);
    }
    return pltIndexes;
}

public:
ColourCmprs(const size_t& numMaxColours, const double& maxDiff, const double& minDiff, uint8_t percentage, const double& learningRate) : 
_numMaxColours(numMaxColours), 
_maxDiffPercent(maxDiff), 
_minDiffPercent(minDiff), 
_learningRate(learningRate),
_percentage(percentage){}

void process(const std::string src, const std::string dest, const bool& verbal){
    std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::time_point<std::chrono::high_resolution_clock>();
    std::chrono::time_point<std::chrono::high_resolution_clock> stop = std::chrono::time_point<std::chrono::high_resolution_clock>();
    _colourKohonen = DKohonen<ColourSpaces::XYZ>([this](const ColourSpaces::XYZ& xyz1, const ColourSpaces::XYZ& xyz2){
            return ColourSpaces::CIEDE2000(xyz1.toLAB(), xyz2.toLAB());
        });
    int height = 0;
    int width = 0;
    std::vector<ColourSpaces::RGB> rgbData = ImageIO::readImageRGB(src.c_str(), width, height);
    if(verbal == true){
        std::cout << "Image read";
    } 
    std::vector<ColourSpaces::RGB> learningRGBData = rgbData;
    std::shuffle(learningRGBData.begin(), learningRGBData.end(), _randEng);
    size_t toProcess = learningRGBData.size() * _percentage / 100.0;
    if(verbal == true){
        std::cout << std::endl << toProcess << " pixels will be processed. Started training Kohonen neural network";
        start = std::chrono::high_resolution_clock::now();
    } 
    double minDiff = 119.475 * _minDiffPercent / 100.0;
    double maxDiff = 119.475 * _maxDiffPercent / 100.0;
    for(size_t i = 0; i < toProcess; i++){
        //std::cout << toProcess << "/" << i << std::endl;
        _colourKohonen.trainStep(learningRGBData[i].toLinRGB().toXYZ(), _numMaxColours, maxDiff, minDiff, _learningRate);
    }
    learningRGBData.clear();
    learningRGBData.shrink_to_fit();
    if(verbal == true){
        stop = std::chrono::high_resolution_clock::now();
        size_t milliSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
        std::cout 
            << std::endl 
            << _colourKohonen.getGroups().size() << " unique colours identified. " 
            << "Took " << milliSeconds << " milliseconds (" << (double)milliSeconds/1000 << " seconds)" << std::endl 
            <<  "Began dithering";
        start = std::chrono::high_resolution_clock::now();
    }
    if(_colourKohonen.getGroups().size() > 256){
        std::vector<ColourSpaces::RGB> dithered = _applyDithering(rgbData, width, height);
        if(verbal == true){
            stop = std::chrono::high_resolution_clock::now();
            size_t milliSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
            std::cout 
                << std::endl 
                << "Dithering done. "
                << "Took " << milliSeconds << " milliseconds (" << (double)milliSeconds/1000 << " seconds)" << std::endl
                << "Image will be written in RGB mode";
        } 
        ImageIO::writeImageRgb(dest.c_str(), dithered, width, height);
    }
    else{
        std::vector<ColourSpaces::XYZ> xyzPallette = _colourKohonen.getGroups();
        std::vector<ColourSpaces::RGB> rgbPallete;
        for(const ColourSpaces::XYZ& valXYZ : xyzPallette){
            rgbPallete.push_back(valXYZ.toLinRGB().toRGB());
        }
        std::vector<unsigned char> dithered = _applyDitheringPLT(rgbData, width, height);
        stop = std::chrono::high_resolution_clock::now();
        size_t milliSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
        if(verbal == true){
            stop = std::chrono::high_resolution_clock::now();
            size_t milliSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
            std::cout 
                << std::endl 
                << "Dithering done. "
                << "Took " << milliSeconds << " milliseconds (" << (double)milliSeconds/1000 << " seconds)" << std::endl
                << "Image will be written in PLT mode";
        } 
        ImageIO::writeImagePLT(dest.c_str(), dithered, rgbPallete, width, height);
    }
}
};