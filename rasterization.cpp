#include"rasterization.h"
using namespace pipeline3D;

#include<iostream>
#include<chrono>

    class ParentVertex { 
        public: 
            float x;
            float y;
            float z;
            float nx;
            float ny;
            float nz;
            float u;
            float v;

            ParentVertex() {};
            ParentVertex(float x, float y, float z, float nx, float ny, float nz, float u, float v) {
                this->x = x;
                this->y = y;
                this->z = z;
                this->nx = nx;
                this->ny = ny;
                this->nz = nz;
                this->u = u;
                this->v = v;
            }
    }; 

    class ChildVertex : public ParentVertex { 
        public: 
            bool b; 

            ChildVertex() {
                ParentVertex();
                //this->b = false;
            };
            ChildVertex(float x, float y, float z, float nx, float ny, float nz, float u, float v, bool b) : ParentVertex(x, y, z, nx, ny, nz, u, v) {
                this->b = b;
            }
    }; 

    struct Shader_a {
        template <class Vertex>
        char shade(Vertex v) {
            if (v.b == true) {
                return 't';
            } else {
                return 'f';
            }
        }
    };

    struct Shader_num {
        template <class Vertex>
        char shade(Vertex v) {
            return static_cast<char>((v.z-1)*10.0f+0.5f)%10+'0';
        }
    };

    int main() {
        const int w=150;
        const int h=50;

        Shader_num shader_num;
        Shader_a shader_a;

        constexpr float slope=-0.2f;

        ParentVertex v1={1,-1,1.5f+slope*(1-1),0,0,0,0,0}, v2={1,1,1.5f+slope*(1+1),0,0,0,1,0}, v3={-1,1,1.5f-+slope*(-1+1),0,0,0,1,0}, v4={-1,-1,1.5f+slope*(-1-1),0,0,0,0,0};
        ChildVertex v1b={1,-1,1.5f+slope*(1-1),0,0,0,0,0,false}, v2b={1,1,1.5f+slope*(1+1),0,0,0,1,0,false}, v3b={-1,1,1.5f-+slope*(-1+1),0,0,0,1,0,false}, v4b={-1,-1,1.5f+slope*(-1-1),0,0,0,0,0,true};

        Rasterizer<char> rasterizer;
        rasterizer.set_perspective_projection(-1,1,-1,1,1,2);

        std::vector<char> screen(w*h,'.');
        rasterizer.set_target(w,h,&screen[0]);

        auto start_time = std::chrono::high_resolution_clock::now();
        for (int i=0; i!=100000; ++i) {
            rasterizer.render_triangle(v1,v2,v3, shader_num);
            rasterizer.render_triangle(v4b,v1b,v3b, shader_a);
        }
        auto end_time = std::chrono::high_resolution_clock::now();
        double elapsed_time = std::chrono::duration<double>(end_time-start_time).count();

        std::cout << "elapsed time: " << elapsed_time << '\n';

        rasterizer.render_triangle(v1,v2,v3, shader_num); // parent vertex
        rasterizer.render_triangle(v4b,v1b,v3b, shader_a); // child vertex

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