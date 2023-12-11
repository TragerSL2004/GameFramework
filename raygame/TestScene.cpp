#include "TestScene.h"
#include "Player.h"
#include "Transform2D.h"

void TestScene::start()
{
	// C# - base.Start()
	Scene::start();

	Player* player = new Player();
	MathLibrary::Vector2 scale = MathLibrary::Vector2(50, 50);
	player->getTransform()->setScale(scale);
	addActor(player);
}
