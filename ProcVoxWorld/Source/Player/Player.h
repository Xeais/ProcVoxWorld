#pragma once

#include "glad/glad.h"
#include "cglm/cglm.h"

typedef struct
{
    vec3 pos;

    vec3 front;
    vec3 up;
    vec3 speed;

    float yaw;
    float pitch;

    uint8_t buildBlock;
    bool pointingAtBlock;
    ivec3 blockPointedAt;

    vec3 hitbox[2];

    bool onGround;
    bool inWater;
    bool isSneaking;
    bool isRunning;

    GLuint VAOItem;
    GLuint VBOItem;
#pragma warning(suppress: 4324)
    mat4 ModelMatItem;
} Player;

Player* PlayerCreate();

void PlayerSetBuildBlock(Player* p, int32_t newBlock);

void PlayerUpdate(Player* p);

void PlayerRenderItem(Player* p);

void PlayerUpdateHitbox(Player* p);

void PlayerSetViewDir(Player* p, float pitch, float yaw);

void PlayerSave(Player* p);

void PlayerDestroy(Player* p);