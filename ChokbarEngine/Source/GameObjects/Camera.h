#pragma once
#include "Engine/ECS/Components/CameraComponent.h"
#include "Engine/ECS/Base/GameObject.h"
#include "Engine/ECS/Components/PlayerComponent.h"


class Camera : public Chokbar::GameObject
{
public:

	Camera(const std::string& name);
	~Camera();

	CameraComponent* GetCameraComponent() const { return m_CameraComponent; }

private:

	CameraComponent* m_CameraComponent;
	PlayerComponent* m_PlayerComponent;

};
