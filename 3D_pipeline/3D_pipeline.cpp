#include <iostream>


class VideoBuffer {
    public:
        VideoBuffer(int width_, int height_) {
            width = width_;
            height = height_;
            buffer = new char[width * height];

            cleanBuffer(); // initialization buffer 
        }

        void cleanBuffer() {
            for (int i = 0; i < (width * height); ++i)
                buffer[i] = '.';
        }

        void printBuffer() {
            for (int i = 0; i < width*height; i++) {
                if ((i+1) % width == 0) {
                    std::cout << buffer[i];
                    std::cout << "\n";

                }
                else{
                    std::cout << buffer[i];
                }
            }
        }
    private:
        int width;
        int height;
        char* buffer;
};

int main(){
    VideoBuffer v = VideoBuffer(150, 50);
    v.printBuffer();
    return 0;
}

