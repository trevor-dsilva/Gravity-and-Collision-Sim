#pragma once
#include<Eigen/Dense>

class PhysicsObject
{
public:
	PhysicsObject();
	PhysicsObject(Eigen::Vector3f pos, Eigen::Vector3f vel, float mass);

	Eigen::Vector3f GetPosition();
	void SetPosition(Eigen::Vector3f newPos);

	Eigen::Vector3f GetVelocity();
	void SetVelocity(Eigen::Vector3f newVel);

	float GetAngle();
	void SetAngle(float newAngle);

	float GetMass();
	void SetMass(float newMass);

private:
	Eigen::Vector3f position;
	Eigen::Vector3f velocity;
	float angle;
	float mass;
};