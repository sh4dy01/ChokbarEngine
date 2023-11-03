#include "Chokbar.h"
#include "PhysicsWorld.h"

using namespace DirectX;

CollisionInfo::CollisionInfo(Collider* colliderA, Collider* colliderB)
	: m_ColliderA(colliderA), m_ColliderB(colliderB), m_State(Enter)
{
}

CollisionInfo::~CollisionInfo()
{
	m_ColliderA = nullptr;
	m_ColliderB = nullptr;
}

void CollisionInfo::UpdateState(CollisionState newState)
{
	m_State = newState;
}

PhysicsWorld::PhysicsWorld()
	: UPDATE_RATE(0.02f), m_CurrentCollisionInfo(nullptr), m_gridSize(0), m_cellSize(0.0f), m_timer(0.0f)
{

}

PhysicsWorld::~PhysicsWorld()
{
	for (auto& rigidbody : m_rigidbodies)
	{
		rigidbody = nullptr;
	}

	for (auto& collisionInfo : m_RegisteredCollisionInfos)
	{
		DELPTR(collisionInfo);
	}

	m_rigidbodies.clear();
}

void PhysicsWorld::Update(float dt)
{
	m_timer += dt;

	if (m_timer >= UPDATE_RATE)
	{
		Engine::GetCoordinator()->FixedUpdateComponents();

		CheckCollision();

		m_timer = 0.0f;
	}
}

void PhysicsWorld::RegisterRigidBody(Rigidbody* rigidbody)
{
	m_rigidbodies.push_back(rigidbody);
}

void PhysicsWorld::RemoveRigidBody(Rigidbody* rigidbody)
{
	if (m_rigidbodies.empty()) return;

	std::erase(m_rigidbodies, rigidbody);
}

void PhysicsWorld::CheckCollision()
{
	if (m_rigidbodies.size() < 2) return;

	for (size_t i = 0; i < m_rigidbodies.size(); i++)
	{
		for (size_t j = 0; j < m_rigidbodies.size(); j++)
		{
			if (i == j) continue;

			if (CheckCollisionShapes(m_rigidbodies[i], m_rigidbodies[j]))
			{
				switch (m_CurrentCollisionInfo->GetState())
				{
				case Enter:

					m_CurrentCollisionInfo->GetColliderA()->CallOnTriggerEnter(m_CurrentCollisionInfo->GetColliderB());
					m_CurrentCollisionInfo->GetColliderB()->CallOnTriggerEnter(m_CurrentCollisionInfo->GetColliderA());

					DEBUG_LOG(m_rigidbodies[i]->gameObject->GetName() << " entered in collision with " << m_rigidbodies[j]->gameObject->GetName())

					break;
				case Stay:

					//m_rigidbodies[i]->CallOnCollisionStay(m_CurrentCollisionInfo.ColliderB);
					//m_rigidbodies[j]->CallOnCollisionStay(m_CurrentCollisionInfo.ColliderA);

					//DEBUG_LOG(m_rigidbodies[i]->gameObject->GetName() << " continue colliding with " << m_rigidbodies[j]->gameObject->GetName())

					break;
				case Exit:

					//m_rigidbodies[i]->CallOnCollisionExit(m_CurrentCollisionInfo.ColliderB);
					//m_rigidbodies[j]->CallOnCollisionExit(m_CurrentCollisionInfo.ColliderA);

					DEBUG_LOG(m_rigidbodies[i]->gameObject->GetName() << " exited collision with " << m_rigidbodies[j]->gameObject->GetName())
					std::erase(m_RegisteredCollisionInfos, m_CurrentCollisionInfo);

					break;
				}
					
				//m_rigidbodies[i]->SetVelocity(XMFLOAT3(0, 0, 0));

				//DEBUG_LOG(m_rigidbodies[i]->gameObject->GetName() << " collided with " << m_rigidbodies[j]->gameObject->GetName());
			}
		}
	}
}

