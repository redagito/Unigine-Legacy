#pragma once

class Joint;
class RigidBody;

class Physic 
{
public:
	
	static void update(float ifps);

protected:
	
	friend class RigidBody;
	friend class Joint;
	friend class JointBall;
	friend class JointHinge;
	friend class JointUniversal;
	
	static float time;
	static float time_step;
	static float gravitation;
	static float velocity_threshold;
	static float angularVelocity_threshold;
	static float time_to_frost;
	static int num_first_iterations;
	static int num_second_iterations;
	
	static int all_joints;
	static int num_joints;
	static Joint **joints;
	
	static int all_rigidbodies;
	static int num_rigidbodies;
	static RigidBody **rigidbodies;
};