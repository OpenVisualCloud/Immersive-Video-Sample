#include <boost/program_options.hpp>
#include "WebRTC360HEVCTilesMerger.h"

#define HWIDTH4K 3840
#define HHEIGHT4K 2048
#define LWIDTH4K 1280
#define LHEIGHT4K 768
#define HWIDTH8K 7680
#define HHEIGHT8K 3840
#define LWIDTH8K 1280
#define LHEIGHT8K 512

namespace po = boost::program_options;

bool ReadBuffer(int* bufferSize, bool* isKey, uint8_t** buffer, FILE* fp) {

    int result = 0;
    fread(bufferSize, sizeof(int), 1, fp);
    fread(isKey, sizeof(bool), 1, fp);
    *buffer = (uint8_t*)malloc(sizeof(uint8_t)*(*bufferSize));
    if (!*buffer) {
        std::cout << "Memory allocate error" << std::endl;
        return false;
    }

    memset(*buffer, 0, sizeof(uint8_t)*(*bufferSize));
    result = fread(*buffer, *bufferSize, 1, fp);
    if (!result) {
        std::cout << "Reading error: " << std::endl;
        free(*buffer);
        *buffer = NULL;
        return false;
    }

    return true;
}

void DisplayPayloadByOffset(uint8_t* src, int offset) {

    uint32_t payload = src[offset + 0]
                     | src[offset + 1] << 8
                     | src[offset + 2] << 16
                     | src[offset + 3] << 24;
    std::cout << "Payload start with offset " << offset << " : " << payload << std::endl;
}

void ConcatPointerUint8(uint8_t* src, uint8_t** target, int uint8count, int* offset) {

    std::copy(src, src+uint8count, *target+*offset);
    *offset += uint8count;

}

void LoadBuffer(uint32_t width, uint32_t height, uint32_t bufferSize,
                uint8_t* buffer, Frame* frame, int* offset) {

    uint8_t widthArray[4] = { 0 }, heightArray[4] = { 0 }, lengthArray[4] = { 0 };
    memcpy(widthArray, &width, sizeof(uint32_t));
    memcpy(heightArray, &height, sizeof(uint32_t));
    memcpy(lengthArray, &bufferSize, sizeof(uint32_t));
    ConcatPointerUint8(widthArray, &frame->payload, 4, offset);
    ConcatPointerUint8(heightArray, &frame->payload, 4, offset);
    ConcatPointerUint8(lengthArray, &frame->payload, 4, offset);
    ConcatPointerUint8(buffer, &frame->payload, bufferSize, offset);

}

