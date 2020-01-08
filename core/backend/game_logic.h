//
// Created by Mike Smith on 2019/12/31.
//

#pragma once

#include "frame_render.h"
#include "gesture_capture.h"
#include <btBulletDynamicsCommon.h>
#include "BulletCollision/Gimpact/btGImpactShape.h"
#include "BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h"
#include <vector>

struct unit
{
	enum unit_type_t {ORGAN, ENEMY, BULLET};
	[[nodiscard]] virtual unit_type_t getType() const = 0;
	btRigidBody* obj{nullptr};
	Geometry* geometry;
};

enum organ_type_t { PLAYER_ARM, PLAYER_LEG, PLAYER_HAND, PLAYER_FOOT, PLAYER_BODY, PLAYER_HEAD };

struct organ : unit
{
	organ_type_t organ_type;
	[[nodiscard]] unit_type_t getType() const override { return ORGAN; }
	organ() noexcept = default;
	organ(Geometry* g, btRigidBody* o, organ_type_t t) : organ_type(t)
	{ 
		geometry = g;
		obj = o;
	}
};

struct enemy : unit
{
	int health;
	int maxHealth;
	bool isFlying;
	float speed;
	btVector3 lastForce;
	[[nodiscard]] unit_type_t getType() const override { return ENEMY; }
	enemy(Geometry* g, btRigidBody* o, int h, bool f, float s) : health(h), maxHealth(h), isFlying(f), speed(s), lastForce(btVector3(0, 0, 0))
	{ 
		geometry = g;
		obj = o;
	}
	void updateForce(btVector3 direction, float length)
	{
		direction.normalize();
		direction *= length;
		obj->applyCentralForce(direction - lastForce);
		lastForce = direction;
	}
};

struct enemy_book_elem
{
	int id;
	int maxHealth;
	float speed;
	bool isFlying;
	std::unique_ptr<Geometry> geometry;
	enemy_book_elem(int _id, int _maxHealth, float _speed, int _isFlying, std::unique_ptr<Geometry> _geometry) : id(_id), maxHealth(_maxHealth), speed(_speed), isFlying(_isFlying), geometry(std::move(_geometry)) {}
};

struct bullet : unit
{
	int damage;
	bool isPlayers;
	[[nodiscard]] unit_type_t getType() const override { return BULLET; }
	bullet(Geometry* g, btRigidBody* o, int d, bool p) : damage(d), isPlayers(p)
	{ 
		geometry = g; 
		obj = o;
	}
};

struct GameState {
	organ organs[14];
	std::vector<enemy*> enemies;
	std::vector<bullet*> bullets;
	int frame;

	int playerHealth;
	int playerMaxHealth;
};

class GameLogic {

private:
    GameState _state;
	btDiscreteDynamicsWorld* world{};
	
	std::vector<std::unique_ptr<Geometry>> organGeometries;
	std::vector<enemy_book_elem> enemyBook;
	

	void organCollideEnemy(organ* organA, enemy* enemyB, float impulse);
	void enemyCollideBullet(enemy* enemyA, bullet* bulletB, float impulse);
	void organCollideBullet(organ* organA, bullet* bulletB, float impulse);
	void collide(unit* unitA, unit* unitB, float impulse);

public:
	void init();
	void generateEnemy();
    void update(const DisplayState &display_state, const GestureState &gesture_state);
    [[nodiscard]] const GameState &state() const noexcept { return _state; }
    
    [[nodiscard]] bool should_exit() const noexcept { return false; /* todo */ }
    
    template<typename ...Args>
    [[nodiscard]] static std::unique_ptr<GameLogic> create(Args &&...args) noexcept {
        auto instance = std::make_unique<GameLogic>(std::forward<Args>(args)...);
        instance->init();
        return instance; 
    }
};