bool PhysicsWorld::CheckCollisionShapes(Rigidbody* rbA, Rigidbody* rbB)
{
	for (const auto& shapeA : rbA->GetAllCollisionShape())
	{
		for (const auto& shapeB : rbB->GetAllCollisionShape())
		{
			if (shapeA->GetType() == Collider::ShapeType::Sphere && shapeB->GetType() == Collider::ShapeType::Sphere)
			{
				const auto sphereA = dynamic_cast<SphereCollider*>(shapeA);
				const auto sphereB = dynamic_cast<SphereCollider*>(shapeB);

				if (AreSpheresColliding(sphereA, sphereB))
				{
					HandleCollision(sphereA, sphereB);

					return true;
				}
				// If there is a collision already registered
				else if (!m_RegisteredCollisionInfos.empty())
				{
					// We check if the collision is still occuring and set it to exit
					if (const auto collisionInfo = GetCollisionInfo(sphereA, sphereB))
					{
						collisionInfo->UpdateState(Exit);
						m_CurrentCollisionInfo = collisionInfo;

						// return true to handle the exit trigger
						return true;
					}
				}	

				return false;
			}

			//          else if (shapeA->GetType() == CollisionShape::ShapeType::Sphere && shapeB->GetType() == CollisionShape::ShapeType::Box)
			//          {
				  //		// sphere-box collision
				  //		// TODO
				  //	}
			//          else if (shapeA->GetType() == CollisionShape::ShapeType::Box && shapeB->GetType() == CollisionShape::ShapeType::Sphere)
			//          {
				  //		// box-sphere collision
				  //		// TODO
				  //	}
			//          else if (shapeA->GetType() == CollisionShape::ShapeType::Box && shapeB->GetType() == CollisionShape::ShapeType::Box)
			//        {
				  //		// box-box collision
				  //		// TODO
				  //	}
				  //}

		}
	}

	return false;
}

void PhysicsWorld::HandleCollision(Collider* const sphereA, Collider* const sphereB)
{
	const auto collisionInfo = GetCollisionInfo(sphereA, sphereB);

	if (!collisionInfo)
	{
		CreateNewCollisionInfo(sphereA, sphereB);
	}
	else
	{
		collisionInfo->UpdateState(Stay);
		m_CurrentCollisionInfo = collisionInfo;
	}
}

void PhysicsWorld::CreateNewCollisionInfo(Collider* const sphereA, Collider* const sphereB)
{
	const auto newCollisionInfo = new CollisionInfo(sphereA, sphereB);

	m_RegisteredCollisionInfos.push_back(newCollisionInfo);
	m_CurrentCollisionInfo = newCollisionInfo;
}

CollisionInfo* PhysicsWorld::GetCollisionInfo(const Collider* sphereA, const Collider* sphereB) const
{
	for (const auto& collisionInfo : m_RegisteredCollisionInfos)
	{
		if (collisionInfo->GetColliderA() == sphereA && collisionInfo->GetColliderB() == sphereB)
		{
			return collisionInfo;
		}
	}

	return nullptr;
}

bool PhysicsWorld::AreSpheresColliding(SphereCollider* sphereA, SphereCollider* sphereB) const
{
	// Load A.
	XMVECTOR vCenterA = DirectX::XMVectorAdd(XMLoadFloat3(&sphereA->GetCenter()), XMLoadFloat3(&sphereA->transform->GetPosition()));
	XMVECTOR vRadiusA = DirectX::XMVectorReplicate(sphereA->GetRadius());

	// Load B.
	XMVECTOR vCenterB = DirectX::XMVectorAdd(XMLoadFloat3(&sphereB->GetCenter()), XMLoadFloat3(&sphereB->transform->GetPosition()));
	XMVECTOR vRadiusB = DirectX::XMVectorReplicate(sphereB->GetRadius());

	// Distance squared between centers.    
	XMVECTOR Delta = DirectX::XMVectorSubtract(vCenterB, vCenterA);
	XMVECTOR DistanceSquared = DirectX::XMVector3LengthSq(Delta);

	// Sum of the radii squared.
	XMVECTOR RadiusSquared = DirectX::XMVectorAdd(vRadiusA, vRadiusB);
	RadiusSquared = DirectX::XMVectorMultiply(RadiusSquared, RadiusSquared);

	return DirectX::XMVector3LessOrEqual(DistanceSquared, RadiusSquared);
}
