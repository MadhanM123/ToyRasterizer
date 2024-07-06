#ifndef IMG_H
#define IMG_H

#include <iostream>
#include <fstream>

#pragma pack(push, 1)

struct TGA_Head{
    char idlen;
    char colormaptype;
    char datatypecode;
    short colormaporig;
    short colormaplen;
    char colormapdepth;
    short x_orig;
    short y_orig;
    short width;
    short height;
    char bitsperpix;
    char imgdescript;
};

#pragma pack(pop)

struct TGAColor{
    union{
        struct{
            unsigned char b, g, r, a;
        };
        unsigned char raw[4];
        unsigned int val;
    };
    int bytespp;

    TGAColor() : val(0), bytespp(1){}

    TGAColor(unsigned char R, unsigned char G, unsigned char B, unsigned char A) : b(B), g(G), r(R), a(A), bytespp(4){}

    TGAColor(int v, int bpp) : val(v), bytespp(bpp){}

    TGAColor(const TGAColor &c) : val(c.val), bytespp(c.bytespp){}

    TGAColor(const unsigned char *p, int bpp) : val(0), bytespp(bpp){
        for(int i = 0; i < bpp; i++){
            raw[i] = p[i];
        }
    }

    TGAColor& operator =(const TGAColor &c){
        if(this != &c){
            bytespp = c.bytespp;
            val = c.val;
        }
        return *this;
    }
};



#endif