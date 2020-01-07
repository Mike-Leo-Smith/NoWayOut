//
// Created by Mike Smith on 2019/12/31.
//

#include "game_logic.h"
#include <random>
#include <algorithm>

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

	btCollisionShape* hand_foot_shape = new btConeShape(0.05f, 0.2f);
	btCollisionShape* arm_leg_shape = new btCylinderShape(btVector3(0.05f, 0.4f, 0.05f));
	btCollisionShape* body_shape = new btCylinderShape(btVector3(0.4f, 0.8f, 0.4f));
	btCollisionShape* head_shape = new btSphereShape(0.25f);

	organGeometries.reserve(6);
	organGeometries[PLAYER_ARM] = Geometry::create("arm.obj");
	organGeometries[PLAYER_LEG] = Geometry::create("leg.obj");
	organGeometries[PLAYER_HAND] = Geometry::create("hand.obj");
	organGeometries[PLAYER_FOOT] = Geometry::create("foot.obj");
	organGeometries[PLAYER_BODY] = Geometry::create("body.obj");
	organGeometries[PLAYER_HEAD] = Geometry::create("head.obj");

	enemyBook.push_back(enemy_book_elem(0, 100, 1.5, true, Geometry::create("flying_horse.obj")));
	enemyBook.push_back(enemy_book_elem(1, 100, 1.5, true, Geometry::create("airplane.obj")));

	organ_type_t organ_types[14]{PLAYER_ARM, PLAYER_ARM, PLAYER_ARM, PLAYER_ARM, PLAYER_LEG, PLAYER_LEG, PLAYER_LEG, PLAYER_LEG, PLAYER_HAND, PLAYER_HAND, PLAYER_FOOT, PLAYER_FOOT, PLAYER_BODY, PLAYER_HEAD};

	for(int i = 0; i < 14; i++)
	{
		btCollisionShape* shape;
		organ_type_t organ_type = organ_types[i];
		switch(organ_type)
		{
		case PLAYER_ARM:
		case PLAYER_LEG:
			shape = arm_leg_shape;
			break;
		case PLAYER_FOOT:
		case PLAYER_HAND:
			shape = hand_foot_shape;
			break;
		case PLAYER_BODY:
			shape = body_shape;
			break;
		case PLAYER_HEAD:
			shape = head_shape;
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

		_state.organs[i] = organ(organGeometries[organ_type].get(), organRigidBody, organ_type);
	}

}

void GameLogic::generateEnemy()
{
	int enemyId = std::rand() % enemyBook.size();
	auto enemyInfo = &enemyBook[enemyId];

	const float distance = 10;
	bool flying = enemyInfo->isFlying;
	std::uniform_real_distribution<> values{0.0, 3.1415926 * 2};
	std::random_device rd;
	std::default_random_engine rng{rd()};
	float theta = values(rng);
	float x = distance * std::cosf(theta);
	float y = flying ? 1.5 : 0;
	float z = distance * std::sinf(theta);
	
	btCollisionShape* enemyShape = new btSphereShape(0.5);
	btDefaultMotionState* enemyMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(x, y, z)));
	btScalar mass = 5;
	btVector3 inertia(0, 0, 0);
	enemyShape->calculateLocalInertia(mass, inertia);
	btRigidBody::btRigidBodyConstructionInfo enemyRigidBodyCI(mass, enemyMotionState, enemyShape, inertia);
	btRigidBody* enemyRigidBody = new btRigidBody(enemyRigidBodyCI);
	//enemyRigidBody->setCollisionFlags(enemyRigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
	//enemyRigidBody->setActivationState(DISABLE_DEACTIVATION);

	_state.enemies.push_back(new enemy(enemyInfo->geometry.get(), enemyRigidBody, enemyInfo->maxHealth, flying, enemyInfo->speed));

	if(flying)
	{
		enemyRigidBody->activate();
		enemyRigidBody->applyCentralForce(btVector3(0, 10, 0));
	}

	enemyRigidBody->setUserPointer(_state.enemies.back());
	world->addRigidBody(enemyRigidBody);
}

