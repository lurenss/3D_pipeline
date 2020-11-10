#include <iostream>
#include <limits>
#include <stdio.h>
#include <cmath>

// A point with x,y,z coordinates
struct Point3D {
    float x, y, z;
};

// A triangle composed of 3 points
struct Triangle {
    Point3D points[3];
};

// A 4x4 matrix used for projection calculations
struct Matrix4x4 {
	float m[4][4];
};

// Screen width and height measurements
#define SCREEN_HEIGHT 50
#define SCREEN_WIDTH 150
#define SCREEN_INIT_CHAR '.'

// Absolute value definition
#define ABS(x) ((x >= 0) ? x : -x)

// Holds the minimum and maximum X value for every horizontal line within the triangle
long ContourX[SCREEN_HEIGHT][2];

template <class target_t>
class Screen {
    public:
        Screen(int screenWidth, int screenHeight) {
            width = screenWidth;
            height = screenHeight;
            
            // Initialize the screen's display buffer
            buffer = new target_t [width * height]; 
            InitializeBuffer(buffer, SCREEN_INIT_CHAR);

            // Initialize the screen's z-value buffer
            zBuffer = new target_t [width * height];
            InitializeBuffer(zBuffer, SCREEN_INIT_CHAR);
        }

        /**
        * Visualizes the screen by printing the value of the display buffer at each pixel
        */
        void Visualize() {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    std::cout << buffer[GetIndex(y,x)];
                }
                std::cout << "\n";
            }
        }

        /**
        * Use perspective projection and normalization to convert the 3D coordinates into 2D coordinates that fit the display screen
        */
        Triangle ProjectCoordinates(Triangle tri) {

            // Perspective projection matrix specifications (column major matrix)
            Matrix4x4 projectionMatrix = { 0 };
            projectionMatrix.m[0][0] = (2 * near) / (right - left);
            projectionMatrix.m[1][1] = (2 * near) / (bottom - top);
            projectionMatrix.m[2][2] = (far + near) / (far - near);
            projectionMatrix.m[2][1] = -(bottom + top) / (bottom - top);
            projectionMatrix.m[3][2] = (-2 * near * far) / (far - near);
            projectionMatrix.m[2][0] = -(right + left) / (right - left);
            projectionMatrix.m[2][3] = 1.0;

            // For each vertex in the triangle, project its coordinates from 3D -> 2D based on the perspective projection matrix
            Triangle triProjected;
            MultiplyVectorMatrix(tri.points[0], triProjected.points[0], projectionMatrix);
            MultiplyVectorMatrix(tri.points[1], triProjected.points[1], projectionMatrix);
            MultiplyVectorMatrix(tri.points[2], triProjected.points[2], projectionMatrix);

            return triProjected;
        }

		/**
        * Normalize the projected vertices to fit the display screen buffer.
        */
		Triangle NormalizeCoordinates(Triangle triProjected) {

            Triangle triNormalized;
        
            // Normalize each vertex of the projected triangle
            for (int i = 0; i < 3; i++) {
            
                // Go from float values within x range [-1, 1] to integer values within the display screen width range [0, SCREEN_WIDTH)
                triNormalized.points[i].x = ((triProjected.points[i].x + 1) * (width)) / 2;

                // Go from float values within x range [-1, 1] to integer values within the display screen height range [0, SCREEN_HEIGHT)
                triNormalized.points[i].y = ((triProjected.points[i].y + 1) * (height)) / 2;

                // Go from float values within z range [-1, 1] to integer values within [0, 9]
                int z = ((triProjected.points[i].z + 1) * 10) / 2;
                triNormalized.points[i].z = z;

                // Set the zBuffer value for the normalized x,y coordinates
                // Note: If there is there is already an integer value present at that zBuffer index that is closer to the camera, do not insert a new value
                char zValue = zBuffer[GetIndex(triNormalized.points[i].y, triNormalized.points[i].x)];
                if (zValue == SCREEN_INIT_CHAR || zValue > z) {
                    zBuffer[GetIndex(triNormalized.points[i].y, triNormalized.points[i].x)] = (char) (z + '0');
                }
            }

            return triNormalized;
		}

        /**
        * Draw the triangle given its three vertices
        */
        void DrawTriangle(Point3D p0, Point3D p1, Point3D p2) {
            int y;

            // For each y value, find the min and max x value
            for (y = 0; y < height; y++) {
                ContourX[y][0] = std::numeric_limits<long>::max(); // min X
                ContourX[y][1] = std::numeric_limits<long>::min(); // max X
            }

            // Scan the three edge lines of the triangle
            ScanLine(p0.x, p0.y, p1.x, p1.y);
            ScanLine(p1.x, p1.y, p2.x, p2.y);
            ScanLine(p2.x, p2.y, p0.x, p0.y);

            // Fill in the triangle
            for (y = 0; y < height; y++) {
                if (ContourX[y][1] >= ContourX[y][0]) {
                    long x = ContourX[y][0];
                    long len = 1 + ContourX[y][1] - ContourX[y][0];

                    while (len--) {
                        // Calculate the z value for each x,y coorodinate pair within the triangle
                        target_t z = (CalculateZ(p0, p1, p2, x, y)) + '0';

                        // Set each pixel within the triangle
                        SetPixel(x++, y, z);
                    }
                }
            }
        };

        private:
            target_t* buffer; // display buffer
            target_t* zBuffer; // z-value vuffer

            int width; // screen width
            int height; // screen height

            float left = -1.0; // distance from viewer to left edge of screen
            float right = 1.0; // distance from viewer to right edge of screen
            float bottom = 1.0; // distance from viewer to bottom of screen
            float top = -1.0; // distance from viewer to top of screen
            float near = 1.0; // distance from viewer to nearest point in screen
            float far = 2.0; // distance from viewer to furthest point in screen

            /**
            * This fragment shader sets a pixel in the display screen to the given character
            */
            void SetPixel(int x, int y, target_t t) {
                target_t zValue = zBuffer[GetIndex(y,x)];

                if (zValue == SCREEN_INIT_CHAR) {
                    zValue = t;
                }

                buffer[GetIndex(y,x)] = zValue;
            }

            /**
            * Initializes a buffer by populating the given character at each index
            */
            void InitializeBuffer(target_t* buffer, target_t character) {
                for (int i = 0; i < (width * height); ++i) {
                    buffer[i] = character;
                }
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
            * Uses the equation plane caluclation to determine the z value.
            */
            float CalculateZ(Point3D p1, Point3D p2, Point3D p3, int x, int y) { 
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
            * Multiply a 3DPoint of (x,y,z) cordinates with a by a projection matrix.
            * Note: This is column major multiplication and the projection matrix is also column major oriented.
            */
            void MultiplyVectorMatrix(Point3D &input, Point3D &output, Matrix4x4 &matrix) {
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
            * Scans the edge of a triangle setting its min and max x value in ContourX[][]
            * Uses Bresenham's line drawing algorithm.
            */
            void ScanLine(long x1, long y1, long x2, long y2) {
                long sx, sy, dx1, dy1, dx2, dy2, x, y, m, n, k, cnt;

                sx = x2 - x1;
                sy = y2 - y1;

                if (sx > 0) {
                    dx1 = 1;
                } else if (sx < 0) {
                     dx1 = -1;
                } else {
                    dx1 = 0;
                }

                if (sy > 0) {
                    dy1 = 1;
                } else if (sy < 0) {
                    dy1 = -1;
                } else {
                    dy1 = 0;
                }

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
                        if (x < ContourX[y][0]) {
                            ContourX[y][0] = x;
                        }
                        if (x > ContourX[y][1]) {
                            ContourX[y][1] = x;
                        }
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
};

int main() {

    // Initialize the screen
    Screen<char> screen = Screen<char>(SCREEN_WIDTH,SCREEN_HEIGHT);

    // Hard-coded points which match the Assignment 1 Example on Moodle
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

    Triangle tri1 = { p1, p2, p3 };
    Triangle tri2 = { p1, p3, p4 };

    Triangle triangles[2] = {tri1, tri2};

    // For each triangle in the list
    for (Triangle triangle : triangles) {

        // Project its vertex coordinates from 3D -> 2D space
        triangle = screen.ProjectCoordinates(triangle);

         // Normalize its vertex coordinates to fit the display screen
        triangle = screen.NormalizeCoordinates(triangle);

        // Draw the triangle
        screen.DrawTriangle(triangle.points[0], triangle.points[1], triangle.points[2]);
    }

    screen.Visualize();

    return 0;
}