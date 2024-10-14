#ifndef PLANE_H
#define PLANE_H

#include <string>

typedef struct{
 float x;
 float y;
 float z;
}Vector;


class Plane{
  public:
    Plane();
    Plane(std::string _id, Vector position, Vector speed);
    ~Plane();

    Vector get_pos () const;
    Vector get_speed() const;
    std::string get_id() const;

    void set_velocity(Vector speed);
    void set_pos(Vector position);

    Vector get_next_position(); //  calculate new position based on velocity and return 0 if successful
   private:
     std::string id;
    Vector position;
    Vector velocity;
};




#endif //PLANE_H
