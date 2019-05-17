#include <iostream>
#include <fstream>
#include <math.h>
#include <string>

using namespace std;

typedef struct {
    int8_t id[2];
    int32_t filesize;
    int16_t reserved[2];
    int32_t headersize;
    int32_t infoSize;
    int32_t width;
    int32_t depth;
    int16_t biPlanes;
    int16_t bits;
    int32_t biCompression;
    int32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    int32_t biClrUsed;
    int32_t biClrImportant;
} BMPHEAD;

typedef struct {
public:
    int8_t redComponent;
    int8_t greenComponent;
    int8_t blueComponent;
} RGBQUAD;

class picture {
    
    BMPHEAD header;
    RGBQUAD  **array;
    
public:
    picture() {};
    void readPicture(const char *FileName);
    void writePicture(const char *SecondFile, float factor);
};

float floatCoordinateX(int x, int imageSize) {
    return (float(x)/float(imageSize));
}

float floatCoordinateY(int y, int imageSize) {
    return (float(y)/float(imageSize));
}

RGBQUAD getColor(float k1,float k2, RGBQUAD c1, RGBQUAD c2, RGBQUAD c3, RGBQUAD c4) {
    RGBQUAD tmp, r1, r2;
    
    r1.blueComponent = int((1-k1)*int(c1.blueComponent)) + int(k1*int(c2.blueComponent));
    r1.redComponent = int(((1-k1)*int(c1.redComponent))) + int(k1*int(c2.redComponent));
    r1.greenComponent = int(((1-k1)*int(c1.greenComponent))) + int(k1*int(c2.greenComponent));
    
    r2.blueComponent = int((1-k1)*int(c3.blueComponent)) + int(k1*int(c4.blueComponent));
    r2.redComponent = int((1-k1)*int(c3.redComponent)) + int(k1*int(c4.redComponent));
    r2.greenComponent = int((1-k1)*int(c3.greenComponent)) + int(k1*int(c4.greenComponent));
    
    tmp.blueComponent = int((1-k2)*int(r1.blueComponent)) + int(k2*int(r2.blueComponent));
    tmp.redComponent = int((1-k2)*int(r1.redComponent)) + int(k2*int(r2.redComponent));
    tmp.greenComponent = int((1-k2)*int(r1.greenComponent)) + int(k2*int(r2.greenComponent));
    
    return tmp;
}


void picture::readPicture(const char *FileName) {
    FILE *fp = fopen(FileName, "rb");
    if (!fp) {
        cout << "File could not be open\n";
        exit(1);
        return;
    }
    
    fread(&header.id, sizeof(header.id), 1, fp);
    fread(&header.filesize, sizeof(header.filesize), 1, fp);
    fread(&header.reserved, sizeof(header.reserved), 1, fp);
    fread(&header.headersize, sizeof(header.headersize), 1, fp);
    fread(&header.infoSize, sizeof(header.infoSize), 1, fp);
    fread(&header.width, sizeof(header.width), 1, fp);
    fread(&header.depth, sizeof(header.depth), 1, fp);
    fread(&header.biPlanes, sizeof(header.biPlanes), 1, fp);
    fread(&header.bits, sizeof(header.bits), 1, fp);
    fread(&header.biCompression, sizeof(header.biCompression), 1, fp);
    fread(&header.biSizeImage, sizeof(header.biSizeImage), 1, fp);
    fread(&header.biXPelsPerMeter, sizeof(header.biXPelsPerMeter), 1, fp);
    fread(&header.biYPelsPerMeter, sizeof(header.biYPelsPerMeter), 1, fp);
    fread(&header.biClrUsed, sizeof(header.biClrUsed), 1, fp);
    fread(&header.biClrImportant, sizeof(header.biClrImportant), 1, fp);
    
    int check = (header.width * 3) % 4;
    int stride = 0;
    if (check) {
        stride = 4 - check;
    }
    
    array = new RGBQUAD*[header.depth];
    for (int i = 0; i < header.depth; i++) {
        array[i] = new RGBQUAD[header.width];
    }
    
    for (int i = 0; i < header.depth; i++) {
        for (int j = 0; j < header.width; j++) {
            fread(&array[i][j].blueComponent, 1, 1, fp);
            fread(&array[i][j].greenComponent, 1, 1, fp);
            fread(&array[i][j].redComponent, 1, 1, fp);
        }
        if (stride != 0) {
            RGBQUAD empty;
            fread(&empty, 1, stride, fp);
        }
    }
    
    fclose(fp);
}

