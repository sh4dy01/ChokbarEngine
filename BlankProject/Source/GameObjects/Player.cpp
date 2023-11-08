#include "Player.h"

#include "Engine/ECS/Components/CameraComponent.h"
#include "Scripts/Player/PlayerMovement.h"
#include "Scripts/Player/PlayerShoot.h"
#include "GameObjects/Wing.h"

Player::Player()
{
	m_Name = "Player";

	AddComponent<Rigidbody>();
	AddComponent<SphereCollider>();

	auto leftWing = GameObject::Instantiate<Wing>();
	leftWing->SetName("LeftWing");
	leftWing->transform->SetPosition(-2, 0, 0);
	//leftWing->transform->SetParent(transform);

	auto rightWing = GameObject::Instantiate<Wing>();
	rightWing->SetName("RightWing");
	rightWing->transform->SetPosition(transform->GetPosition());
	rightWing->transform->SetParent(transform);

	MeshRenderer* mesh = AddComponent<MeshRenderer>();
	mesh->Init(SPHERE, SIMPLE);

	AddComponent<PlayerMovement>();
	AddComponent<PlayerShoot>();
}
