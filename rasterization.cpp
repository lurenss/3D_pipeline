#include"rasterization.h"
using namespace pipeline3D;

#include<iostream>
#include<chrono>

    // Parent vertex class that contains all standard information
    class Vertex_standard { 
        public: 
            float x;
            float y;
            float z;
            float nx;
            float ny;
            float nz;
            float u;
            float v;

            Vertex_standard(float x, float y, float z, float nx, float ny, float nz, float u, float v) {
                this->x = x;
                this->y = y;
                this->z = z;
                this->nx = nx;
                this->ny = ny;
                this->nz = nz;
                this->u = u;
                this->v = v;
            }

			void perspective_correct(Vertex_standard& v) {
				v.z = 1.0f/v.z;
				v.x *= v.z;
				v.y *= v.z;
				v.nx *= v.z;
				v.ny *= v.z;
				v.nz *= v.z;
				v.u *= v.z;
				v.v *= v.z;
			}
    }; 

    // Child class of Vertex that also contains a boolean value
    class Vertex_bool : public Vertex_standard { 
        public: 
            bool b; 

            Vertex_bool(float x, float y, float z, float nx, float ny, float nz, float u, float v, bool b) : Vertex_standard(x, y, z, nx, ny, nz, u, v) {
                this->b = b;
            }
    }; 

    // Shades a pixel based on if its boolean value is true or false
    struct Shader_bool {
        template <class Vertex>
        char shade(Vertex v) {
            if (v.b == true) {
                return 't';
            } else {
                return 'f';
            }
        }
    };

    // Shades a pixel based on if its z-value
    struct Shader_depth {
        template <class Vertex>
        char shade(Vertex v) {
            return static_cast<char>((v.z-1)*10.0f+0.5f)%10+'0';
        }
    };

    int main() {
        const int w=150;
        const int h=50;

       // Create two kinds of shaders
        Shader_depth shader_depth;
        Shader_bool shader_bool;

        // Create two kinds of vertices
        constexpr float slope=-0.2f;
        Vertex_standard v1 = {1,-1,1.5f+slope*(1-1),0,0,0,0,0}, v2 = {1,1,1.5f+slope*(1+1),0,0,0,1,0}, v3 = {-1,1,1.5f-+slope*(-1+1),0,0,0,1,0};
        Vertex_bool v1_bool = {1,-1,1.5f+slope*(1-1),0,0,0,0,0,false}, v3_bool = {-1,1,1.5f-+slope*(-1+1),0,0,0,1,0,true}, v4_bool = {-1,-1,1.5f+slope*(-1-1),0,0,0,0,0,true};
        
        // Set up rasterizer with projection & screen information
        Rasterizer<char> rasterizer;
        rasterizer.set_perspective_projection(-1,1,-1,1,1,2);

        std::vector<char> screen(w*h,'.');
        rasterizer.set_target(w,h,&screen[0]);

        // Caluclate elapsed time
        auto start_time = std::chrono::high_resolution_clock::now();
        for (int i=0; i!=100000; ++i) {
            rasterizer.render_triangle(v1, v2, v3, shader_depth);
            rasterizer.render_triangle(v4_bool, v1_bool, v3_bool, shader_bool);
        }
        auto end_time = std::chrono::high_resolution_clock::now();
        double elapsed_time = std::chrono::duration<double>(end_time-start_time).count();

        std::cout << "elapsed time: " << elapsed_time << '\n';

        // Render our two object with their related shaders
        rasterizer.render_triangle(v1, v2, v3, shader_depth); // Standard vertex & depth shader
        rasterizer.render_triangle(v4_bool, v1_bool, v3_bool, shader_bool); // Vertex with boolean value & boolean shader

        // Print out the screen with a frame around it
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