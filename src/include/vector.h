// vector.h
#ifndef VECTOR_H
#define VECTOR_H

struct Vector {
    double x;
    double y;
    double z;

    // Default constructor
    Vector() : x(0.0), y(0.0), z(0.0) {}

    // Constructor with parameters
    Vector(double xVal, double yVal, double zVal) : x(xVal), y(yVal), z(zVal) {}
};

#endif // VECTOR_H
