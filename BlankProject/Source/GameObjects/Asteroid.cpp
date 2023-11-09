#include "BlankProject.h"
#include "Asteroid.h"

#include "../Scripts/Asteroids/AsteroidLife.h"
#include "Engine/Resource.h"
#include "Scripts/TriggerCollisionEvent/AsteroidCollisionEvent.h"

Asteroid::Asteroid()
{
	m_Name = "Asteroid";

	auto rb = AddComponent<Rigidbody>();
	rb->SetMass(100.f);
	rb->SetBodyType(BodyType::Dynamic);

	auto mesh = AddComponent<MeshRenderer>();
	mesh->Init(SPHERE, TEXTURE);
	mesh->RegisterTexture(Resource::Load<Texture>("Resources/Textures/mars.dds"));

	//auto particle = AddComponent<ParticleRenderer>();
	//particle->Init(CUBE, PARTICLE);

	transform->SetScale(2.f);

	auto sc = AddComponent<SphereCollider>();
	sc->SetRadius(2.f);
	//sc->SetCenter({ -4.f, -4.f, -4.f });

	AddComponent<AsteroidLife>();
	AddComponent<AsteroidCollisionEvent>();

	m_CategoryBitmask.SetLayer(LayerID::ASTEROID);
	m_CollisionBitmask.SetLayer(LayerID::PLAYER);
	m_CollisionBitmask.AddLayer(LayerID::ASTEROID);
	m_CollisionBitmask.AddLayer(LayerID::PROJECTILE);
	m_CollisionBitmask.AddLayer(LayerID::OBSTACLE);


	mesh = nullptr;
}

Asteroid::~Asteroid()
{
}