void picture::writePicture(const char *SecondFile, float factor)
{
    
    FILE *fp2 = fopen(SecondFile, "wb");
    
    int32_t headerDepthOld = header.depth; // размеры исходника
    int32_t headerWidthOld = header.width;
    
    header.depth = int(header.depth * factor);
    header.width = int(header.width * factor); // NEW EDIT
    
    int check = (header.width * 3) % 4;
    int stride = 0;
    if (check) {
        stride = 4 - check;
    }
    
    header.biSizeImage = header.depth*header.width * 3 + stride * header.depth;
    header.filesize = header.biSizeImage + sizeof(BMPHEAD);
    
    
    RGBQUAD **ARRAY = new RGBQUAD*[header.depth];
    for (int i = 0; i < header.depth; i++) {
        ARRAY[i] = new RGBQUAD[header.width];
    }
    
    
//    for (int i=0; i< header.depth; i++) // по Oy
//    {
//        for (int j=0; j< header.width; j++) // по Ox
//        {
//            ARRAY[i][j].blueComponent = array[1][2].blueComponent;
//            ARRAY[i][j].greenComponent = array[1][2].greenComponent;
//            ARRAY[i][j].redComponent = array[1][2].redComponent;
//
//        }
//    }
    
    
    for (int i=0; i< (header.depth-5); i++) // по Oy
    {
        for (int j=0; j< (header.width-5); j++) // по Ox
        {

            float floatX = floatCoordinateX(j, header.width); // float координаты в старой картинке
            float floatY = floatCoordinateY(i, header.depth);

            float k1,k2; // коэфиценты 01 для интерполяции - дробная часть 15.2 = 0.2

            float a;
            k1 = modf(headerWidthOld * floatX , &a);
            k2 = modf(headerDepthOld * floatY , &a);

            int c1 = int(headerWidthOld * floatX); // ? УГЛЫ НИЖНИЕ координаты какие
            int c2 = int(headerWidthOld * floatX)+1; // (headerWidthOld * floatX)-1 > header.width

            int c3 = int(headerDepthOld * floatY); //
            int c4 = int(headerDepthOld * floatY)+1;


            ARRAY[i][j] = getColor(k1, k2, array[c3][c1], array[c3][c2], array[c4][c1], array[c4][c2]);

            
                cout << "j: " << j <<  " header.width: " << header.width << " floatX: " << floatX << endl;
            cout << "i: " << i <<  " header.depth: " << header.depth << " floatY: " << floatY << endl;
        
            cout << "k1: " << k1 <<  " k2: " << k2 << endl;
                cout << "c1: " << c1 <<  " c2: " << c2 << " c3: " << c3 <<  " c4: " << c4 << endl<<endl;


        }
    }
    
//
//        for (int i = 0, q = 0; i < header.depth, q < header.depth / factor; i += factor, q++) {
//            for (int j = 0, w = 0; j < header.width, w < header.width / factor; j += factor, w++) {
//                for (int k = 0; k < factor; k++) {
//                    for (int f = 0; f < factor; f++) {
//
//                        ARRAY[i + k][j + f] = array[q][w];
//                        //ARRAY[i + k][j + f] = getColor(0.30, 0.60, array[3][3], array[3][4], array[4][3], array[4][4]);
//                        }
//                    }
//                }
//            }
    
    
    fwrite(&header.id, sizeof(header.id), 1, fp2);
    fwrite(&header.filesize, sizeof(header.filesize), 1, fp2);
    fwrite(&header.reserved, sizeof(header.reserved), 1, fp2);
    fwrite(&header.headersize, sizeof(header.headersize), 1, fp2);
    fwrite(&header.infoSize, sizeof(header.infoSize), 1, fp2);
    fwrite(&header.width, sizeof(header.width), 1, fp2);
    fwrite(&header.depth, sizeof(header.depth), 1, fp2);
    fwrite(&header.biPlanes, sizeof(header.biPlanes), 1, fp2);
    fwrite(&header.bits, sizeof(header.bits), 1, fp2);
    fwrite(&header.biCompression, sizeof(header.biCompression), 1, fp2);
    fwrite(&header.biSizeImage, sizeof(header.biSizeImage), 1, fp2);
    fwrite(&header.biXPelsPerMeter, sizeof(header.biXPelsPerMeter), 1, fp2);
    fwrite(&header.biYPelsPerMeter, sizeof(header.biYPelsPerMeter), 1, fp2);
    fwrite(&header.biClrUsed, sizeof(header.biClrUsed), 1, fp2);
    fwrite(&header.biClrImportant, sizeof(header.biClrImportant), 1, fp2);
    
    for (int i = 0; i < header.depth; i++) {
        for (int j = 0; j < header.width; j++) {
            fwrite(&ARRAY[i][j].blueComponent, 1, 1, fp2);
            fwrite(&ARRAY[i][j].greenComponent, 1, 1, fp2);
            fwrite(&ARRAY[i][j].redComponent, 1, 1, fp2);
            
            cout << "new pixel: [ " << i << " " << j << " ] " << int(ARRAY[i][j].redComponent) << " " << int(ARRAY[i][j].greenComponent) << " "<<  int(ARRAY[i][j].blueComponent) << endl;
            
        }
        int8_t empty = 0x00;
        if (stride != 0) {
            fwrite(&empty, 1, stride, fp2);
        }
    }
    
    fclose(fp2);
}

int main(int argc, const char * argv[]) {
    
    picture try_it;
    const char *file = argv[1];
    try_it.readPicture(file);
    const char *result = argv[2];
    string buff = argv[3];
    float num = stof(buff);
    
    //int num = stoi(buff);
    
    
    try_it.writePicture(result, num);
    cout << "Program complited successfully. Try to check file \"output.bmp\"" << endl << endl;

    return 0;
}
