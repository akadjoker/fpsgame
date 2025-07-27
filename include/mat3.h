#pragma once

// #include <math.h>       // Required for: sinf(), cosf(), tan(), atan2f(), sqrtf(), floor(), fminf(), fmaxf(), fabsf()
// #include <raymath.h>

// 3x3 Matrix structure (column-major like OpenGL)
typedef struct Mat3 {
    float m0, m1, m2;    // 1st column (X axis)
    float m3, m4, m5;    // 2nd column (Y axis) 
    float m6, m7, m8;    // 3rd column (Z axis)
} Mat3;



 
static inline Mat3 Mat3Identity(void) {
    Mat3 result = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };
    return result;
}

static inline Mat3 Mat3FromFloat(const float rot[9]) {
    Mat3 result = {
        rot[0], rot[1], rot[2],  // linha 0: eixo X
        rot[3], rot[4], rot[5],  // linha 1: eixo Y
        rot[6], rot[7], rot[8]   // linha 2: eixo Z
    };
    return result;
}

 
static inline Mat3 Mat3FromMD3Axis(Vector3 axis[3]) {
    Mat3 result = {
        axis[0].x, axis[0].y, axis[0].z,  // X axis
        axis[1].x, axis[1].y, axis[1].z,  // Y axis
        axis[2].x, axis[2].y, axis[2].z   // Z axis
    };
    return result;
}

static inline Mat3 Mat3FromMD3AxisColumn(Vector3 axis[3]) {
    Mat3 result = {
        axis[0].x, axis[1].x, axis[2].x,  // Coluna 0 = eixo X
        axis[0].y, axis[1].y, axis[2].y,  // Coluna 1 = eixo Y
        axis[0].z, axis[1].z, axis[2].z   // Coluna 2 = eixo Z
    };
    return result;
}

// Create Mat3 from Euler angles (degrees)
static inline Mat3 Mat3FromEuler(float pitch, float yaw, float roll) {
    float cp = cosf(DEG2RAD * pitch);
    float sp = sinf(DEG2RAD * pitch);
    float cy = cosf(DEG2RAD * yaw);
    float sy = sinf(DEG2RAD * yaw);
    float cr = cosf(DEG2RAD * roll);
    float sr = sinf(DEG2RAD * roll);

    Mat3 result = {
        cy*cr - sy*sp*sr,  cy*sr + sy*sp*cr,  -sy*cp,
        -cp*sr,            cp*cr,             sp,
        sy*cr + cy*sp*sr,  sy*sr - cy*sp*cr,  cy*cp
    };
    return result;
}

 

// Convert Mat3 to Matrix (4x4)
static inline Matrix Mat3ToMatrix(Mat3 mat3) {
    Matrix result = {
        mat3.m0, mat3.m1, mat3.m2, 0.0f,
        mat3.m3, mat3.m4, mat3.m5, 0.0f,
        mat3.m6, mat3.m7, mat3.m8, 0.0f,
        0.0f,    0.0f,    0.0f,    1.0f
    };
    return result;
}

// Convert Matrix (4x4) to Mat3 (extract rotation part)
static inline Mat3 Mat3FromMatrix(Matrix mat) {
    Mat3 result = {
        mat.m0, mat.m1, mat.m2,
        mat.m4, mat.m5, mat.m6,
        mat.m8, mat.m9, mat.m10
    };
    return result;
}

// Convert Mat3 to Quaternion
static inline Quaternion Mat3ToQuaternion(const Mat3& m)
 {
    Quaternion q;
    float trace = m.m0 + m.m4 + m.m8; // soma da diagonal

    if (trace > 0.00000001f)
    {
        float s = sqrtf(trace + 1.0f) * 2.0f;
        q.w = 0.25f * s;
        q.x = (m.m7 - m.m5) / s;
        q.y = (m.m2 - m.m6) / s;
        q.z = (m.m3 - m.m1) / s;
    }
    else
    {
        if (m.m0 > m.m4 && m.m0 > m.m8)
        {
            float s = sqrtf(1.0f + m.m0 - m.m4 - m.m8) * 2.0f;
            q.w = (m.m7 - m.m5) / s;
            q.x = 0.25f * s;
            q.y = (m.m1 + m.m3) / s;
            q.z = (m.m2 + m.m6) / s;
        }
        else if (m.m4 > m.m8)
        {
            float s = sqrtf(1.0f + m.m4 - m.m0 - m.m8) * 2.0f;
            q.w = (m.m2 - m.m6) / s;
            q.x = (m.m1 + m.m3) / s;
            q.y = 0.25f * s;
            q.z = (m.m5 + m.m7) / s;
        }
        else
        {
            float s = sqrtf(1.0f + m.m8 - m.m0 - m.m4) * 2.0f;
            q.w = (m.m3 - m.m1) / s;
            q.x = (m.m2 + m.m6) / s;
            q.y = (m.m5 + m.m7) / s;
            q.z = 0.25f * s;
        }
    }

    // Normalizar
    float len = sqrtf(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
    if (len > 0.00000001f)
    {
        q.x /= len;
        q.y /= len;
        q.z /= len;
        q.w /= len;
    }

    return q;
}


 

// Multiply two Mat3 matrices
static inline Mat3 Mat3Multiply(Mat3 left, Mat3 right) {
    Mat3 result;
    
    result.m0 = left.m0*right.m0 + left.m3*right.m1 + left.m6*right.m2;
    result.m1 = left.m1*right.m0 + left.m4*right.m1 + left.m7*right.m2;
    result.m2 = left.m2*right.m0 + left.m5*right.m1 + left.m8*right.m2;
    
    result.m3 = left.m0*right.m3 + left.m3*right.m4 + left.m6*right.m5;
    result.m4 = left.m1*right.m3 + left.m4*right.m4 + left.m7*right.m5;
    result.m5 = left.m2*right.m3 + left.m5*right.m4 + left.m8*right.m5;
    
    result.m6 = left.m0*right.m6 + left.m3*right.m7 + left.m6*right.m8;
    result.m7 = left.m1*right.m6 + left.m4*right.m7 + left.m7*right.m8;
    result.m8 = left.m2*right.m6 + left.m5*right.m7 + left.m8*right.m8;
    
    return result;
}

// Transform Vector3 with Mat3
static inline Vector3 Mat3Transform(Mat3 mat, Vector3 vec) {
    Vector3 result;
    result.x = mat.m0*vec.x + mat.m3*vec.y + mat.m6*vec.z;
    result.y = mat.m1*vec.x + mat.m4*vec.y + mat.m7*vec.z;
    result.z = mat.m2*vec.x + mat.m5*vec.y + mat.m8*vec.z;
    return result;
}

 
static inline Mat3 Mat3Transpose(Mat3 mat) {
    Mat3 result = {
        mat.m0, mat.m3, mat.m6,
        mat.m1, mat.m4, mat.m7,
        mat.m2, mat.m5, mat.m8
    };
    return result;
}

 
static inline float Mat3Determinant(Mat3 mat) {
    return mat.m0 * (mat.m4*mat.m8 - mat.m5*mat.m7) -
           mat.m3 * (mat.m1*mat.m8 - mat.m2*mat.m7) +
           mat.m6 * (mat.m1*mat.m5 - mat.m2*mat.m4);
}

 
