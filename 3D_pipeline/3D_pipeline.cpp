#include <iostream>

struct vector3d {
	float x, y, z;
};

struct triangle {
	vector3d p[3];
};

struct matrix4x4 {
	float m[4][4];
};

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

        // TODO: Currently only works with one input triangle, can expand to multiple triangles later if needed
        void drawTriangles(triangle tri) {

            // DEBUG - Can remove later
            int range[3] = {0,1,2};
            std::cout << "ORIGINAL COORDINATES\n";
            for ( auto i : range ) {
                std::cout << tri.p[i].x;
                std::cout << "\n";
                std::cout << tri.p[i].y;
                std::cout << "\n";
                std::cout << tri.p[i].z;
                std::cout << "\n\n";
            }

            // Projection matrix specifications
            float near = 1.0f; // distance from viewer to nearest point in screen
            float far = 2.0f; // distance from viewer to furthest point in screen
            float fov = 90.0f; // field of view = 90 degrees
            float fovRadian = 1.0f / tanf(fov * 0.5f / 180.0f * 3.14159f);
            float aspectRatio = (float) height / (float) width;
            float zScaleFactor = far / (far - near);
            float zScaleFactorOffset = (-far * near) / (far - near); // offset to account for the space between the viewer and the screen

            matrix4x4 projectionMatrix = { 0 };
            projectionMatrix.m[0][0] = aspectRatio * fovRadian;
            projectionMatrix.m[1][1] = fovRadian;
            projectionMatrix.m[2][2] = zScaleFactor;
            projectionMatrix.m[3][2] = zScaleFactorOffset;
            projectionMatrix.m[2][3] = 1.0f;
            projectionMatrix.m[3][3] = 0.0f;

            // Projection - For each vertex in the vector, change its coordinates from 3D -> 2D
            triangle triProjected;
            multiplyVectorMatrix(tri.p[0], triProjected.p[0], projectionMatrix);
            multiplyVectorMatrix(tri.p[1], triProjected.p[1], projectionMatrix);
            multiplyVectorMatrix(tri.p[2], triProjected.p[2], projectionMatrix);

            // DEBUG - Can remove later - We should only need to use the x and y cordinates after projection
            std::cout << "AFTER PROJECTION\n";
            for ( auto i : range ) {
                std::cout << triProjected.p[i].x;
                std::cout << "\n";
                std::cout << triProjected.p[i].y;
                std::cout << "\n\n";
            }

            // TODO: Still need to rasterize, clip, scale, plot/visualize onto output buffer display
        }

        // TODO: Can we use some sort of library for this instead of a manual calculation?
        /**
         * @brief Multiplies a 3D vector of (x,y,z) cordinates (with an assumed w=1 fourth coordinate) by a matrix
         * @param &input pointer to the initial vector3d (x,y,z) coordinates
         * @param &output pointer to the output vector3d object which contains projected coordinates
         * @return void
        **/
        void multiplyVectorMatrix(vector3d &input, vector3d &output, matrix4x4 &matrix) {
            output.x = (input.x * matrix.m[0][0]) + (input.y * matrix.m[1][0]) + (input.z * matrix.m[2][0]) + matrix.m[3][0];
            output.y = (input.x * matrix.m[0][1]) + (input.y * matrix.m[1][1]) + (input.z * matrix.m[2][1]) + matrix.m[3][1];
            output.z = (input.x * matrix.m[0][2]) + (input.y * matrix.m[1][2]) + (input.z * matrix.m[2][2]) + matrix.m[3][2];

            float w = input.x * matrix.m[0][3] + input.y * matrix.m[1][3] + (input.z * matrix.m[2][3]) + matrix.m[3][3];

            if (w != 0.0f) {
                output.x /= w;
                output.y /= w;
                output.z /= w;
            }
        }
    private:
        int width;
        int height;
        char* buffer;
};

int main(){
    VideoBuffer v = VideoBuffer(150, 50);

    // TODO: Remove hardcoded vertices/triangles and accept them as input parameters?
    vector3d v1 = { 1.0f, -1.0f, 1.5f };
    vector3d v2 = { 1.0f, 1.0f, 1.1f };
    vector3d v3 = { -1.0f, 1.0f, 1.5f };

    triangle tri1 = { v1, v2, v3 };

    v.drawTriangles(tri1);
    v.printBuffer();
    return 0;
}