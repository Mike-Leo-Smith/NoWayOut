//
// Created by Mike Smith on 2019/12/31.
//

#pragma once

#include "frame_render.h"
#include "gesture_capture.h"
#include <btBulletDynamicsCommon.h>
#include <vector>

struct unit
{
	enum unit_type_t {ORGAN, ENEMY, BULLET};
	virtual unit_type_t getType() = 0;
	btCollisionObject* obj;
};

enum organ_type_t { PLAYER_ARM, PLAYER_LEG, PLAYER_HAND, PLAYER_FOOT, PLAYER_BODY, PLAYER_HEAD };

struct organ : unit
{
	organ_type_t organ_type;
	unit_type_t getType() override { return ORGAN; }
	organ(organ_type_t t) : organ_type(t) {}
};

struct enemy : unit
{
	int enemyId;
	int health;
	int maxHealth;
	unit_type_t getType() override { return ENEMY; }
	enemy(int i, int h) : enemyId(i), health(h), maxHealth(h) {}
};

struct bullet : unit
{
	int damage;
	bool isPlayers;
	unit_type_t getType() override { return BULLET; }
	bullet(int d, bool p) : damage(d), isPlayers(p) {}
};

struct GameState {
	organ organs[14]{PLAYER_ARM, PLAYER_ARM, PLAYER_ARM, PLAYER_ARM, PLAYER_LEG, PLAYER_LEG, PLAYER_LEG, PLAYER_LEG, PLAYER_HAND, PLAYER_HAND, PLAYER_FOOT, PLAYER_FOOT, PLAYER_BODY, PLAYER_HEAD};
	std::vector<enemy*> enemies;
	std::vector<bullet> bullets;
	int frame;
};

class GameLogic {

private:
    GameState _state;
	btDiscreteDynamicsWorld* world;
public:
	void init();
	void generateEnemy();
    void update(const DisplayState &display_state, const GestureState &gesture_state);
    [[nodiscard]] const GameState &state() const noexcept { return _state; }
    
    [[nodiscard]] bool should_exit() const noexcept { return false; /* todo */ }
    
    template<typename ...Args>
    [[nodiscard]] static std::unique_ptr<GameLogic> create(Args &&...args) noexcept {
		init();
        return std::make_unique<GameLogic>(std::forward<Args>(args)...);
    }
};
