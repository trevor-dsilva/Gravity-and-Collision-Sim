#include "PhysicsObject.h"

PhysicsObject::PhysicsObject()
{
	position = Eigen::Vector3f::Zero();
	velocity = Eigen::Vector3f::Zero();
	angle = 0.0f;
	mass = 1.0f;
}

PhysicsObject::PhysicsObject(Eigen::Vector3f pos, Eigen::Vector3f vel, float mass)
{
	position = pos;
	velocity = vel;
	angle = 0.0f;
	mass = 1.0f;
}

Eigen::Vector3f PhysicsObject::GetPosition()
{
	return position;
}
void PhysicsObject::SetPosition(Eigen::Vector3f newPos)
{
	position = newPos;
}

Eigen::Vector3f PhysicsObject::GetVelocity()
{
	return velocity;
}

void PhysicsObject::SetVelocity(Eigen::Vector3f newVel)
{
	velocity = newVel;
}

float PhysicsObject::GetAngle()
{
	return angle;
}

void PhysicsObject::SetAngle(float newAngle)
{
	angle = newAngle;
}

float PhysicsObject::GetMass()
{
	return mass;
}

void PhysicsObject::SetMass(float newMass)
{
	mass = newMass;
}