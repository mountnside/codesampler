#include "RigidBodyEquationWRF.h"
#include "mpMathUtils.h"
#include <cmath>

//it is ok to use the "using" declaration in a .cpp compilation unit
using namespace mp;

RigidBodyEquationWRF::RigidBodyEquationWRF() :
m_dimension(40.0)
{
    //define the mass properties
    m_mass = 1.0;

    //set the inertia matrix. 
    //The body axes are assumed to be the principle axes of inertia.
    //Therefore, the matrix is diagonal, although it doesn't need to be.
    //Simply plug in your matrix here.
    m_inertiaMatrixBody.set(1.0,0.0,0.0,
        0.0,1.0,0.0,
        0.0,0.0,1.0);
    m_inertiaMatrixBodyInverse = m_inertiaMatrixBody;
    m_inertiaMatrixBodyInverse.invert();

    resetInitialConditions();
}

RigidBodyEquationWRF::~RigidBodyEquationWRF() 
{
}

void RigidBodyEquationWRF::resetInitialConditions()
{
    m_position.set(0.0,0.0,0.0);

    //reset Linear momentum
    m_linearMomentum.set(0.0,0.0,0.0);

    //reset rotation
    m_orientation.makeIdentity();

    //reset angular momentum

    //initial angular velocity in World ref. frame
    Vec3 omega(0.0,0.0,0.0);

    Matrix inertiaWorld(m_orientation);
    inertiaWorld.transpose();
    inertiaWorld.preMultiply(m_inertiaMatrixBody);
    inertiaWorld.preMultiply(m_orientation);

    m_angularMomentum = inertiaWorld * omega;
}

void RigidBodyEquationWRF::integrate(double timeBegin, double timeEnd)
{
    //Try to avoid numerical problems by making the integration step small.
    //FWD Euler is known for its numerical instabilities.
    //So, we ignore timeBegin and timeEnd for now.
    double dT = 0.01;

    //use simple FWD Euler integration for now...

    //simple FWD Euler
    m_position[0] += dT * m_linearMomentum[0]/m_mass;
    m_position[1] += dT * m_linearMomentum[1]/m_mass;
    m_position[2] += dT * m_linearMomentum[2]/m_mass;

    m_linearMomentum[0] += dT * m_force[0];
    m_linearMomentum[1] += dT * m_force[1];
    m_linearMomentum[2] += dT * m_force[2];

    //Inertia inverse is computed as Rotation * Ibody_inverse * Rotation_transpose

    //Rotation and angular momentum are integrated in the World reference frame
    Vec3 omega;

    computeAngularVelocity(omega);

    //Time derivative of the orientation matrix
    //is computed as [skew-symmetric-omega] * [orientaiton]
    //form the skew-symmetric matrix
    Matrix dR = makeSkewSymmetric(omega);

    //multiply by the orientation
    dR *= m_orientation;

    //integrate orientation using FWD Euler
    m_orientation += dT*dR;

    //Angular momentum is integrated in the World ref. frame
    //integrate angular momentum using FWD Euler
    m_angularMomentum += dT * m_torque;

    //avoid accumulation of numerical error
    //Otherwise, you may observe strange-looking
    //scaling artifacts in the orientation matrix
    mp::orthonormalize(m_orientation);
}

void RigidBodyEquationWRF::computeAngularVelocity(Vec3& omega) const
{
    //Rotation and angular momentum are integrated in the World reference frame
    Matrix inertiaInverse = m_orientation;

    inertiaInverse.transpose();

    inertiaInverse.preMultiply(m_inertiaMatrixBodyInverse);

    inertiaInverse.preMultiply(m_orientation);

    omega = inertiaInverse * m_angularMomentum;
}

//Converts a given vector (position) from the WORLD to the BODY
//reference frame.
void RigidBodyEquationWRF::convertWorldToBody(Vec3& r, bool bIsPosition) const
{
    //Use the transpose of the orientation matrix for the conversion.
    Matrix orientT = m_orientation;
    orientT.transpose();

    r = orientT * r;

    if(bIsPosition)
    {
        r += m_position;
    }
}

//Converts a given vector (position) from the BODY to the WORLD
//reference frame.
void RigidBodyEquationWRF::convertBodyToWorld(Vec3& r, bool bIsPosition) const
{
    //Use the orientation matrix to convert
    r = m_orientation * r;

    if(bIsPosition)
    {
        r += m_position;
    }
}

