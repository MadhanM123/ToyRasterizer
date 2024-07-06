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

class TGAImg{
    
    protected:
        unsigned char* data;
        int width;
        int height;
        int bytespp;

        bool load_rle_data(std::ifstream &in);
        bool unload_rle_data(std::ofstream &out);

    public:
        enum Format{
            GRAYSCALE = 1, RGB = 3, RGBA = 4
        };

        TGAImg();
        TGAImg(int w, int h, int bpp);
        TGAImg(const TGAImg &img);

        bool read_tga_file(const char *filename);
        bool write_tga_file(const char *filename, bool rle = true);
        bool flip_horiz();
        bool flip_vert();
        bool scale(int w, int h);
        TGAColor get(int x, int y);
        bool set(int x, int y, TGAColor c);
        ~TGAImg();
        TGAImg& operator = (const TGAImg &img);
        int get_width();
        int get_height();
        int get_bytespp();
        unsigned char* buffer;
        void clear();
};



#endif //IMG_H