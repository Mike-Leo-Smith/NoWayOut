//
// Created by Mike Smith on 2019/12/31.
//

#include "game_logic.h"
#include <random>
#include <algorithm>

void GameLogic::init() {
    _state.frame = 0;
    
    btBroadphaseInterface *broadphase = new btDbvtBroadphase();
    btDefaultCollisionConfiguration *collisionConfiguration = new btDefaultCollisionConfiguration();
    btCollisionDispatcher *dispatcher = new btCollisionDispatcher(collisionConfiguration);
    btSequentialImpulseConstraintSolver *solver = new btSequentialImpulseConstraintSolver;
    world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
    //world->setGravity(btVector3(0, -10, 0));
    world->setGravity(btVector3(0, 0, 0));

    
    btCollisionShape *floorShape = new btStaticPlaneShape(btVector3(0, 1, 0), 0);
    btDefaultMotionState *floorMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
    btRigidBody::btRigidBodyConstructionInfo floorRigidBodyCI(0, floorMotionState, floorShape, btVector3(0, 0, 0));
    btRigidBody *floorRigidBody = new btRigidBody(floorRigidBodyCI);
	floorRigidBody->setUserPointer(new ground());
    world->addRigidBody(floorRigidBody);
    
    btCollisionShape *hand_foot_shape = new btConeShape(0.05f, 0.2f);
    btCollisionShape *arm_leg_shape = new btCylinderShape(btVector3(0.05f, 0.4f, 0.05f));
    btCollisionShape *body_shape = new btCylinderShape(btVector3(0.4f, 0.8f, 0.4f));
    btCollisionShape *head_shape = new btSphereShape(0.25f);
    
    organGeometries.resize(6);

    organGeometries[PLAYER_ARM] = Geometry::create("data/meshes/primitives/cylinder.obj");
    organGeometries[PLAYER_LEG] = Geometry::create("data/meshes/primitives/cylinder.obj");
    organGeometries[PLAYER_HAND] = Geometry::create("data/meshes/primitives/cylinder.obj");
    organGeometries[PLAYER_FOOT] = Geometry::create("data/meshes/primitives/cylinder.obj");
    organGeometries[PLAYER_BODY] = Geometry::create("data/meshes/primitives/cylinder.obj");
    organGeometries[PLAYER_HEAD] = Geometry::create("data/meshes/primitives/cylinder.obj");
    
    auto rotation = glm::rotate(glm::mat4{1.0f}, glm::radians(90.0f), glm::vec3{1.0f, 0.0f, 0.0f});
    //enemyBook.push_back(enemy_book_elem(0, 100, 1.5, true, Geometry::create("data/meshes/flying_horse/flying_horse.obj")));
    //enemyBook.push_back(enemy_book_elem(1, 100, 1.5, true, Geometry::create("data/meshes/airplane/airplane.obj")));
	enemyBook.push_back(enemy_book_elem(0, 100, 0.5, true, Geometry::create("data/meshes/flying_horse/flying_horse.obj"), bullet_info(0.1, 0.05, 15, 40)));

	bulletGeometry = Geometry::create("data/meshes/primitives/sphere.obj", glm::scale(glm::mat4{1.0f}, glm::vec3{0.05f, 0.05f, 0.05f}));
    
    organ_type_t organ_types[14]
        {PLAYER_ARM, PLAYER_ARM, PLAYER_ARM, PLAYER_ARM, PLAYER_LEG, PLAYER_LEG, PLAYER_LEG, PLAYER_LEG, PLAYER_FOOT, PLAYER_FOOT, PLAYER_BODY,
         PLAYER_HEAD};
    
    for (int i = 0; i < 14; i++) {
        btCollisionShape *shape;
        organ_type_t organ_type = organ_types[i];
        switch (organ_type) {
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
        btDefaultMotionState *organMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
        
        if (organ_type == PLAYER_HEAD) {
            organMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 2, 3)));
        }
        
        btScalar mass = 5;
        btVector3 inertia(0, 0, 0); //todo
        shape->calculateLocalInertia(mass, inertia);
        btRigidBody::btRigidBodyConstructionInfo organRigidBodyCI(mass, organMotionState, shape, inertia);
        btRigidBody *organRigidBody = new btRigidBody(organRigidBodyCI);
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
	
	//btCollisionShape* enemyShape = new btSphereShape(0.5);

	std::vector<uint32_t> index_buffer = enemyInfo->geometry->index_buffer();
	auto& pos_buffer = enemyInfo->geometry->position_buffer();

	std::cout << "generate enemy: triangle num is " << index_buffer.size() / 3 << " point num is " << pos_buffer.size() << "flying is " << flying << "\n";

	//auto indexVertexArrays = new btTriangleIndexVertexArray(index_buffer.size() / 3, reinterpret_cast<int *>(index_buffer.data()),
	//														3 * sizeof(int), pos_buffer.size(), reinterpret_cast<float*>(pos_buffer.data()), sizeof(float) * 3);
	//btGImpactMeshShape* enemyShape = new btGImpactMeshShape(indexVertexArrays);

	btConvexHullShape* enemyShape = new btConvexHullShape(reinterpret_cast<float*>(pos_buffer.data()), pos_buffer.size(), sizeof(float) * 3);

	btCollisionDispatcher * dispatcher = static_cast<btCollisionDispatcher *>(world->getDispatcher());
	btGImpactCollisionAlgorithm::registerAlgorithm(dispatcher);

	btDefaultMotionState* enemyMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(x, y, z)));
	btScalar mass = 5;
	btVector3 inertia(0, 0, 0);
	enemyShape->calculateLocalInertia(mass, inertia);
	btRigidBody::btRigidBodyConstructionInfo enemyRigidBodyCI(mass, enemyMotionState, enemyShape, inertia);
	btRigidBody* enemyRigidBody = new btRigidBody(enemyRigidBodyCI);
	//enemyRigidBody->setCollisionFlags(enemyRigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
	//enemyRigidBody->setActivationState(DISABLE_DEACTIVATION);

	_state.enemies.push_back(new enemy(enemyInfo->geometry.get(), enemyRigidBody, enemyInfo->maxHealth, flying, enemyInfo->speed, enemyInfo->shooterInfo));

	enemyRigidBody->setUserPointer(_state.enemies.back());
	world->addRigidBody(enemyRigidBody);
}

