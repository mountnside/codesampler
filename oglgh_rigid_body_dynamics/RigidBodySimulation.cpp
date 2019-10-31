#include "RigidBodySimulation.h"
#include "mpMathUtils.h"
#include <cmath>
#include <iostream>

//it is ok to use the "using" declaration in a .cpp compilation unit
using namespace mp;


RigidBodySimulation::RigidBodySimulation() :
 m_simulationTime(0.0)
{
    resetSimulation();
}

RigidBodySimulation::~RigidBodySimulation() 
{

}

void RigidBodySimulation::resetSimulation()
{
    m_simulationTime = 0.0;

    //setup the spring attachments
    m_springEndPoint1[0] = 0.0; m_springEndPoint1[1] = 50.0; m_springEndPoint1[2] = -420.0;

    double unstretchedSpringLength = 40.0;
    m_springForce.setUnstretchedLength(unstretchedSpringLength);

    //reset rigid body position
    m_rigidBodyEquationWRF.getPosition().set(m_springEndPoint1[0],
        -unstretchedSpringLength + m_springEndPoint1[1],m_springEndPoint1[2]);

    //reset rotation
    m_rigidBodyEquationWRF.getOrientation().makeIdentity();

    //reset Linear momentum
    m_rigidBodyEquationWRF.getLinearMomentum().set(0.0,0.0,0.0);

    //reset angular momentum
    m_rigidBodyEquationWRF.getAngularMomentum().set(0.0,0.0,0.0);

    //reset rigid body position
    m_rigidBodyEquationBRF.getPosition().set(m_springEndPoint1[0],
        -unstretchedSpringLength + m_springEndPoint1[1],m_springEndPoint1[2]);

    //reset rotation
    m_rigidBodyEquationBRF.getOrientation().makeIdentity();

    //reset Linear momentum
    m_rigidBodyEquationBRF.getLinearMomentum().set(0.0,0.0,0.0);

    //reset angular velocity
    m_rigidBodyEquationBRF.getAngularVelocity().set(0.0,0.0,0.0);

    //initialize the spring attachment point
    double dim = m_rigidBodyEquationWRF.getDimension();

    m_springAttachmentPointBody.set(dim/2.0,dim/2.0,dim/2.0);
}

void RigidBodySimulation::update(unsigned int frameNum)
{
    m_simulationTime = frameNum/60.0; //assume frame rate of 60Hz

    //the integration interval is [timeBegin, timeEnd]
    double timeBegin = m_simulationTime;
    double timeEnd = m_simulationTime + 1.0/60.0;

    updateRigidBodyWRF(timeBegin,timeEnd);
    updateRigidBodyBRF(timeBegin,timeEnd);
}

void RigidBodySimulation::updateRigidBodyWRF(double timeBegin, double timeEnd)
{
    //update forces in the WORLD reference frame.

    Vec3 forceWorld;

    //The gravitational force
    forceWorld[0] = 0.0;
    forceWorld[1] = -m_rigidBodyEquationWRF.getMass() * 9.8;
    forceWorld[2] = 0.0;

    //The spring force
    //recompute the attachment point in the WORLD reference frame
    Vec3 springAttachmentPoint = m_springAttachmentPointBody;
    m_rigidBodyEquationWRF.convertBodyToWorld(springAttachmentPoint,true);

    Vec3 springForce;

    m_springForce.compute(springAttachmentPoint,m_springEndPoint1,springForce);

    forceWorld += springForce;

    double dampingCM = -0.1;// 
    //viscous damping that simulates the air friction.
    //We use this force mostly to numerically stabilize the equations
    Vec3 velocityCenterMass = m_rigidBodyEquationWRF.getLinearMomentum();
    velocityCenterMass *= (1.0/m_rigidBodyEquationWRF.getMass());
    //add a force that is proportional to velocity of the center of mass
    forceWorld += dampingCM * velocityCenterMass;

    m_rigidBodyEquationWRF.setForce(forceWorld);

    //Now, compute torques
    //The gravity torque about the center of mass is 0 because 
    //the force is applied at the center of mass.
    Vec3 torqueWorld;

    //The spring torque in the WORLD reference frame
    //a vector from the center of mass to the point of application of the
    //spring force
    Vec3 springMoment = springAttachmentPoint - m_rigidBodyEquationWRF.getPosition();
    //compute torque about the Center of mass in the World Ref. frame
    //as a cross product
    springMoment = mp::cross(springMoment,springForce);

    torqueWorld += springMoment;

    Vec3 dampingTorque;

    Vec3 omegaWRF;
    m_rigidBodyEquationWRF.computeAngularVelocity(omegaWRF);

    m_dampingTorque.compute(omegaWRF, dampingTorque);

    torqueWorld += dampingTorque;

    m_rigidBodyEquationWRF.setTorque(torqueWorld);

    m_rigidBodyEquationWRF.integrate(timeBegin,timeEnd);
}

void RigidBodySimulation::updateRigidBodyBRF(double timeBegin, double timeEnd)
{
    //update forces in the WORLD reference frame.

    Vec3 forceWorld;

    //The gravitational force
    forceWorld[0] = 0.0;
    forceWorld[1] = -m_rigidBodyEquationWRF.getMass() * 9.8;
    forceWorld[2] = 0.0;

    //The spring force
    //recompute the attachment point in the WORLD reference frame
    Vec3 springAttachmentPoint = m_springAttachmentPointBody;
    m_rigidBodyEquationBRF.convertBodyToWorld(springAttachmentPoint,true);

    Vec3 springForce;

    m_springForce.compute(springAttachmentPoint,m_springEndPoint1,springForce);

    forceWorld += springForce;

    double dampingCM = -0.1;// 
    //viscous damping that simulates the air friction.
    //We use this force mostly to numerically stabilize the equations
    Vec3 velocityCenterMass = m_rigidBodyEquationBRF.getLinearMomentum();
    velocityCenterMass *= (1.0/m_rigidBodyEquationBRF.getMass());
    //add a force that is proportional to velocity of the center of mass
    forceWorld += dampingCM * velocityCenterMass;

    m_rigidBodyEquationBRF.setForce(forceWorld);

    //Now, compute torques
    //The gravity torque about the center of mass is 0 because 
    //the force is applied at the center of mass.
    Vec3 torque;

    //The spring torque in the WORLD reference frame
    //a vector from the center of mass to the point of application of the
    //spring force
    Vec3 springMoment = springAttachmentPoint - m_rigidBodyEquationBRF.getPosition();
    //compute torque about the Center of mass in the World Ref. frame
    //as a cross product
    springMoment = mp::cross(springMoment,springForce);

    torque += springMoment;

    //So far, the torque was in the WORLD reference frame.
    //Torque needs to be converted to the BODY ref. frame
    m_rigidBodyEquationBRF.convertWorldToBody(torque,false);

    //the damping torque is in the BODY reference frame
    Vec3 dampingTorque;

    //angular velocity in the BODY reference frame
     m_dampingTorque.compute(m_rigidBodyEquationBRF.getAngularVelocity(), dampingTorque);

    torque += dampingTorque;

    m_rigidBodyEquationBRF.setTorque(torque);

    m_rigidBodyEquationBRF.integrate(timeBegin,timeEnd);
}

