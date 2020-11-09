#include <iostream>
#include <limits>
#include <stdio.h>

// A point with x,y,z coordinates
struct Point3D {
    float x, y, z;
};

// A triangle composed of 3 points
struct triangle {
    Point3D points[3];
};

// A 4x4 matrix used for projection calculations
struct matrix4x4 {
	float m[4][4];
};

// Screen width and height measurements
#define SCREEN_HEIGHT 50
#define SCREEN_WIDTH 150

// Absolute value definition
#define ABS(x) ((x >= 0) ? x : -x)

// Holds the minimum and maximum X value for every horizontal line within the triangle
long ContourX[SCREEN_HEIGHT][2];

class Screen {
    public:
        Screen(int screenWidth, int screenHeight) {
            width = screenWidth;
            height = screenHeight;
            
            // Initialize the screen's display buffer
            buffer = new char [width * height]; 
            InitializeBuffer(buffer, '.');

            // Initialize the screen's z-value buffer
            zBuffer = new char [width * height];
            InitializeBuffer(zBuffer, '.');
            // TODO: maybe make this a 0 and then compare differently the zbuffer values?
        }

        /**
        * Visualizes the screen by printing the value of the display buffer at each pixel
        */
        void Visualize() {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    printf("%c", buffer[GetIndex(y,x)]);
                }
                printf("\n");
            }
        }

        // TODO
        triangle project3Dto2D(triangle tri) {

            // DEBUG - Can remove later
            int range[3] = {0,1,2};
            std::cout << "ORIGINAL COORDINATES\n";
            for ( auto i : range ) {
                std::cout << tri.points[i].x;
                std::cout << "\n";
                std::cout << tri.points[i].y;
                std::cout << "\n";
                std::cout << tri.points[i].z;
                std::cout << "\n\n";
            }

            // Projection matrix specifications
            float fov = 90.0f; // field of view = 90 degrees
            float fovRadian = 1.0f / tanf(fov * 0.5f / 180.0f * 3.14159f); // field of view in radians
            float aspectRatio = (float) height / width;
            float zScaleFactor = far / (far - near);
            float zScaleFactorOffset = (-far * near) / (far - near); // offset to account for the space between the viewer and the screen

            matrix4x4 projectionMatrix = { 0 };

            // original
            /*projectionMatrix.m[0][0] = aspectRatio * fovRadian;
            projectionMatrix.m[1][1] = fovRadian;
            projectionMatrix.m[2][2] = zScaleFactor;
            projectionMatrix.m[3][2] = zScaleFactorOffset;
            projectionMatrix.m[2][3] = 1.0f;
            projectionMatrix.m[3][3] = 0.0f;*/

            // col major

            matrix4x4 otherMatrix = { 0 };
            otherMatrix.m[0][0] = (2 * near)/ (right - left);
            otherMatrix.m[1][1] = (2 * near)/ (bottom - top);
            otherMatrix.m[2][2] = (far + near) / (far - near);
            otherMatrix.m[2][1] = -(bottom + top)/(bottom - top);
            otherMatrix.m[3][2] = (-2 * near * far) / (far - near);
            otherMatrix.m[2][0] = -(right + left)/(right - left);
            otherMatrix.m[2][3] = 1.0;

            // Projection - For each vertex in the triangle, change its coordinates from 3D -> 2D
            /*triangle triProjected;
            multiplyVectorMatrix(tri.points[0], triProjected.points[0], projectionMatrix);
            multiplyVectorMatrix(tri.points[1], triProjected.points[1], projectionMatrix);
            multiplyVectorMatrix(tri.points[2], triProjected.points[2], projectionMatrix);*/

            triangle triProjected;
            multiplyVectorMatrix(tri.points[0], triProjected.points[0], otherMatrix);
            multiplyVectorMatrix(tri.points[1], triProjected.points[1], otherMatrix);
            multiplyVectorMatrix(tri.points[2], triProjected.points[2], otherMatrix);

            // DEBUG - Can remove later - We should only need to use the x and y cordinates after projection
            std::cout << "AFTER PROJECTION\n";
            for ( auto i : range ) {
                std::cout << triProjected.points[i].x;
                std::cout << "\n";
                std::cout << triProjected.points[i].y;
                std::cout << "\n";
                std::cout << triProjected.points[i].z;
                std::cout << "\n\n";
            }

            // Convert from 2D x,y coordinates to integer values within our display screen buffer
            triangle tri_norm;
            tri_norm.points[0] = normalizeCoordinates(triProjected.points[0]);
            tri_norm.points[1] = normalizeCoordinates(triProjected.points[1]);
            tri_norm.points[2] = normalizeCoordinates(triProjected.points[2]);

            std::cout << "AFTER NORMALIZING\n";
            for ( auto i : range ) {
                std::cout << tri_norm.points[i].x;
                std::cout << "\n";
                std::cout << tri_norm.points[i].y;
                std::cout << "\n";
                std::cout << tri_norm.points[i].z;
                std::cout << "\n\n";
            }

            return tri_norm;
        }

		// TODO
		Point3D normalizeCoordinates(Point3D v) {

            // go from float values (-1, 1) to integer values (0, widht*height)
            Point3D p;
            p.x = round(((v.x+1) * (width)) / 2); // go from float values (-1, 1) to integer values (0, width)
            p.y = round(((v.y+1) * (height)) / 2); // go from float values (-1, 1) to integer values (0, height)

            // normalize z and set it in z buffer of same size as display buffer
            // z_projected is in [1,2] -> we shift it in [0,1] -> multiply it by 10 -> round it to nearest integer
            int z = round(v.z * 10);

            if (((zBuffer[GetIndex(p.y,p.x)]) == '.') || (zBuffer[GetIndex(p.y,p.x)] > z)) {
                zBuffer[GetIndex(p.y,p.x)] = (char) (z+'0');
            }
    
            p.z = z;

            return p;
		}

        // TODO
        void DrawTriangle(Point3D p0, Point3D p1, Point3D p2) {
            int y;

            for (y = 0; y < height; y++) {
                ContourX[y][0] = std::numeric_limits<long>::max(); // min X
                ContourX[y][1] = std::numeric_limits<long>::min(); // max X
            }

            ScanLine(p0.x, p0.y, p1.x, p1.y);
            ScanLine(p1.x, p1.y, p2.x, p2.y);
            ScanLine(p2.x, p2.y, p0.x, p0.y);

            for (y = 0; y < height; y++) {
                if (ContourX[y][1] >= ContourX[y][0]) {
                    long x = ContourX[y][0];
                    long len = 1 + ContourX[y][1] - ContourX[y][0];

                    // Can draw a horizontal line instead of individual pixels here
                    while (len--) {
                        char z = (char) (round(calculateZ(p0, p1, p2, x, y)) + '0');
                        SetPixel(x++, y, z);
                    }
                }
            }
        };

        private:
            char* buffer; // display buffer
            char* zBuffer; // z-value vuffer

            int width; // screen width
            int height; // screen height

            float left = -1.0; // distance from viewer to left edge of screen
            float right = 1.0; // distance from viewer to right edge of screen
            float bottom = -1.0; // distance from viewer to bottom of screen
            float top = 1.0; // distance from viewer to top of screen
            float near = 1.0; // distance from viewer to nearest point in screen
            float far = 2.0; // distance from viewer to furthest point in screen

            /**
            * TODO
            */
            void SetPixel(long x, long y, char c) {
                if ( x < 0 || x >= width * height || y < 0 || y >= width * height) {
                    return;
                }
                char d = zBuffer[GetIndex(y,x)];

                if (d == '.') {
                    d = c;
                }

                buffer[GetIndex(y,x)] = d;
            }

            /**
            * Initializes a buffer by populating the given character at each index
            */
            void InitializeBuffer(char* buffer, char character) {
                for (int i = 0; i < (width * height); ++i)
                    buffer[i] = character;
            }

            /**
            * Given x,y values in the display screen, identify and return the equivalent location (index value) in the linear buffer.
            * Example: Given a 5x5 display screen and values x = 2 and y = 2, the returned index of the array would be index = 6
            */
            int GetIndex(int y, int x) {
                return (width * (y-1)) + x - 1;
            }

            /**
            * Given the three vertices of a triangle and some x,y values within the triangle, caluclate the z value related to the given x and y values.
            * Uses the equation plane caluclation to detemrine the z value.
            */
           // Could do inside outside test here to fer tgere barycentric coordinares (w1, w2, w3)
            float calculateZ(Point3D p1, Point3D p2, Point3D p3, int x, int y) { 
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

            /**
            * TODO: Scans a side of a triangle setting min X and max X in ContourX[][]
            // (using the Bresenham's line drawing algorithm).
            */
           // instad of this we can do the bounding box and then check every pixel
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

                if (m < n) {
                    m = ABS(sy);
                    n = ABS(sx);
                    dx2 = 0;
                    dy2 = dy1;
                }

                x = x1; y = y1;
                cnt = m + 1;
                k = n / 2;

                while (cnt--) {
                    if ((y >= 0) && (y < height)) {
                        if (x < ContourX[y][0]) ContourX[y][0] = x;
                        if (x > ContourX[y][1]) ContourX[y][1] = x;
                    }

                    k += n;
                    if (k < m) {
                        x += dx2;
                        y += dy2;
                    } else {
                        k -= m;
                        x += dx1;
                        y += dy1;
                    }
                }
            }

            /**
            * Checks if the vertex coordinates are within the range of the screen.
            */
            bool checkInputValues(Point3D vertex) {
                if (vertex.x < left || vertex.x > right || vertex.y < bottom || vertex.y > top || vertex.z < near || vertex.z > far) {
                    printf("The vertex you entered was out of range.");
                    printf("Please insert valid coordinates: \nx=[%f,%f]\ny=[%f,%f]\nz=[%f,%f]", left, right, bottom, top, near, far);
                    return false;
                }

                return true;
            }
};

