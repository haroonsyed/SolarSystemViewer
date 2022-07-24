#pragma once
#include "../gravBody.h"
#include "Boundary.h"

// Could upgrade to use generics, but it's okay
class QuadTree {
private:
    Boundary m_boundary;
    GravBody* m_body;
    QuadTree* m_Q1;
    QuadTree* m_Q2;
    QuadTree* m_Q3;
    QuadTree* m_Q4;

public:
    QuadTree(Boundary boundary);
    bool insert(GravBody* bodyToInsert);
    void subdivide();
};