// vector.h
#ifndef VECTOR_H
#define VECTOR_H

struct Vector {
    double x;
    double y;
    double z;

    // Default constructor
    Vector() : x(0), y(0), z(0) {}

    // Parameterized constructor
    Vector(double x_val, double y_val, double z_val)
        : x(x_val), y(y_val), z(z_val) {}

    // Overload addition operator
    Vector operator+(const Vector& other) const {
        return Vector(x + other.x, y + other.y, z + other.z);
    }

    // Overload multiplication operator for scalar
    Vector operator*(double scalar) const {
        return Vector(x * scalar, y * scalar, z * scalar);
    }
};

#endif // VECTOR_H
