#include "Player.h"
#include "World.h"

Player::Player()
{
}

void Player::getBlockLookAtCoords(Camera& camera, World& world)
{
    // Digital Differentation analyzer(fancy name for drawing a line)
    glm::vec3 playerPosOffset = pos + glm::vec3(0.5f); /// offset as block coordinate are of their center, but drawing is done on sides

    int xMap = floor(playerPosOffset.x);
    int yMap = floor(playerPosOffset.y);
    int zMap = floor(playerPosOffset.z);

    glm::vec3 dir = glm::normalize(camera.cameraFront);

    int xStep = (dir.x > 0) ? 1 : -1; // To what direction
    int yStep = (dir.y > 0) ? 1 : -1;
    int zStep = (dir.z > 0) ? 1 : -1;

    float tx = abs(1.0f / dir.x); // Amount of time to travel
    float ty = abs(1.0f / dir.y);
    float tz = abs(1.0f / dir.z);

    float txMax = ((dir.x > 0) ? (xMap + 1 - playerPosOffset.x) : (playerPosOffset.x - xMap)) * tx; // The time to travel to the next grid
    float tyMax = ((dir.y > 0) ? (yMap + 1 - playerPosOffset.y) : (playerPosOffset.y - yMap)) * ty;
    float tzMax = ((dir.z > 0) ? (zMap + 1 - playerPosOffset.z) : (playerPosOffset.z - zMap)) * tz;

    float maxDist = 10.0f;
    float dist = 0.0f;
    isLookingAtBlock = true;
    while (dist < maxDist)
    {
        if (world.returnBlockID(xMap, yMap, zMap) != 0)
        {
            this->lookAtBlockCoords = glm::vec3(xMap, yMap, zMap);
            return;
        }
        if (txMax < tyMax && txMax < tzMax)
        {
            xMap += xStep;
            dist = txMax;
            txMax += tx;
        }
        else if (tyMax < tzMax)
        {
            yMap += yStep;
            dist = tyMax;
            tyMax += ty;
        }
        else
        {
            zMap += zStep;
            dist = tzMax;
            tzMax += tz;
        }
    }
    isLookingAtBlock = false;
    this->lookAtBlockCoords = glm::vec3(xMap, yMap, zMap);
}
