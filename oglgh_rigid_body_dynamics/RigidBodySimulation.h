#ifndef MPRIGIDBODYSIMULATION_H_
#define MPRIGIDBODYSIMULATION_H_

#include "mpVec3.h"
#include "mpMatrix3x3.h"
#include "RigidBodyEquationWRF.h"
#include "RigidBodyEquationBRF.h"
#include <cmath>

/*
 * This class defines a simulation that uses the two rigid body equations.
 * The external forces that act on the rigid body include:
 * - the gravity force
 * - a spring force, where end1 of the spring can be positioned by the user
 *   (i.e. via a mouse position) and end2 of the spring is attached to the
 *   rigid body. The user can modify the attachment point, that is defined 
 *   in the body reference frame. The spring is simulated as a linear spring
 *   (no damping) that is present only if the displacement is greater than
 *   a given "unstretched" length. So, the spring acts more like a rope.
 * - viscous damping force that is proportional to the velocity of the center
 *   of mass of the rigid body. This force simulates the "air friction".
 *   However, the true reason why we have this force is to stabilize the 
 *   numerical integration. This is a common trick.
 *  - a viscous torque that is similar to the viscous force. Again, the primary
 *   reason for introducing this torque is to stabilize numerical integration
 *   of the orientation.
 *
 * The simulation actually integrates two rigid body equations simultaneously.
 * 1) RigidBodyEquationWRF that integrates the rigid body in the WORLD reference
 *    frame. 
 * 2) RigidBodyEquationBRF that integrates angular velocity in the BODY reference
 *    frame.
 * See the RigidBodyEquationWRF and RigidBodyEquationBRF classes for more details.
 * Since both equations are integrated simultaneously, you can verify that
 * their integration produces almost identical results.
 */

class RigidBodySimulation  {
public:

    //a few useful typedefs.
    typedef RigidBodyEquationWRF::Vec3 Vec3;
    typedef RigidBodyEquationWRF::Matrix Matrix;

    //this enum can be used to query information
    //from one of the two rigid body equations.
    enum RigidBodyEquation {

        RIGID_BODY_EQUATION_WRF,

        RIGID_BODY_EQUATION_BRF
    };

    //a little class that computes the spring force given
    //positions of the spring ends.
    struct SpringForce {

        typedef RigidBodyEquationWRF::Vec3 Vec3;
        typedef RigidBodyEquationWRF::Matrix Matrix;

        SpringForce() : m_stiffness(1.0), m_unstretchedLength(40.0){}

        void    setStiffness(double k)
        {
            if(k < 0.0) return;
            m_stiffness = k;
        }

        double getStiffness() const { return m_stiffness; }

        void   setUnstretchedLength(double l) {  m_unstretchedLength = l;  }

        double getUnstretchedLength() const { return m_unstretchedLength;  }

        void   compute(const Vec3& endPoint1, const Vec3& endPoint2, Vec3& force)
        {
            //the spring force is directed along the line that connects
            //the endPoint2 -> endPoint1.
            Vec3 direction = endPoint2 - endPoint1;

            //make it a unit vector, compute the norm 
            double length = direction.normalize();

            //subtract displacement for string length with no tension
            //This spring acts more like a rope: its force is 0 if the
            //total length of the spring is less than the unstretched length.
            double displacement = std::fabs(length) > m_unstretchedLength ? fabs(length-m_unstretchedLength) : 0.0;

            //use Hook's law for the linear-proportional spring
            force = m_stiffness * displacement * direction;
        }

    private:

        double m_stiffness;

        double m_unstretchedLength;

    };

    //This is the "cheat" torque that is used to stabilize the equations.
    //The damping coefficient should be negative.
    struct DampingTorque {

        DampingTorque() : m_damping(-30.0){}

        void   setDamping(double d)
        {
            if(d > 0.0) return;
            m_damping = d;  
        }

        double getDamping() const { return m_damping; }

        void   compute(const Vec3& angularVelocity, Vec3& torque)
        {
            torque = m_damping * angularVelocity;
        }

    private:

        double  m_damping;

    };

	RigidBodySimulation();

	virtual ~RigidBodySimulation();

    void    update(unsigned int frameNum);

    double  getSimulationTime() const { return m_simulationTime;   }

    void    resetSimulation();


    //*********************************************************************************
    // API to control the spring force
    //*********************************************************************************

    //Specify (in BODY reference frame) where the spring is attached to the 
    //rigid body.
    void    setSpringAttachmentPointBody(const Vec3& point)
    {
        m_springAttachmentPointBody = point;
    }

    const Vec3& getSpringAttachmentPointBody() const { return m_springAttachmentPointBody;  }

    const SpringForce& getSpringForce() const { return m_springForce; }
  
    SpringForce& getSpringForce() { return m_springForce; }

    void setSpringEndPoint1(const Vec3& pt) { m_springEndPoint1 = pt; }

    const Vec3&  getSpringEndPoint1() const {  return m_springEndPoint1; }

    const DampingTorque& getDampingTorque() const { return m_dampingTorque;  }

    DampingTorque& getDampingTorque() { return m_dampingTorque;  }

    //*********************************************************************************
    //API to query information from a given rigid body equation
    //*********************************************************************************

    //A wrapper to access position of the currently active rigid body
    const Vec3& getRigidBodyPosition(RigidBodyEquation eq) const
    {
        return (eq == RIGID_BODY_EQUATION_WRF ? 
            m_rigidBodyEquationWRF.getPosition() :
            m_rigidBodyEquationBRF.getPosition());
    }

    //A wrapper to access orientation of the currently active rigid body
    const Matrix& getRigidBodyOrientation(RigidBodyEquation eq) const
    {
        return (eq == RIGID_BODY_EQUATION_WRF ? 
            m_rigidBodyEquationWRF.getOrientation() :
            m_rigidBodyEquationBRF.getOrientation());
    }

    //A wrapper to access dimension of the currently active rigid body
    double getRigidBodyDimension(RigidBodyEquation eq) const
    {
        return (eq == RIGID_BODY_EQUATION_WRF ? 
            m_rigidBodyEquationWRF.getDimension() :
            m_rigidBodyEquationBRF.getDimension());
    }

    //A wrapper to the spring attachment in the WORLD reference frame
    //for the currently active rigid body
    void getSpringEndPoint2(RigidBodyEquation eq, Vec3& springEnd) const
    {

        springEnd = m_springAttachmentPointBody;
        if(eq == RIGID_BODY_EQUATION_WRF)
            m_rigidBodyEquationWRF.convertBodyToWorld(springEnd,true);
        else 
            m_rigidBodyEquationBRF.convertBodyToWorld(springEnd,true);
    }

protected:

    void    updateRigidBodyWRF(double timeBegin, double timeEnd);

    void    updateRigidBodyBRF(double timeBegin, double timeEnd);

private:

    //Variables that defines the simulation time
    double  m_simulationTime;

    //Specifies the attachment point for the second spring end
    //in the BODY reference frame.
    Vec3    m_springAttachmentPointBody;

    //position of the first spring end point in the WORLD reference frame.
    Vec3    m_springEndPoint1; 

    //*************************************************************************
    // All forces and torques are defined in the WORLD reference frame.
    //*************************************************************************
    SpringForce     m_springForce; //a placeholder of the spring coefficient

    DampingTorque   m_dampingTorque;//a placeholder for the damping coefficient

    RigidBodyEquationWRF    m_rigidBodyEquationWRF;

    RigidBodyEquationBRF    m_rigidBodyEquationBRF;
};

#endif //MPRIGIDBODYSIMULATION_H_
