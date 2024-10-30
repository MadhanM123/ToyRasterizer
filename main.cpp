#include "tgaimg.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

int main(int argc, char** argv){
    TGAImg img(100, 100, TGAImg::RGB);
    img.set(52, 41, white);

    img.flip_vert();
    img.write_tga_file("out.tga");
    return 0;
}