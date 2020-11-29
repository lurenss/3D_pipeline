#include"rasterization.h"
using namespace pipeline3D;

#include<iostream>
#include<chrono>

    struct my_vertex {
        float x;
        float y;
        float z;
        float nx;
        float ny;
        float nz;
        float u;
        float v;
    };

    struct my_shader {
         char shade(my_vertex v) {
            return static_cast<char>((v.z-1)*10.0f+0.5f)%10+'0';
        }
    };

    int main() {
        const int w=150;
        const int h=50;

        my_shader shader;
        my_vertex vertex;

        constexpr float slope=-0.2f;
        my_vertex v1={1,-1,1.5f+slope*(1-1),0,0,0,0,0}, v2={1,1,1.5f+slope*(1+1),0,0,0,1,0}, v3={-1,1,1.5f-+slope*(-1+1),0,0,0,1,0}, v4={-1,-1,1.5f+slope*(-1-1),0,0,0,0,0};

        Rasterizer<char, my_shader, my_vertex> rasterizer;
        rasterizer.set_perspective_projection(-1,1,-1,1,1,2);

        std::vector<char> screen(w*h,'.');
        rasterizer.set_target(w,h,&screen[0]);

        auto start_time = std::chrono::high_resolution_clock::now();
        for (int i=0; i!=100000; ++i) {
            rasterizer.render_triangle(v1,v2,v3);
            rasterizer.render_triangle(v4,v1,v3);
        }
        auto end_time = std::chrono::high_resolution_clock::now();
        double elapsed_time = std::chrono::duration<double>(end_time-start_time).count();

        std::cout << "elapsed time: " << elapsed_time << '\n';

        rasterizer.render_triangle(v1,v2,v3);
        rasterizer.render_triangle(v4,v1,v3);

        // print out the screen with a frame around it
        std::cout << '+';
        for (int j=0; j!=w; ++j) std::cout << '-';
        std::cout << "+\n";

        for (int i=0;i!=h;++i) {
            std::cout << '|';
            for (int j=0; j!=w; ++j) std::cout << screen[i*w+j];
            std::cout << "|\n";
        }

        std::cout << '+';
        for (int j=0; j!=w; ++j) std::cout << '-';
        std::cout << "+\n";

        return 0;
    }