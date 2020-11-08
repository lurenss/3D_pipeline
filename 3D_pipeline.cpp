#include <iostream>
#include <limits>
#include <stdio.h>
#include <string.h>
#include <cmath>

struct vector3d {
	float x, y, z;
};

typedef struct {
  float x, y, z;
} Point3D;

struct triangle {
	 Point3D p[3];
};

struct matrix4x4 {
	float m[4][4];
};

#define SCREEN_HEIGHT 50
#define SCREEN_WIDTH  150

// min X and max X for every horizontal line within the triangle
long ContourX[SCREEN_HEIGHT][2];

#define ABS(x) ((x >= 0) ? x : -x)

class Screen {
    public:
        Point3D b0, b1, b2;

        Screen(int width_, int height_) {
            width = width_;
            height = height_;

            buffer = new char [width * height]; 
            zBuffer = new char [width * height]; 
            cleanBuffer();
            zcleanBuffer();
        }
        void SetPixel(long x, long y, char c) {
            //if ((x < 0) || (x >= width) || (y < 0) || (y >= height)) {
            if ( x < 0 || x >= width * height || y < 0 || y >= width * height) {
                return;
            }
            char d = zBuffer[GetIndex(y,x)];

            if (d == '.') {
                d = c;
            }

            buffer[GetIndex(y,x)] = d;
    
        }

        void cleanBuffer() {
            for (int i = 0; i < (width * height); ++i)
                buffer[i] = '.';
        }

        void zcleanBuffer() {
            for (int i = 0; i < (width * height); ++i)
                zBuffer[i] = '.';
        }

        void Visualize(void) {
            long x, y;

            for (y = 0; y < SCREEN_HEIGHT; y++) {
                for (x = 0; x < SCREEN_WIDTH; x++) {
                    printf("%c", buffer[GetIndex(y,x)]);
                }

                printf("\n");
            }
        }

        // Scans a side of a triangle setting min X and max X in ContourX[][]
        // (using the Bresenham's line drawing algorithm).
        void ScanLine(long x1, long y1, long x2, long y2) {
            long sx, sy, dx1, dy1, dx2, dy2, x, y, m, n, k, cnt;

            sx = x2 - x1;
            sy = y2 - y1;

            if (sx > 0) dx1 = 1;
            else if (sx < 0) dx1 = -1;
            else dx1 = 0;

            if (sy > 0) dy1 = 1;
            else if (sy < 0) dy1 = -1;
            else dy1 = 0;

            m = ABS(sx);
            n = ABS(sy);
            dx2 = dx1;
            dy2 = 0;

            if (m < n)
            {
                m = ABS(sy);
                n = ABS(sx);
                dx2 = 0;
                dy2 = dy1;
            }

            x = x1; y = y1;
            cnt = m + 1;
            k = n / 2;

            while (cnt--)
            {
                if ((y >= 0) && (y < SCREEN_HEIGHT))
                {
                if (x < ContourX[y][0]) ContourX[y][0] = x;
                if (x > ContourX[y][1]) ContourX[y][1] = x;
                }

                k += n;
                if (k < m)
                {
                x += dx2;
                y += dy2;
                }
                else
                {
                k -= m;
                x += dx1;
                y += dy1;
                }
            }
        }
        triangle projectCoordinates(triangle tri) {

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
            float aspectRatio = (float) SCREEN_HEIGHT / (float) SCREEN_WIDTH;
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
                std::cout << "\n";
                std::cout << triProjected.p[i].z;
                std::cout << "\n\n";
            }

            // Convert from 2D x,y coordinates to integer values within our display screen buffer
            triangle tri_norm;
            tri_norm.p[0] = normalizeCoordinates(triProjected.p[0]);
            tri_norm.p[1] = normalizeCoordinates(triProjected.p[1]);
            tri_norm.p[2] = normalizeCoordinates(triProjected.p[2]);
          
            std::cout << "AFTER PROJECTION TO BUFFER PIXELS\n";
            std::cout << tri_norm.p[0].x;
            std::cout << "\n";
            std::cout << tri_norm.p[0].y;
            std::cout << "\n\n";
            std::cout << tri_norm.p[1].x;
            std::cout << "\n";
            std::cout << tri_norm.p[1].y;
            std::cout << "\n\n";
            std::cout << tri_norm.p[2].x;
            std::cout << "\n";
            std::cout << tri_norm.p[2].y;
            std::cout << "\n\n";

