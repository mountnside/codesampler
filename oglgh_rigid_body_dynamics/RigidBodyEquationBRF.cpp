#include "RigidBodyEquationBRF.h"
#include "mpMathUtils.h"
#include <cmath>

//it is ok to use the "using" declaration in a .cpp compilation unit
using namespace mp;

RigidBodyEquationBRF::RigidBodyEquationBRF() :
m_dimension(40.0)
{
    //define the mass properties
    m_mass = 1.0;

    //set the principle moments of inertia. 
    m_principleMomentsOfInertia.set(1.0,1.0,1.0);

    resetInitialConditions();
}

RigidBodyEquationBRF::~RigidBodyEquationBRF() 
{
}

void RigidBodyEquationBRF::resetInitialConditions()
{
    m_position.set(0.0,0.0,0.0);

    //reset Linear momentum
    m_linearMomentum.set(0.0,0.0,0.0);

    //reset rotation
    m_orientation.makeIdentity();

    //initial angular velocity in the BODY ref. frame
    m_angularVelocity.set(0.0,0.0,0.0);
}

void RigidBodyEquationBRF::integrate(double timeBegin, double timeEnd)
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

    //Rotation is integrated in the World reference frame.
    //Therefore, we need to convert angular velocity to the WORLD ref. frame
    Vec3 omegaWorld = m_angularVelocity;
    convertBodyToWorld(omegaWorld,false);

    //Time derivative of the orientation matrix
    //is computed as [skew-symmetric-omega] * [orientaiton]
    //form the skew-symmetric matrix
    Matrix dR = makeSkewSymmetric(omegaWorld);

    //multiply by the orientation
    dR *= m_orientation;

    //integrate orientation using FWD Euler
    m_orientation += dT*dR;

    //angular velocity is integrated in the BODY reference frame:
    // dw/dt = I_inverse * (torque - w x Iw)
    // This equation is simplified when using the Principle axes of inertia
    Vec3 dwdt; //angular acceleration
    //Principle moments of inertia about the corresponding axis
    //We introduce these temporaries to make the equations easier to read.
    double Ix = m_principleMomentsOfInertia[0];
    double Iy = m_principleMomentsOfInertia[1];
    double Iz = m_principleMomentsOfInertia[2];
    dwdt[0] = (m_torque[0] + (Iy - Iz)*m_angularVelocity[1]*m_angularVelocity[2])/Ix;
    dwdt[1] = (m_torque[1] + (Iz - Ix)*m_angularVelocity[2]*m_angularVelocity[0])/Iy;
    dwdt[2] = (m_torque[2] + (Ix - Iy)*m_angularVelocity[0]*m_angularVelocity[1])/Iz;

    //FWD Euler step
    m_angularVelocity += dT * dwdt;

    //avoid accumulation of numerical error
    //Otherwise, you may observe strange-looking
    //scaling artifacts in the orientation matrix
    mp::orthonormalize(m_orientation);
}

void RigidBodyEquationBRF::computeAngularMomentum(Vec3& momentum) const
{
    //Angular momentum in the BODY reference frame is equal to
    // Inertia * omega
    momentum = m_angularVelocity;

    //The inertia matrix is diagonal when using the principle inertia axes
    momentum[0] *= m_principleMomentsOfInertia[0];
    momentum[1] *= m_principleMomentsOfInertia[1];
    momentum[2] *= m_principleMomentsOfInertia[2];
}

//Converts a given vector (position) from the WORLD to the BODY
//reference frame.
void RigidBodyEquationBRF::convertWorldToBody(Vec3& r, bool bIsPosition) const
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
void RigidBodyEquationBRF::convertBodyToWorld(Vec3& r, bool bIsPosition) const
{
    //Use the orientation matrix to convert
    r = m_orientation * r;

    if(bIsPosition)
    {
        r += m_position;
    }
}

