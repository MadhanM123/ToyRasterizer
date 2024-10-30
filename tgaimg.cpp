#include <iostream>
#include <fstream>
#include <string.h>
#include <time.h>
#include <math.h>

#include "tgaimg.h"

TGAImg::TGAImg() : data(NULL), width(0), height(0), bytespp(0){}

TGAImg::TGAImg(int w, int h, int bpp) : data(NULL), width(w), height(h), bytespp(bpp){
    unsigned long nbytes = width * height * bytespp;
    data = new unsigned char[nbytes];
    memset(data, 0, nbytes);
}

TGAImg::TGAImg(const TGAImg &img){
    width = img.width;
    height = img.height;
    bytespp = img.bytespp;
    unsigned long nbytes = width * height * bytespp;
    data = new unsigned char[nbytes];
    memcpy(data, img.data, nbytes);
}

TGAImg::~TGAImg(){
    if (data) delete [] data;
}

TGAImg &TGAImg::operator =(const TGAImg &img){
    if(this != &img){
        if(data) delete [] data;
        width = img.width;
        height = img.height;
        bytespp = img.bytespp;
        unsigned long nbytes = width * height * bytespp;
        data = new unsigned char[nbytes];
        memcpy(data, img.data, nbytes);
    }
    return *this;
}

bool TGAImg::read_tga_file(const char* filename){
    if(data) delete [] data;
    data = NULL;
    std::ifstream in;

    in.open(filename, std::ios::binary);
    if(!in.is_open()){
        std::cerr << "CANNOT OPEN FILE" << filename << "\n";
        in.close();
        return false;
    }

    TGA_Head head;
    in.read((char*)&head, sizeof(head));
    if(!in.good()){
        in.close();
        std::cerr << "an err occurred while reading header\n";
        return false;
    }

    width = head.width;
    height = head.height;
    bytespp = head.bitsperpix >> 3;

    if(width <= 0 || height <= 0 || (bytespp != GRAYSCALE && bytespp != RGB && bytespp != RGBA)){
        in.close();
        std::cerr << "bad bpp/width/height value\n";
        return false;
    }

    unsigned long nbytes = bytespp * width * height;
    data = new unsigned char[nbytes];

    if(3 == head.datatypecode || 2 == head.datatypecode){
        in.read((char *) data, nbytes);

        if(!in.good()){
            in.close();
            std::cerr << "an err occurred while reading data\n";
            return false;
        }
    }
    else if(10 == head.datatypecode || 11 == head.datatypecode){
        if(!load_rle_data(in)){
            in.close();
            std::cerr << "an err occurred while reading data\n";
            return false;
        }
    }
    else{
        in.close();
        std::cerr << "unknown file format " << (int)head.datatypecode << '\n';
        return false;
    }

    if(!(head.imgdescript & 0x20)){
        flip_vert();
    }
    if(head.imgdescript & 0x10){
        flip_horiz();
    }

    std::cerr << width << "x" << height << "/" << bytespp * 8 << "\n";
    in.close();
    return true;
}

bool TGAImg::load_rle_data(std::ifstream &input){
    unsigned long pixcnt = width * height;
    unsigned long curpix = 0;
    unsigned long curbyte = 0;
    TGAColor colorbuff;

    do{
        unsigned char chunkhead = 0;
        chunkhead = input.get();
        if(!input.good()){
            std::cerr << "an err occurred while reading data\n";
            return false;
        }

        if(chunkhead < 128){
            chunkhead++;
            for(int i = 0; i < chunkhead; i++){
                input.read((char *)colorbuff.raw, bytespp);

                if(!input.good()){
                    std::cerr << "An err occurred while reading data\n";
                    return false;
                }

                for(int t = 0; t < bytespp; t++){
                    data[curbyte++] = colorbuff.raw[t];
                    curpix++;

                    if(curpix > pixcnt){
                        std::cerr << "Too many pixels read\n";
                        return false;
                    }
                }
            }
        }
        else{
            chunkhead = -127;
            input.read((char *)colorbuff.raw, bytespp);
            
            if(!input.good()){
                std::cerr << "An err occurred while reading header\n";
                return false;
            }

            for(int i = 0; i < chunkhead; i++){
                for(int t = 0; t < bytespp; t++){
                    data[curbyte++] = colorbuff.raw[t];
                }
                
                curpix++;
                
                if(curpix > pixcnt){
                    std::cerr << "Too many pixels read\n";
                    return false;
                }
            }
        }
    }
    while(curpix < pixcnt);
    return true;
}

