#pragma once
#include <vector>
#include "gravBody.h"
#include "QuadTreeGPU/quadTreeStructs.h"

class System {
  private:
    float G = 6.67430e-11; // Modified by scale factor!

    std::vector<GravBody*> m_bodies; // Used for cpu calculations

    unsigned int m_SSBO_BODIES;       // Used for gpu calculations
    unsigned int m_SSBO_BODIES_OUTPUT;       // Used for gpu calculations
    unsigned int m_SSBO_TREE;         // Used for gpu calculations
    unsigned int m_SSBO_BODIES_COUNT; // Used for gpu calculations
    unsigned int m_SSBO_TREE_COUNT;   // Used for gpu calculations


    float m_timeFactor;

    // The scaling factor is needed to avoid float errors with using just SI units.
    float m_SIUnitScaleFactor;

    void updateUsingBarnesHut(float adjustedTimeFactor);
    void updateUsingNaive(float adjustedTimeFactor);
    void updateUsingNaiveGPU(float adjustedTimeFactor);
    void updateUsingBarnesHutGPU(float adjustedTimeFactor);

  public:
    System();
    float getSIUnitScaleFactor();
    void setSIUnitScaleFactor(float physicsDistanceFactor);
    void setTimeFactor(float timeFactor);
    void addBody(GravBody* body);
    void setBodiesGPU(std::vector<Body>& bodies);
    std::vector<GravBody*> getBodies();
    void update(float deltaT);
    unsigned int getNumberOfGPUBodies();
    unsigned int getBodiesSSBO();
};