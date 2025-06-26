#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <windows.h>

#define WIDTH       (80)
#define HEIGHT      (40)

struct point_3D {
    float x;
    float y;
    float z;
};

static float A = 0;
static float B = 0;
static float C = 0;

static float cube_width = 20;
static const int background_ascii_code = ' ';

static float z_buffer[WIDTH * HEIGHT];
static int c_buffer[WIDTH * HEIGHT];

static float distance_from_cam = 100;

float x;
float y;
float z;

int zp;
int xp;
int yp;

float ooz;

// The rotation matrix around the X-axis R_x(θ)：
/*
R_x(θ) = 
[  1      0        0   ]
[  0   cos(θ)  sin(θ)  ]
[  0  -sin(θ)  cos(θ)  ]
*/

// The rotation matrix around the Y-axis R_y(θ)：
/*
R_y(θ) = 
[ cos(θ)   0   sin(θ)  ]
[    0     1      0    ]
[ -sin(θ)   0   cos(θ) ]
*/

// The rotation matrix around the Z-axis R_z(θ)：
/*
R_z(θ) = 
[ cos(θ)   -sin(θ)   0 ]
[ sin(θ)    cos(θ)   0 ]
[    0        0      1 ]
*/

/*
R = R_x(A) * R_y(B) * R_z(C) = 
[  cos(B) * cos(C) + sin(B) * sin(C)     sin(A) * sin(B) * cos(C) + cos(A) * sin(C)   -cos(A) * sin(B) * cos(C) + sin(A) * sin(C) ]
[ -cos(B) * sin(C) + sin(B) * cos(C)    -sin(A) * sin(B) * sin(C) + cos(A) * cos(C)    cos(A) * sin(B) * sin(C) + sin(A) * cos(C) ]
[  sin(B)                               -sin(A) * cos(B)                               cos(A) * cos(B)                            ]
*/

float caculate_x(struct point_3D *point) {
    float result = 0.0f;
    result = point->x * (cos(B) * cos(C) + sin(B) * sin(C)) +
            point->y * (sin(A) * sin(B) * cos(C) + cos(A) * sin(C)) +
            point->z * (-cos(A) * sin(B) * cos(C) + sin(A) * sin(C));
    return result;
}

float caculate_y(struct point_3D *point) {
    float result = 0.0f;
    result = point->x * (-cos(B) * sin(C) + sin(B) * cos(C)) +
            point->y * (-sin(A) * sin(B) * sin(C) + cos(A) * cos(C)) +
            point->z * (cos(A) * sin(B) * sin(C) + sin(A) * cos(C));
    return result;
}

float caculate_z(struct point_3D *point) {
    float result = 0.0f;
    result = point->x * (sin(B)) +
            point->y * (-sin(A) * cos(B)) +
            point->z * (cos(A) * cos(B));
    return result;
}

/*                          |          + ___________    
                           +|           |           |
                            |           |           |
+---------------------------|           |     +     |
                            |           |           |
                            |           |___________|
                            |
----------------------------zp
----------------------------------------z

xp / x = zp / z
yp / y = zp / z

xp = x * zp / z
yp = y * zp / z

For z, if the value of ooz is larger, then the point that is closest to the canvas will be displayed.
*/

void caculate_for_point(struct point_3D *point, int c) {
    x = caculate_x(point);
    y = caculate_y(point);
    z = caculate_z(point) + distance_from_cam;

    ooz = 1 / z;
    zp = 10;

    int xp_offset =  WIDTH / 2;
    int yp_offset =  HEIGHT / 2;

    xp = (int)((x + xp_offset) + zp * ooz * x);
    yp = (int)((y + yp_offset) + zp * ooz * y);
    
    int idx = xp + yp * WIDTH;
    
    if (idx >= 0 && idx < WIDTH * HEIGHT) {
        if (ooz > z_buffer[idx]) {
            z_buffer[idx] = ooz;
            c_buffer[idx] = c;
        }
    }
}

void InitConsole(HANDLE *hConsole, CONSOLE_SCREEN_BUFFER_INFO *csbi, CHAR_INFO **backBuffer, COORD *bufferSize, COORD *bufferCoord);
void Render(HANDLE hConsole, CHAR_INFO *backBuffer);
void Draw(HANDLE hConsole, CHAR_INFO *backBuffer, int x, int y, char c);

int main(void) {
    HANDLE hConsole;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    CHAR_INFO *backBuffer;
    COORD bufferSize = {WIDTH, HEIGHT};
    COORD bufferCoord = {0, 0};

    InitConsole(&hConsole, &csbi, &backBuffer, &bufferSize, &bufferCoord);

    while (1) {
        memset(c_buffer, 0, WIDTH * HEIGHT * sizeof(int));
        memset(z_buffer, 0, WIDTH * HEIGHT * sizeof(float));

        for (int i = 0; i < WIDTH * HEIGHT; ++i) {
            backBuffer[i].Char.AsciiChar = background_ascii_code;
            backBuffer[i].Attributes = 0;
        }
        
        struct point_3D point;
        for (float i = -cube_width / 2; i < cube_width / 2; i += 0.15) {
            for (float j = -cube_width / 2; j < cube_width / 2; j += 0.15) {
                point = (struct point_3D){i, j, -cube_width / 2};
                caculate_for_point(&point, '@');
                point = (struct point_3D){cube_width / 2, j, i};
                caculate_for_point(&point, '$');
                point = (struct point_3D){-cube_width / 2, j, -i};
                caculate_for_point(&point, '-');
                point = (struct point_3D){-i, j, cube_width / 2};
                caculate_for_point(&point, '#');
                point = (struct point_3D){i, -cube_width / 2, -j};
                caculate_for_point(&point, ';');
                point = (struct point_3D){i, cube_width / 2, j};
                caculate_for_point(&point, '+');
            }
        }

        for (int k = 0; k < HEIGHT * WIDTH; ++k) {
            Draw(hConsole, backBuffer, k % WIDTH, k / WIDTH, c_buffer[k]);
        }

        Render(hConsole, backBuffer);

        A += 0.05;
        B += 0.06;
        C += 0.07;

        Sleep(16);
    }
    return 0;
}

void InitConsole(HANDLE *hConsole, CONSOLE_SCREEN_BUFFER_INFO *csbi, CHAR_INFO **backBuffer, COORD *bufferSize, COORD *bufferCoord) {
    *hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(*hConsole, csbi);
    SetConsoleScreenBufferSize(*hConsole, *bufferSize);
    *backBuffer = (CHAR_INFO*)malloc(sizeof(CHAR_INFO) * bufferSize->X * bufferSize->Y);
    SMALL_RECT rect = {0, 0, bufferSize->X - 1, bufferSize->Y - 1};
    SetConsoleWindowInfo(*hConsole, TRUE, &rect);
}

void Render(HANDLE hConsole, CHAR_INFO *backBuffer) {
    WriteConsoleOutput(hConsole, backBuffer, (COORD){WIDTH, HEIGHT}, (COORD){0, 0}, &(SMALL_RECT){0, 0, WIDTH - 1, HEIGHT - 1});
}

void Draw(HANDLE hConsole, CHAR_INFO *backBuffer, int x, int y, char c) {
    int idx = x + y * WIDTH;
    if (idx >= 0 && idx < WIDTH * HEIGHT) {
        backBuffer[idx].Char.AsciiChar = c;
        backBuffer[idx].Attributes = 0x07;
    }
}
