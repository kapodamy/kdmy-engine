#ifndef _sh4matrix_types_h
#define _sh4matrix_types_h


#define SH4MATRIX_SIZE 16


typedef __attribute__((aligned(32))) float SH4Matrix[SH4MATRIX_SIZE];


typedef enum {
    Corner_TopLeft,
    Corner_TopRight,
    Corner_BottomLeft,
    Corner_BottomRight
} Corner;

typedef struct {
    float angle;
    float x;
    float y;
} CornerRotation;

#endif