/*void UserInputTriangles(Point3D v) {
    int i = 0;
    int j = i + 1;
    char answer;
    do {
        Point3D v1, v2, v3;
        std::cout << "Insert the triangle vertices:\nVertex 1: ";
        std::cin >> v1.x >> v1.y >> v1.z;
        //if user inserts an invalid input -> redo the cycle//
        if (!checkValues(v1))
            continue;
        std::cout << "\nVertex 2: ";
        std::cin >> v2.x >> v2.y >> v2.z;
        if (!checkValues(v2))
            continue;
        std::cout << "\nVertex 3: ";
        std::cin >> v3.x >> v3.y >> v3.z;
        if (!checkValues(v3))
            continue;

            //graphic interface?
        std::cout << "Would you like to insert another triangle? (y to answer yes)\n";
        std::cin >> answer;
        if (answer == 'y')
            ++j; //if yes: keep cycling (i will be equal to j-1), else i=j and the loop ends
        ++i;
        triangle tri1 = { v1, v2, v3 };
        v.drawTriangles(tri1);
    } while (i < j);
    return;
}*/

/*get vertices from user and call DrawTriangles to perform projection
* Issues:
* 1) if the user behaves "well" it seems to work...but if he inserts a char as any of the vertex coordinates everything
*    breaks
* 2) if we need to keep in memory the values of triangles, I think we need to modify something
*    because in the loop I reuse the same triangle instance everytime
*
* Let me know if you think this is a bad design. If you need clarifications text me anytime
*/
int main() {

    // Initialize the screen
    //Screen<char> screen(SCREEN_WIDTH,SCREEN_HEIGHT);
    Screen screen = Screen(SCREEN_WIDTH,SCREEN_HEIGHT);

    //UserInputTriangles(v);

    Point3D p1, p2, p3, p4;

    p1.x = 1.0;
    p1.y = -1.0;
    p1.z = 1.5;

    p2.x = 1.0;
    p2.y = 1.0;
    p2.z = 1.1;

    p3.x = -1.0;
    p3.y = 1.0;
    p3.z = 1.5;

    p4.x = -1.0;
    p4.y = -1.0;
    p4.z = 1.9;

    triangle tri1 = { p1, p2, p3 };
    triangle tri2 = { p1, p3, p4 };

    triangle triangles[2] = {tri1, tri2};

    // For each triangle in the list
    for (triangle triangle : triangles) {

        // Project its vertices from 3D -> 2D space
        triangle = screen.project3Dto2D(triangle);

        // Draw the triangle given its 3 vertices
        screen.DrawTriangle(triangle.points[0], triangle.points[1], triangle.points[2]);
    }

    screen.Visualize();

    return 0;
}