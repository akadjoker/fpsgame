#pragma once
#include "Config.hpp"
#include <raylib.h>
#include <raymath.h>
#include <vector>


class MatrixStack 
{
private:
    std::vector<Matrix> stack;

public:
    MatrixStack();

    void LoadIdentity();
    void PushMatrix();
    void PopMatrix();
    const Matrix& Top() const;
    void Reset();

    // Transforms
    void Translate(float x, float y, float z);
    void Translate(const Vector3& translation);

    void Rotate(float angle, float x, float y, float z);
    void Rotate(float angle, const Vector3& axis);
    void Rotate(const Quaternion& quat);
    void RotateEuler(float pitch, float yaw, float roll);
    void RotateX(float angle);
    void RotateY(float angle);
    void RotateZ(float angle);

    void Scale(float x, float y, float z);
    void Scale(const Vector3& scale);
    void Scale(float uniform);

    void MultMatrix(const Matrix& matrix);
    void LoadMatrix(const Matrix& matrix);

 
    size_t Depth() const;
    bool Empty() const;

    Vector3 TransformPoint(const Vector3& point) const;
    Vector3 TransformDirection(const Vector3& direction) const;

    Vector3 GetRight() const;
    Vector3 GetUp() const;
    Vector3 GetForward() const;
    Vector3 GetPosition() const;

 
};