bool TGAImg::write_tga_file(const char* filename, bool rle){
    unsigned char dev_area_ref[4] = {0, 0, 0, 0};
    unsigned char exten_area_ref[4] = {0, 0, 0, 0};
    unsigned char foot[18] = {'T', 'R', 'U', 'E', 'V', 'I', 'S', 'I', 'O', 'N', '-', 'X', 'F', 'I', 'L', 'E', '.', '\0'};

    std::ofstream output;
    output.open(filename, std::ios::binary);
    if(!output.is_open()){
        std::cerr << "can't open file" << filename << '\n';
        output.close();
        return false;
    }

    TGA_Head head;
    memset((void *)&head, 0, sizeof(head));

    head.bitsperpix = bytespp << 3;
    head.width = width;
    head.height = height;
    head.datatypecode = (bytespp == GRAYSCALE ? (rle ? 11 : 3) : (rle ? 10 : 2));
    head.imgdescript = 0x20; //orig at left top

    output.write((char *)&head, sizeof(head));

    if(!output.good()){
        output.close();
        std::cerr << "Can't dump tga file\n";
        return false;
    }

    if(!rle){
        output.write((char *)data, width*height*bytespp);
        if(!output.good()){
            output.close();
            std::cerr << "can't unload raw data\n";
            return false;
        }
    }
    else{
        if(!unload_rle_data(output)){
            output.close();
            std::cerr << "can't unload rle data\n";
            return false;
        }
    }

    output.write((char *)dev_area_ref, sizeof(dev_area_ref));
    if(!output.good()){
        output.close();
        std::cerr << "can't dump tga file\n";
        return false;
    }

    output.write((char *)exten_area_ref, sizeof(exten_area_ref));
    if(!output.good()){
        output.close();
        std::cerr << "can't dump tga file\n";
        return false;
    }

    output.write((char *)foot, sizeof(foot));
        if(!output.good()){
        output.close();
        std::cerr << "can't dump tga file\n";
        return false;
    }

    output.close();
    return true;
}


bool TGAImg::unload_rle_data(std::ofstream& output){
    const unsigned char max_chunk_len = 128;
    unsigned long npix = width * height;
    unsigned long curpix = 0;

    while(curpix < npix){
        unsigned long chunkstart = curpix * bytespp;
        unsigned long curbyte = curpix * bytespp;
        unsigned char runlen = 1;
        bool raw = true;

        while(curpix + runlen < npix && runlen < max_chunk_len){
            bool succ_eq = true;
            for(int t = 0; t < bytespp && succ_eq; t++){
                succ_eq = (data[curbyte + t] == data[curbyte + t + bytespp]);
            }

            curbyte += bytespp;
            
            if(runlen == 1){
                raw = !succ_eq;
            }

            if(succ_eq && raw){
                runlen--;
                break;
            }

            if(!raw && !succ_eq){
                break;
            }

            runlen++;
        }

        curpix += runlen;
        output.put(raw ? runlen - 1: runlen + 127);

        if(!output.good()){
            std::cerr << "can't dump tga file\n";
            return false;
        }

        output.write((char *)(data + chunkstart), (raw ? runlen * bytespp: bytespp));
        if(!output.good()){
            std::cerr << "can't dump tga file\n";
            return false;
        }
    }

    return true;
}

TGAColor TGAImg::get(int x, int y){
    if(!data || x < 0 || y < 0 || x >= width || y >= height){
        return TGAColor();
    }

    return TGAColor(data + (x + y * width)*bytespp, bytespp);
}

bool TGAImg::set(int x, int y, TGAColor c){
    if(!data || x < 0 || y < 0 || x >= width || y >= height){
        return false;
    }

    memcpy(data + (x + y * width)*bytespp, c.raw, bytespp);
    return true;
}

int TGAImg::get_height(){
    return height;
}

int TGAImg::get_bytespp(){
    return bytespp;
}

int TGAImg::get_width(){
    return width;
}

bool TGAImg::flip_horiz(){
    if(!data) return false;

    int half = width >> 1;
    for(int i = 0; i < half; i++){
        for(int j = 0; j < height; j++){
            TGAColor c1 = get(i, j);
            TGAColor c2 = get(width - 1 - i, j);
            set(i, j, c2);
            set(width - 1 - i, j, c1);
        }
    }

    return true;
}

bool TGAImg::flip_vert(){
    if(!data) return false;

    unsigned long bytespl = width * bytespp;
    unsigned char* line = new unsigned char[bytespl];
    int half = height >> 1;

    for(int i = 0; i < half; i++){
        unsigned long l1 = i * bytespl;
        unsigned long l2 = (height - 1 - i) * bytespl;

        memmove((void *)line, (void *)(data + l1), bytespl);
        memmove((void *)(data + l1), (void *)(data + l2), bytespl);
        memmove((void *)(data + l2), (void *)line, bytespl);
    }

    delete [] line;
    return true;
}

unsigned char* TGAImg::buffer(){
    return data;
}

void TGAImg::clear(){
    memset((void *) data, 0, width*height*bytespp);
}

bool TGAImg::scale(int w, int h){
    if(w <= 0 || h <= 0 || !data){
        return false;
    }

    unsigned char* tdata = new unsigned char[w*h*bytespp];
    int nscanline = 0;
    int oscanline = 0;
    int dify = 0;

    unsigned long nlinebytes = w*bytespp;
    unsigned long olinebytes = width*bytespp;

    for(int j = 0; j < height; j++){
        int difx = width - w;
        int nx = -bytespp;
        int ox = -bytespp;

        for(int i = 0; i < width; i++){
            ox += bytespp;
            difx += w;

            while(difx >= (int)width){
                difx -= width;
                nx += bytespp;
                memcpy(tdata + nscanline + nx, data + oscanline + ox, bytespp);
            }
        }

        dify += h;
        oscanline += olinebytes;

        while(dify >= (int)height){
            if(dify >= (int)height << 1){
                memcpy(tdata + nscanline + nlinebytes, tdata + nscanline, nlinebytes);
            }
            dify -= height;
            nscanline += nlinebytes;
        }
    }

    delete [] data;
    data = tdata;
    width = w;
    height = h;

    return true;
}
