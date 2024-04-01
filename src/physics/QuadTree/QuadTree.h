#pragma once
#include <vector>
#include "../gravBody.h"
#include "Boundary.h"
#include "../QuadTreeGPU/quadTreeStructs.h"

// Could upgrade to use generics, but it's okay
class QuadTree {
private:
    Boundary m_boundary;

public:
    GravBody* m_body;
    QuadTree* m_Q1;
    QuadTree* m_Q2;
    QuadTree* m_Q3;
    QuadTree* m_Q4;
    ~QuadTree();
    QuadTree(Boundary& boundary);
    bool insert(GravBody* bodyToInsert);
    void subdivide();
    std::vector<GravBody*> query(Boundary& range);
    std::vector<GravBody*> query(Boundary& range, std::vector<GravBody*>& result);
    GravBody* aggregateCenterAndTotalMass();
    std::vector<GravBody*> barnesHutQuery(GravBody* body, float theta);
    void barnesHutQuery(GravBody* body, float theta, std::vector<GravBody*>& result);
    bool isLeaf();
    bool isEmptyNode();
};