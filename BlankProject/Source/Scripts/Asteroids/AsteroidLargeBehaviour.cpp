#include "AsteroidLargeBehaviour.h"


void AsteroidLargeBehaviour::Awake() 
{

	m_Speed = 1.0f;
	m_Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_Lifetime = 5.0f;
}

void AsteroidLargeBehaviour::Start() 
{

}

void AsteroidLargeBehaviour::Initialize(const XMFLOAT3 direction, float speed)
{
	m_Direction = direction;
	m_Speed = speed;
}

void AsteroidLargeBehaviour::Update()
{
    transform->Translate(XMVectorScale(XMLoadFloat3(&m_Direction), m_Speed * TimeManager::GetDeltaTime()));

    float deltaTime = TimeManager::GetDeltaTime();
    float rotationSpeedX = 100.0f * deltaTime;

    transform->Rotate(rotationSpeedX, 0, 0);

    DestroyAfterATime();
}

void AsteroidLargeBehaviour::DestroyAfterATime()
{
    this->m_Lifetime -= TimeManager::GetDeltaTime();
    if (m_Lifetime <= 0)
    {
        gameObject->Destroy();
    }
}