void GameLogic::organCollideEnemy(organ* organA, enemy* enemyB, float impulse)
{
	enemyB->health -= impulse;
	if(organA->organ_type == organ_type_t::PLAYER_BODY)
		_state.playerHealth -= impulse;
	else if(organA->organ_type == organ_type_t::PLAYER_HEAD)
		_state.playerHealth -= impulse * 1.5;
	else
		_state.playerHealth -= impulse * 0.3;
	
	if(_state.playerHealth < 0) //todo
		std::cout << "You are dead\n";
}

void GameLogic::enemyCollideBullet(enemy* enemyA, bullet* bulletB, float impulse)
{
	enemyA->health -= impulse;
}

void GameLogic::organCollideBullet(organ* organA, bullet* bulletB, float impulse)
{
	if(organA->organ_type == organ_type_t::PLAYER_BODY)
		_state.playerHealth -= impulse;
	else if(organA->organ_type == organ_type_t::PLAYER_HEAD)
		_state.playerHealth -= impulse * 1.5;
	else
		_state.playerHealth -= impulse * 0.3;

	if(_state.playerHealth < 0) //todo
		std::cout << "You are dead\n";
}

void GameLogic::collide(unit* unitA, unit* unitB, float impulse)
{
	if(unitA->getType() == unit::unit_type_t::ORGAN && unitB->getType() == unit::unit_type_t::ENEMY)
		organCollideEnemy((organ*)unitA, (enemy*)unitB, impulse);
	else if(unitA->getType() == unit::unit_type_t::ENEMY && unitB->getType() == unit::unit_type_t::BULLET)
		enemyCollideBullet((enemy*)unitA, (bullet*)unitB, impulse);
	else if(unitA->getType() == unit::unit_type_t::ORGAN && unitB->getType() == unit::unit_type_t::BULLET)
		organCollideBullet((organ*)unitA, (bullet*)unitB, impulse);
}

void GameLogic::update(const DisplayState &display_state, const GestureState &gesture_state) {
	_state.frame++;

	if(_state.frame % 60 == 0)
		generateEnemy();

	world->stepSimulation(1 / 60.f, 10); //todo: change fps

	auto head_trans = _state.organs[13].obj->getWorldTransform();
	auto head_origin = head_trans.getOrigin();
	for(auto& e : _state.enemies)
	{
		auto enemy_trans = e->obj->getWorldTransform();
		auto enemy_origin = enemy_trans.getOrigin();
		auto diff_origin = head_origin - enemy_origin;
		if(e->isFlying)
		{
			e->updateForce(diff_origin, e->speed);
		}
		else
		{
			if(diff_origin.length2() < 2 * 2)
			{
				e->updateForce(diff_origin, e->speed * 1.5);
			}
			else
			{
				diff_origin.setY(0);
				e->updateForce(diff_origin, e->speed);
			}
		}
	}

	int numManifolds = world->getDispatcher()->getNumManifolds();
	for(int i = 0; i < numManifolds; ++i)  // pairs
	{
		btPersistentManifold* contactManifold = world->getDispatcher()->getManifoldByIndexInternal(i);
		const btCollisionObject* bA = static_cast<const btCollisionObject*>(contactManifold->getBody0());
		const btCollisionObject* bB = static_cast<const btCollisionObject*>(contactManifold->getBody1());
		unit* unitA = (unit*)(bA->getUserPointer());
		unit* unitB = (unit*)(bB->getUserPointer());

		if(unitA->getType() == unitB->getType())
			continue;

		if(unitA->getType() > unitB->getType())
			std::swap(unitA, unitB);

		int numContacts = contactManifold->getNumContacts();
		for(int j = 0; j < numContacts; j++)
		{
			btManifoldPoint& pt = contactManifold->getContactPoint(j);
			if(pt.getDistance() <= 0.f)
			{
				std::cout << pt.getAppliedImpulse() << std::endl;
				collide(unitA, unitB, pt.getAppliedImpulse());
			}
		}
	}
}
