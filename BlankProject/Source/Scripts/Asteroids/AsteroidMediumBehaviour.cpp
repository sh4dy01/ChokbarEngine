#include "AsteroidMediumBehaviour.h"


void AsteroidMediumBehaviour::Awake()
{

    srand(time(NULL));

    m_Speed = 5.0f;
    m_PlayerTransform = nullptr;
    m_Lifetime = 5.0f;

    m_TeleportationAngle = 66.6f;
    m_TeleportationDistance = 33.0f;
    m_TeleportInterval = 0.5f;  

    m_TimeSinceLastTeleport = 0.0f;
}

void AsteroidMediumBehaviour::Start()
{
}

void AsteroidMediumBehaviour::Initialize(Transform* target, float speed)
{
	m_PlayerTransform = target;
	m_Speed = speed;

}

void AsteroidMediumBehaviour::Update()
{
    m_TimeSinceLastTeleport += TimeManager::GetDeltaTime();

    XMFLOAT3 asteroidPos = transform->GetPosition();
    XMFLOAT3 playerPos = m_PlayerTransform->GetPosition();

    XMVECTOR asteroidVecPos = XMLoadFloat3(&asteroidPos);
    XMVECTOR playerVecPos = XMLoadFloat3(&playerPos);

    XMVECTOR directionToPlayer = XMVectorSubtract(playerVecPos, asteroidVecPos);
    directionToPlayer = XMVector3Normalize(directionToPlayer);

    if (m_TimeSinceLastTeleport >= m_TeleportInterval) {
        m_TimeSinceLastTeleport = 0.0f;

        // Cr�ez une direction al�atoire avec une composante Z toujours positive
        XMVECTOR randomDir = XMVectorSet(
            ((float)rand() / (float)RAND_MAX) * 20.0f, // X
            ((float)rand() / (float)RAND_MAX) * 20.0f, // Y
            ((float)rand() / (float)RAND_MAX) * 20.0f, // Z toujours positif
            0.0f
        );
        randomDir = XMVector3Normalize(randomDir);

        // Assurez-vous que le vecteur al�atoire pointe globalement dans la m�me direction que la direction vers le joueur
        if (XMVectorGetX(XMVector3Dot(randomDir, directionToPlayer)) < 0.0f) {
            randomDir = XMVectorNegate(randomDir);
        }

        // Calculez le d�placement al�atoire avec un l�ger d�calage vers l'avant
        XMVECTOR teleportOffset = XMVectorScale(randomDir, m_TeleportationDistance);
        XMVECTOR teleportPosition = XMVectorAdd(asteroidVecPos, teleportOffset);

        XMFLOAT3 teleportPos;
        XMStoreFloat3(&teleportPos, teleportPosition);
        transform->SetPosition(teleportPos);

        // R�initialisez asteroidVecPos pour �tre la nouvelle position de d�part pour le d�placement apr�s la t�l�portation
        asteroidVecPos = teleportPosition;
    }

    // D�placez l'ast�ro�de vers le joueur
    XMVECTOR newPosition = XMVectorAdd(asteroidVecPos, XMVectorScale(directionToPlayer, m_Speed * TimeManager::GetDeltaTime()));

    XMFLOAT3 finalPosition;
    XMStoreFloat3(&finalPosition, newPosition);
    transform->SetPosition(finalPosition);

    DestroyAfterATime();
}

void AsteroidMediumBehaviour::DestroyAfterATime()
{
    this->m_Lifetime -= TimeManager::GetDeltaTime();
    if (m_Lifetime <= 0)
    {
        gameObject->Destroy();
    }
}