int main(int argc, char **argv) {

    std::string hiFileToLoad, lowFileToLoad;
    std::string resolution;
    int32_t output_frames = 30;

    try {

        po::options_description desc("Allowd options");
        desc.add_options()
            ("help", "produce help message")
            ("frame_num,f", po::value<int>(), "set frames")
            ("resolution,r", po::value<std::string>(), "set resolution")
            ("high_res_file,h", po::value<std::string>(), "set file path")
            ("low_res_file,l", po::value<std::string>(), "set fiel path");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if(vm.count("help")) {
            std::cout << desc << std::endl;
            return 0;
        }

        if(vm.count("frame_num")) {
            output_frames = vm["frame_num"].as<int>();
        }

        if(vm.count("resolution")) {
            resolution = vm["resolution"].as<std::string>();
        }

        if(vm.count("high_res_file")) {
            hiFileToLoad = vm["high_res_file"].as<std::string>();
        }

        if(vm.count("low_res_file")) {
            lowFileToLoad = vm["low_res_file"].as<std::string>();
        }

    }
    catch(std::exception& e) {
        std::cerr << "error" << e.what() << std::endl;
        return 1;
    }
    catch(...) {
        std::cerr << "Exception of unknown type!" << std::endl;
        return 1;
    }

    int hWidth = 0, hHeight = 0, lWidth = 0, lHeight = 0;
    if (resolution == "4K") {
        hWidth = HWIDTH4K;
        hHeight = HHEIGHT4K;
        lWidth = LWIDTH4K;
        lHeight = LHEIGHT4K;
    } else if (resolution == "8K") {
        hWidth = HWIDTH8K;
        hHeight = HHEIGHT8K;
        lWidth = LWIDTH8K;
        lHeight = LHEIGHT8K;
    } else {
        std::cerr << "Resolution \"-r\" must be 4K or 8K" << std::endl;
        return 1;
    }

    FILE* fpHigh = fopen(hiFileToLoad.c_str(), "r");
    FILE* fpLow = fopen(lowFileToLoad.c_str(), "r");

    VideoFrameSpecificInfo* highResInfo = new VideoFrameSpecificInfo();
    memset(highResInfo, 0, sizeof(VideoFrameSpecificInfo));
    highResInfo->width = (uint16_t)hWidth;
    highResInfo->height = (uint16_t)hHeight;
    highResInfo->isKeyFrame = false;
    VideoFrameSpecificInfo* lowResInfo = new VideoFrameSpecificInfo();
    memset(lowResInfo, 0, sizeof(VideoFrameSpecificInfo));
    lowResInfo->width = (uint16_t)lWidth;
    lowResInfo->height = (uint16_t)lHeight;
    lowResInfo->isKeyFrame = false;

    Frame* inFrame = new Frame();
    memset(inFrame, 0, sizeof(Frame));
    inFrame->payload = NULL;
    inFrame->format = FRAME_FORMAT_H265;
    memcpy(&inFrame->additionalInfo, highResInfo, sizeof(VideoFrameSpecificInfo));

    HEVCTilesMerger* tilesMerger = new HEVCTilesMerger();

    int offset = 0, index = 0, hiBufferSize = 0, lowBufferSize = 0;
    uint32_t hiWidth = hWidth, hiHeight = hHeight;
    uint32_t lowWidth = lWidth, lowHeight = lHeight;
    bool hiIsKey = NULL, lowIsKey = NULL;
    uint8_t* hiBuffer = NULL;
    uint8_t* lowBuffer = NULL;

    while (!feof(fpHigh) || !feof(fpLow)) {

        if (index == output_frames) {
            break;
        }
        index++;

        if (!ReadBuffer(&hiBufferSize, &hiIsKey, &hiBuffer, fpHigh)) {
            std::cerr << "High res read buffer failed" << std::endl;
        } else {

            int payloadSize = sizeof(int)*(hiBufferSize) + 12;
            inFrame->payload = (uint8_t*)malloc(payloadSize);
            memset(inFrame->payload, 0, payloadSize);

            LoadBuffer(hiWidth, hiHeight, hiBufferSize, hiBuffer, inFrame, &offset);

            free(hiBuffer);
            hiBuffer = NULL;
        }

        if (!ReadBuffer(&lowBufferSize, &lowIsKey, &lowBuffer, fpLow)) {
            std::cerr << "Low res read buffer failed" << std::endl;
        } else {

            LoadBuffer(lowWidth, lowHeight, lowBufferSize, lowBuffer, inFrame, &offset);

            if (hiIsKey == lowIsKey) {
                uint8_t isKeyFrame[1] = { 0 };
                memcpy(&inFrame->additionalInfo.video.isKeyFrame, &lowIsKey, sizeof(bool));
                ConcatPointerUint8(isKeyFrame, &inFrame->payload, 1, &offset);
            } else {
                std::cout << "Key frame info mismatch!" << std::endl;
            }

            free(lowBuffer);
            lowBuffer = NULL;
        }

        Frame outFrame = *inFrame;
        tilesMerger->onFrame(*inFrame, &outFrame);
        free(inFrame->payload);
        inFrame->payload = NULL;
        offset = 0;

    }

    fclose(fpHigh);
    fclose(fpLow);
    delete inFrame;
    delete highResInfo;
    delete lowResInfo;
    std::cout << std::endl << " --------- Done ------------ " << std::endl;

    return 0;
}
