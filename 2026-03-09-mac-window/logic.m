#import <Foundation/Foundation.h>


// This is the function we will swap out
void update_buffer(unsigned char* data, int width, int height, int frame) {
    for (int i = 0; i < width * height * 4; i += 4) {
        data[i+0] = (unsigned char)(i + frame); // Red
        data[i+1] = 100;                        // Green
        data[i+2] = 200;                        // Blue
        data[i+3] = 255;                        // Alpha
    }
}
