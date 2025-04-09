
extern "C"

#define _USE_MATH_DEFINES
#include <iostream>
#include <cstdio>
#include "include/ColourCmprs.hpp"

using namespace std;

void showHelp(){
    cout 
        << "Help:" << endl
        << "chromini <max_colors> <learning_portion> <difference_threshold> <sameness_threshold> <learning_rate> <input> <output>" << endl
        << "ONLY OPAQUE PNG FILES ARE SUPPORTED" << endl
        << "max_colors - maximum amount of colours ([1; 256] as PLT; >256 for SRGB)" << endl
        << "learning_portion - percent of the image to learn from [1; 100]" << endl
        << "difference_threshold - difference threshold percentage. Specifies how different two colours should be to be considered unique [1; 100]" << endl
        << "sameness_threshold - sameness threshold percentage. Specifies how different two colours should be to be considered same for removal [1; 100]" << endl
        << "learning_rate - colour learning rate [0; 1]" << endl
        << "input - path to input file" << endl
        << "output - path to output file";
}

int main(int argc, char* argv[])
{
    int numColours = 16;
    int learnPercent = 50;
    double diffPercentage = 50;
    double samenessPercentage = 50;
    double learningRate = 0.0001;
    if(argc != 8){
        showHelp();
        return 0;
    }
    else{
        numColours = atoi(argv[1]);
        if(numColours < 1){
            showHelp();
            return 0;
        }
        learnPercent = atoi(argv[2]);
        if((learnPercent < 0) || (learnPercent > 100)){
            showHelp();
            return 0;
        }
        diffPercentage = atof(argv[3]);
        if((diffPercentage < 1.0) || (diffPercentage > 100.0)){
            showHelp();
            return 0;
        }
        samenessPercentage = atof(argv[3]);
        if((samenessPercentage < 1.0) || (samenessPercentage > 100)){
            showHelp();
            return 0;
        }
        learningRate = atof(argv[5]);
        if(learningRate <= 0) || (learningRate > 1){
            showHelp();
            return 0;
        }
    }
    ColourCmprs imgCmprs(numColours, diffPercentage, samenessPercentage, learnPercent, learningRate);
    try{
        imgCmprs.process(argv[6], argv[7], true);
    }
    catch(const runtime_error& e){
        cout 
        << "Our deepest condolences. An error has occured while processing your file" << endl
        << "Error: " << e.what();
        return 1;
    }
}
