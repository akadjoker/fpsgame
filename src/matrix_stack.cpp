 
#include "matrix_stack.hpp"
#include "node.hpp"

MatrixStack::MatrixStack()
{
    stack.reserve(32);
    LoadIdentity();
}

void MatrixStack::LoadIdentity()
{
    if (stack.empty())
        stack.push_back(MatrixIdentity());
    else
        stack.back() = MatrixIdentity();
}

void MatrixStack::PushMatrix()
{
    if (stack.empty())
        stack.push_back(MatrixIdentity());
    else
        stack.push_back(stack.back());
}

void MatrixStack::PopMatrix()
{
    if (stack.size() > 1)
        stack.pop_back();
}

const Matrix& MatrixStack::Top() const
{
    static Matrix identity = MatrixIdentity();
    return stack.empty() ? identity : stack.back();
}

void MatrixStack::Reset()
{
    stack.clear();
    LoadIdentity();
}

void MatrixStack::Translate(float x, float y, float z)
{
    if (!stack.empty())
        stack.back() = MatrixMultiply(stack.back(), MatrixTranslate(x, y, z));
}

void MatrixStack::Translate(const Vector3& translation)
{
    Translate(translation.x, translation.y, translation.z);
}

void MatrixStack::Rotate(float angle, float x, float y, float z)
{
    if (!stack.empty())
        stack.back() = MatrixMultiply(stack.back(), MatrixRotate({x, y, z}, angle * DEG2RAD));
}

void MatrixStack::Rotate(float angle, const Vector3& axis)
{
    Rotate(angle, axis.x, axis.y, axis.z);
}

void MatrixStack::Rotate(const Quaternion& quat)
{
    if (!stack.empty())
        stack.back() = MatrixMultiply(stack.back(), QuaternionToMatrix(quat));
}

void MatrixStack::RotateEuler(float pitch, float yaw, float roll)
{
    Rotate(QuaternionFromEuler(pitch * DEG2RAD, yaw * DEG2RAD, roll * DEG2RAD));
}

void MatrixStack::RotateX(float angle) { Rotate(angle, 1.0f, 0.0f, 0.0f); }
void MatrixStack::RotateY(float angle) { Rotate(angle, 0.0f, 1.0f, 0.0f); }
void MatrixStack::RotateZ(float angle) { Rotate(angle, 0.0f, 0.0f, 1.0f); }

void MatrixStack::Scale(float x, float y, float z)
{
    if (!stack.empty())
        stack.back() = MatrixMultiply(stack.back(), MatrixScale(x, y, z));
}

void MatrixStack::Scale(const Vector3& scale)
{
    Scale(scale.x, scale.y, scale.z);
}

void MatrixStack::Scale(float uniform)
{
    Scale(uniform, uniform, uniform);
}

void MatrixStack::MultMatrix(const Matrix& matrix)
{
    if (!stack.empty())
        stack.back() = MatrixMultiply(stack.back(), matrix);
}

void MatrixStack::LoadMatrix(const Matrix& matrix)
{
    if (stack.empty())
        stack.push_back(matrix);
    else
        stack.back() = matrix;
}

size_t MatrixStack::Depth() const { return stack.size(); }
bool MatrixStack::Empty() const { return stack.empty(); }

Vector3 MatrixStack::TransformPoint(const Vector3& point) const
{
    return Vector3Transform(point, Top());
}

Vector3 MatrixStack::TransformDirection(const Vector3& direction) const
{
    Matrix mat = Top();
    return {
        mat.m0 * direction.x + mat.m4 * direction.y + mat.m8 * direction.z,
        mat.m1 * direction.x + mat.m5 * direction.y + mat.m9 * direction.z,
        mat.m2 * direction.x + mat.m6 * direction.y + mat.m10 * direction.z
    };
}

Vector3 MatrixStack::GetRight() const
{
    Matrix mat = Top();
    return { mat.m0, mat.m1, mat.m2 };
}

Vector3 MatrixStack::GetUp() const
{
    Matrix mat = Top();
    return { mat.m4, mat.m5, mat.m6 };
}

Vector3 MatrixStack::GetForward() const
{
    Matrix mat = Top();
    return { -mat.m8, -mat.m9, -mat.m10 };
}

Vector3 MatrixStack::GetPosition() const
{
    Matrix mat = Top();
    return { mat.m12, mat.m13, mat.m14 };
}

// void MatrixStack::ApplyMD3Tag(const Vector3& origin, const Vector3 axis[3], bool convertCoords)
// {
//     if (convertCoords)
//     {
//         Vector3 convertedOrigin = { origin.x, origin.z, -origin.y };

//         Matrix tagMatrix = {
//             axis[0].x, axis[0].z, -axis[0].y, convertedOrigin.x,
//             axis[1].x, axis[1].z, -axis[1].y, convertedOrigin.y,
//             axis[2].x, axis[2].z, -axis[2].y, convertedOrigin.z,
//             0.0f,      0.0f,      0.0f,       1.0f
//         };

//         MultMatrix(tagMatrix);
//     }
//     else
//     {
//         Matrix tagMatrix = {
//             axis[0].x, axis[0].y, axis[0].z, origin.x,
//             axis[1].x, axis[1].y, axis[1].z, origin.y,
//             axis[2].x, axis[2].y, axis[2].z, origin.z,
//             0.0f,      0.0f,      0.0f,      1.0f
//         };

//         MultMatrix(tagMatrix);
//     }
// }
