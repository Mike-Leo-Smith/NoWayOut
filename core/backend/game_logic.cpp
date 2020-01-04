//
// Created by Mike Smith on 2019/12/31.
//

#include "game_logic.h"
#include <random>

void GameLogic::init() {
	_state.frame = 0;

	btBroadphaseInterface* broadphase = new btDbvtBroadphase();
	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;
	world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
	world->setGravity(btVector3(0, -10, 0));
	
	btCollisionShape* floorShape = new btStaticPlaneShape(btVector3(0, 1, 0), 0);
	btDefaultMotionState* floorMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0)));
	btRigidBody::btRigidBodyConstructionInfo floorRigidBodyCI(0, floorMotionState, floorShape, btVector3(0, 0, 0));
	btRigidBody* floorRigidBody = new btRigidBody(floorRigidBodyCI);
	world->addRigidBody(floorRigidBody);

	btCollisionShape* hand_foot = new btConeShape(0.05f, 0.2f);
	btCollisionShape* arm_leg = new btCylinderShape(btVector3(0.05f, 0.4f, 0.05f));
	btCollisionShape* body = new btCylinderShape(btVector3(0.4f, 0.8f, 0.4f));
	btCollisionShape* head = new btSphereShape(0.25f);

	for(int i = 0; i < 14; i++)
	{
		btCollisionShape* shape;
		switch(_state.organs[i].organ_type)
		{
		case PLAYER_ARM:
		case PLAYER_LEG:
			shape = arm_leg;
			break;
		case PLAYER_FOOT:
		case PLAYER_HAND:
			shape = hand_foot;
			break;
		case PLAYER_BODY:
			shape = body;
			break;
		case PLAYER_HEAD:
			shape = head;
			break;
		}
		btDefaultMotionState* organMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
		btScalar mass = 5;
		btVector3 inertia(0, 0, 0); //todo
		shape->calculateLocalInertia(mass, inertia);
		btRigidBody::btRigidBodyConstructionInfo organRigidBodyCI(mass, organMotionState, shape, inertia);
		btRigidBody* organRigidBody = new btRigidBody(organRigidBodyCI);
		organRigidBody->setCollisionFlags(organRigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
		organRigidBody->setActivationState(DISABLE_DEACTIVATION);

		organRigidBody->setUserPointer(&(_state.organs[i]));
		world->addRigidBody(organRigidBody);

		_state.organs[i].obj = organRigidBody;
	}

}

void GameLogic::generateEnemy()
{
	const float distance = 10;
	std::uniform_real_distribution<> values{0.0, 3.1415926 * 2};
	std::random_device rd;
	std::default_random_engine rng{rd()};
	float theta = values(rng);
	float x = distance * std::cosf(theta);
	float y = 0; //todo
	float z = distance * std::sinf(theta);
	
	btCollisionShape* enemyShape = new btSphereShape(0.5);
	btDefaultMotionState* enemyMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(x, y, z)));
	btScalar mass = 5;
	btVector3 inertia(0, 0, 0);
	enemyShape->calculateLocalInertia(mass, inertia);
	btRigidBody::btRigidBodyConstructionInfo enemyRigidBodyCI(mass, enemyMotionState, enemyShape, inertia);
	btRigidBody* enemyRigidBody = new btRigidBody(enemyRigidBodyCI);
	enemyRigidBody->setCollisionFlags(enemyRigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
	enemyRigidBody->setActivationState(DISABLE_DEACTIVATION);

	_state.enemies.push_back(new enemy(0, 100));

	enemyRigidBody->setUserPointer(_state.enemies.back());
	world->addRigidBody(enemyRigidBody);
	_state.enemies.back()->obj = enemyRigidBody;
}

void GameLogic::update(const DisplayState &display_state, const GestureState &gesture_state) {
	_state.frame++;

	if(_state.frame % 60 == 0)
		generateEnemy();

	world->stepSimulation(1 / 60.f, 10); //todo: change fps

	int numManifolds = world->getDispatcher()->getNumManifolds();
	for(int i = 0; i < numManifolds; ++i)  // pairs
	{
		btPersistentManifold* contactManifold = world->getDispatcher()->getManifoldByIndexInternal(i);
		const btCollisionObject* bA = static_cast<const btCollisionObject*>(contactManifold->getBody0());
		const btCollisionObject* bB = static_cast<const btCollisionObject*>(contactManifold->getBody1());
		int numContacts = contactManifold->getNumContacts();
		for(int j = 0; j < numContacts; j++)
		{
			btManifoldPoint& pt = contactManifold->getContactPoint(j);
			if(pt.getDistance() <= 0.f)
			{
				btVector3 posA = pt.getPositionWorldOnA();
				btVector3 posB = pt.getPositionWorldOnB();
				printf("%d A -> {%f, %f, %f}\n", i, posA.getX(), posA.getY(), posA.getZ());
				printf("%d B -> {%f, %f, %f}\n", i, posB.getX(), posB.getY(), posB.getZ());
				std::cout << pt.getAppliedImpulse() << std::endl;
			}
		}
	}
}