void GameLogic::organCollideEnemy(organ *organA, enemy *enemyB, float impulse) {
    enemyB->health -= impulse;
    if (organA->organ_type == organ_type_t::PLAYER_BODY) {
        _state.playerHealth -= impulse;
    } else if (organA->organ_type == organ_type_t::PLAYER_HEAD) {
        _state.playerHealth -= impulse * 1.5;
    } else {
        _state.playerHealth -= impulse * 0.3;
    }
    
    if (_state.playerHealth < 0) { //todo
        std::cout << "You are dead\n";
    }
}

void GameLogic::enemyCollideBullet(enemy *enemyA, bullet *bulletB, float impulse) {
	if(bulletB->isPlayers)
	{
		bulletDrop(bulletB);
		enemyA->health -= impulse;
	}
}

void GameLogic::organCollideBullet(organ *organA, bullet *bulletB, float impulse) {
	if(!bulletB->isPlayers)
	{
		bulletDrop(bulletB);
		if(organA->organ_type == organ_type_t::PLAYER_BODY) {
			_state.playerHealth -= impulse;
		}
		else if(organA->organ_type == organ_type_t::PLAYER_HEAD) {
			_state.playerHealth -= impulse * 1.5;
		}
		else {
			_state.playerHealth -= impulse * 0.3;
		}

		if(_state.playerHealth < 0) { //todo
			std::cout << "You are dead\n";
		}
	}
}

void GameLogic::enemyDrop(enemy* enemyA)
{
	if(enemyA->health <= 0)
		enemyA->shouldRemove = true;
}

void GameLogic::bulletDrop(bullet* bulletA)
{
	bulletA->shouldRemove = true;
}

void GameLogic::collide(unit *unitA, unit *unitB, float impulse) {
	if(unitA->getType() == unit::unit_type_t::GROUND && unitB->getType() == unit::unit_type_t::ENEMY) {
		enemyDrop((enemy *)unitB);
	}
	else if(unitA->getType() == unit::unit_type_t::GROUND && unitB->getType() == unit::unit_type_t::BULLET) {
		bulletDrop((bullet *)unitB);
	}
    else if (unitA->getType() == unit::unit_type_t::ORGAN && unitB->getType() == unit::unit_type_t::ENEMY) {
        organCollideEnemy((organ *)unitA, (enemy *)unitB, impulse);
    } else if (unitA->getType() == unit::unit_type_t::ENEMY && unitB->getType() == unit::unit_type_t::BULLET) {
        enemyCollideBullet((enemy *)unitA, (bullet *)unitB, impulse);
    } else if (unitA->getType() == unit::unit_type_t::ORGAN && unitB->getType() == unit::unit_type_t::BULLET) {
        organCollideBullet((organ *)unitA, (bullet *)unitB, impulse);
    }
}

