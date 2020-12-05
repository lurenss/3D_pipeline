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

            Vertex_standard interpolate(const Vertex_standard& v1, const Vertex_standard& v2, float w) const {
				const float w2 = (1.0f-w);
				Vertex_standard v = v1;
				v.x = (w*v1.x + w2*v2.x);
				v.y = (w*v1.y + w2*v2.y);
				v.z = (w*v1.z + w2*v2.z);
				v.nx = (w*v1.nx + w2*v2.nx);
				v.ny = (w*v1.ny + w2*v2.ny);
				v.nz = (w*v1.nz + w2*v2.nz);
				v.u = (w*v1.u + w2*v2.u);
				v.v = (w*v1.v + w2*v2.v);

				return v;
			}
    }; 

    // Child class of Vertex that also contains a boolean value
    class Vertex_bool : public Vertex_standard { 
        public: 
            bool b; 

            Vertex_bool(float x, float y, float z, float nx, float ny, float nz, float u, float v, bool b) : Vertex_standard(x, y, z, nx, ny, nz, u, v) {
                this->b = b;
            }

            Vertex_bool interpolate(const Vertex_bool& v1, const Vertex_bool& v2, float w) const {
                const float w2 = (1.0f-w);
				Vertex_bool v = v1;
				v.x = (w*v1.x + w2*v2.x);
				v.y = (w*v1.y + w2*v2.y);
				v.z = (w*v1.z + w2*v2.z);
				v.nx = (w*v1.nx + w2*v2.nx);
				v.ny = (w*v1.ny + w2*v2.ny);
				v.nz = (w*v1.nz + w2*v2.nz);
				v.u = (w*v1.u + w2*v2.u);
				v.v = (w*v1.v + w2*v2.v);
		
				return v;
			}
    }; 

    // Shades a pixel based on the bolean value at the vertex
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

    // Shades a pixel based on the z-value at the vertex
    struct Shader_depth {
        template <class Vertex>
        char shade(Vertex v) {
            return static_cast<char>((v.z-1)*10.0f+0.5f)%10+'0';
        }
    };




    struct my_object : public Rasterizer<char>::scene_object{
        std::vector<Vertex_standard> v;
        Shader_depth shader;

        void render(Rasterizer<char>& r) override {
            r.render_triangle(v[0],v[1],v[2],shader);
        };
    };

    struct my_object1 : public Rasterizer<char>::scene_object{
        std::vector<Vertex_bool> v;
        Shader_bool shader;

        void render(Rasterizer<char>& r) override {
            r.render_triangle(v[0],v[1],v[2],shader);
        };
    };

    class Scene : public Rasterizer<char>::scene{};
    
    int main() {
        const int w=150;
        const int h=50;

        // Create two different types of vertices
        constexpr float slope=-0.2f;
        Vertex_standard v1 = {1,-1,1.5f+slope*(1-1),0,0,0,0,0}, v2 = {1,1,1.5f+slope*(1+1),0,0,0,1,0}, v3 = {-1,1,1.5f-+slope*(-1+1),0,0,0,1,0};
        Vertex_bool v1_bool = {1,-1,1.5f+slope*(1-1),0,0,0,0,0,false}, v3_bool = {-1,1,1.5f-+slope*(-1+1),0,0,0,1,0,true}, v4_bool = {-1,-1,1.5f+slope*(-1-1),0,0,0,0,0,true};
        
        
        // Insert vertices inside the two differe objects  
        my_object A;
        A.v.push_back(v1);
        A.v.push_back(v2);
        A.v.push_back(v3);

        my_object1 B;
        B.v.push_back(v1_bool);
        B.v.push_back(v3_bool);
        B.v.push_back(v4_bool);

        
        //Set up scene 
        Scene s;
        s.scene.push_back(&A);
        s.scene.push_back(&B);
        
        // Set up rasterizer with projection & screen information
        Rasterizer<char> rasterizer;
        rasterizer.set_perspective_projection(-1,1,-1,1,1,2);


        std::vector<char> screen(w*h,'.');
        rasterizer.set_target(w,h,&screen[0]);

        // Caluclate elapsed time
        auto start_time = std::chrono::high_resolution_clock::now();
        for (int i=0; i!=100000; ++i) {
            s.render(rasterizer);
        }
        auto end_time = std::chrono::high_resolution_clock::now();
        double elapsed_time = std::chrono::duration<double>(end_time-start_time).count();

        std::cout << "elapsed time: " << elapsed_time << '\n';


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