            return tri_norm;
		
        }

		
		Point3D normalizeCoordinates(Point3D v) {
            Point3D p;
            p.x = round(((v.x+1) * (width)) / 2);
            p.y = round(((v.y+1) * (height)) / 2);

            // normalize z and set it in z buffer of same size as display buffer
            // z_projected is in [1,2] -> we shift it in [0,1] -> multiply it by 10 -> round it to nearest integer
            int z = round(v.z * 10);
            


            if (((zBuffer[GetIndex(p.y,p.x)]) == '.') || (zBuffer[GetIndex(p.y,p.x)] > z)) {
                zBuffer[GetIndex(p.y,p.x)] = (char) (z+'0');
            }
    
            p.z = z;

            return p;
		}

        /*int normalizeZ(float z) {

            //do soemting to make the float 0-1
            return ;
        }*/

		// TODO: Can we use some sort of library for this instead of a manual calculation?
        /**
        * @brief Multiplies a 3D vector of (x,y,z) cordinates (with an assumed w=1 fourth coordinate) by a matrix
        * @param &input pointer to the initial vector3d (x,y,z) coordinates
        * @param &output pointer to the output vector3d object which contains projected coordinates
        * @return void
        */
        void multiplyVectorMatrix(Point3D &input, Point3D &output, matrix4x4 &matrix) {
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

        void DrawTriangle(Point3D p0, Point3D p1, Point3D p2) {
            int y;

            for (y = 0; y < SCREEN_HEIGHT; y++)
            {
                ContourX[y][0] = std::numeric_limits<long>::max(); // min X
                ContourX[y][1] = std::numeric_limits<long>::min(); // max X
            }

            ScanLine(p0.x, p0.y, p1.x, p1.y);
            ScanLine(p1.x, p1.y, p2.x, p2.y);
            ScanLine(p2.x, p2.y, p0.x, p0.y);

            for (y = 0; y < SCREEN_HEIGHT; y++)
            {
                if (ContourX[y][1] >= ContourX[y][0]) {
                    long x = ContourX[y][0];
                    long len = 1 + ContourX[y][1] - ContourX[y][0];

                    // Can draw a horizontal line instead of individual pixels here
                    while (len--) {
                        char z = (char) (round(calcZ(p0, p1, p2, x, y)) + '0');
                        SetPixel(x++, y, z);
                    }
                }
            }
        };

        private:
            int width;
            int height;  
            char* buffer;  
            char* zBuffer;  
            int rowCount;
            int colCount;

            int GetIndex(long y, long x) {
                return (width * (y-1)) + x - 1;
            }

            float calcZ(Point3D p1, Point3D p2, Point3D p3, int x, int y) { 
                float a1 = p2.x - p1.x; 
                float b1 = p2.y - p1.y; 
                float c1 = p2.z - p1.z; 
                float a2 = p3.x - p1.x; 
                float b2 = p3.y - p1.y; 
                float c2 = p3.z - p1.z; 
                float a = b1 * c2 - b2 * c1; 
                float b = a2 * c1 - a1 * c2; 
                float c = a1 * b2 - b1 * a2; 
                float d = (- a * p1.x - b * p1.y - c * p1.z); 

                return (-a*x-b*y-d)/c;
            }
            

};

/*
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
                std::cout << triProjected.p[i].z;
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
*/

/*checks if vertex coordinates are in screen range
(not sure if the range I assumed is correct (x=[-1,1],y=[-1,1],z=[1,2])*/
bool checkValues(vector3d v) {
    //check if the inserted vertex is on screen
    if (v.x < -1 || v.x>1 || v.y < -1 || v.y>1 || v.z < 1 || v.z>2) {
        std::cout << "Out of screen vertex. Please insert valid coordinates \nx=[-1,1]\ny=[-1,1]\nz=[1,2]";
        return false;
    }
    else
        return true;
}

/*get vertices from user and call DrawTriangles to perform projection
* Issues:
* 1) if the user behaves "well" it seems to work...but if he inserts a char as any of the vertex coordinates everything
*    breaks
* 2) if we need to keep in memory the values of triangles, I think we need to modify something
*    because in the loop I reuse the same triangle instance everytime
*
* Let me know if you think this is a bad design. If you need clarifications text me anytime
*/




/*int main() {
    VideoBuffer v = VideoBuffer(SCREEN_WIDTH, SCREEN_HEIGHT);

    //* Haley hardcoded values
    TODO: Remove hardcoded vertices/triangles and accept them as input parameters?
    vector3d v1 = { 1.0f, -1.0f, 1.5f };
    vector3d v2 = { 1.0f, 1.0f, 1.1f };
    vector3d v3 = { -1.0f, 1.0f, 1.5f };
    triangle tri1 = { v1, v2, v3 };
    v.drawTriangles(tri1);

    //Francesco function to get user vertices
    //UserInputTriangles(v);
    v.printBuffer();
    return 0;
}*/

int main() {
    Point3D p0, p1, p2, p3;

    Screen s = Screen(SCREEN_WIDTH,SCREEN_HEIGHT);
 
    // generate random triangle coordinates
    srand((unsigned)time(NULL));

    p0.x = 1.0;
    p0.y = -1.0;
    p0.z = 1.5;

    p1.x = 1.0;
    p1.y = 1.0;
    p1.z = 1.1;

    p2.x = -1.0;
    p2.y = 1.0;
    p2.z = 1.5;

    p3.x = -1.0;
    p3.y = -1.0;
    p3.z = 1.9;

    triangle tri1 = { p0, p1, p2 };
    triangle tri2 = { p0, p2, p3 };

    triangle t1 = s.projectCoordinates(tri1);
    triangle t2 = s.projectCoordinates(tri2);

    //s.DrawTriangle(s.b0, s.b1, s.b2);
    s.DrawTriangle(t1.p[0], t1.p[1], t1.p[2]);
    s.DrawTriangle(t2.p[0], t2.p[1], t2.p[2]);

    // also draw the triangle's vertices
    s.SetPixel(s.b0.x, s.b0.y, s.b0.z);
    s.SetPixel(s.b1.x, s.b1.y, s.b1.z);
    s.SetPixel(s.b2.x, s.b2.y, s.b2.z);

    s.SetPixel(t1.p[0].x, t1.p[0].y, t1.p[0].z);
    s.SetPixel(t1.p[1].x, t1.p[1].y, t1.p[1].z);
    s.SetPixel(t1.p[2].x, t1.p[2].y, t1.p[2].z);

    s.SetPixel(t2.p[0].x, t2.p[0].y, t2.p[0].z);
    s.SetPixel(t2.p[1].x, t2.p[1].y, t2.p[1].z);
    s.SetPixel(t2.p[2].x, t2.p[2].y, t2.p[2].z);

    s.Visualize();

    return 0;
}