void GameLogic::applyForce()
{
	auto head_trans = _state.organs[11].obj->getWorldTransform();
	auto head_origin = head_trans.getOrigin();
	for(auto& b : _state.bullets)
	{
		b->setGravity();
	}
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
				e->setGravity();
			}
		}
	}
}

void GameLogic::generateBullet()
{
	auto head_trans = _state.organs[13].obj->getWorldTransform();
	auto head_origin = head_trans.getOrigin();

	for(auto e : _state.enemies)
	{

		if(e->shooterInfo.interval <= 0)
			continue;

		e->lastShot++;
		if(e->lastShot == e->shooterInfo.interval)
		{
			e->lastShot = 0;

			auto enemyPos = e->obj->getWorldTransform().getOrigin();

			//btGImpactMeshShape* enemyShape = new btGImpactMeshShape(indexVertexArrays);

			btCollisionShape *bulletShape = new btSphereShape(e->shooterInfo.radius);

			btCollisionDispatcher * dispatcher = static_cast<btCollisionDispatcher *>(world->getDispatcher());
			btGImpactCollisionAlgorithm::registerAlgorithm(dispatcher);

			btDefaultMotionState* bulletMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), enemyPos));
			btScalar mass = e->shooterInfo.mass;
			btVector3 inertia(0, 0, 0);
			bulletShape->calculateLocalInertia(mass, inertia);
			btRigidBody::btRigidBodyConstructionInfo bulletRigidBodyCI(mass, bulletMotionState, bulletShape, inertia);
			btRigidBody* bulletRigidBody = new btRigidBody(bulletRigidBodyCI);
			bulletRigidBody->setCollisionFlags(bulletRigidBody->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
			//enemyRigidBody->setActivationState(DISABLE_DEACTIVATION);

			_state.bullets.push_back(new bullet(bulletGeometry.get(), bulletRigidBody, false));

			bulletRigidBody->setUserPointer(_state.bullets.back());
			world->addRigidBody(bulletRigidBody);

			auto direction = head_origin - enemyPos;
			direction.normalize();
			float impulse = mass * e->speed;
			direction *= impulse;
			bulletRigidBody->applyCentralImpulse(direction);
		}
	}
}

void GameLogic::update(const DisplayState &display_state, const GestureState &gesture_state) { 
	_state.frame++;

	if(_state.frame == 60 || _state.frame == 120 || _state.frame == 180)//if(_state.frame % 60 == 0)
		generateEnemy();

	generateBullet();
	applyForce();

	world->stepSimulation(1 / 60.f, 10); //todo: change fps

	/*for(auto e : _state.enemies)
	{
		auto origin = e->obj->getWorldTransform().getOrigin();
		std::cout << "enemy: " << origin.x() << " " << origin.y() << " " << origin.z() << "\n";
	}*/

	std::cout << _state.bullets.size() << "\n";

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
				std::cout << "Collide! Impulse is " << pt.getAppliedImpulse() << std::endl;
				collide(unitA, unitB, pt.getAppliedImpulse());
			}
		}
	}
	
	auto remove_elem = [&](auto &vec) {
		auto first_remove = std::remove_if(vec.begin(), vec.end(), [](auto e) { return e->shouldRemove; });
		for(auto it = first_remove; it != vec.end(); it++)
			world->removeRigidBody((*it)->obj);
		vec.erase(first_remove, vec.end());
	};

	remove_elem(_state.enemies);
	remove_elem(_state.bullets);


//	auto update_object_transform = [](unit &object) {
//	    glm::mat4 m;
//	    object.obj->getWorldTransform().getOpenGLMatrix(glm::value_ptr(m));
//	    object.geometry->set_transform(m);
//	};
//
//	for (auto &&organ : _state.organs) {
//	    update_object_transform(organ);
//	}
//	for (auto &&enemy : _state.enemies) {
//	    update_object_transform(*enemy);
//	}
//	for (auto &&bullet : _state.bullets) {
//	    update_object_transform(*bullet);
//	}
//
	
}
