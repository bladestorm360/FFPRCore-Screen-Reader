#pragma once
namespace ff3::puzzles {
bool supported(int map);
bool active();
bool repeatable();
void next();
void move();
void stop(bool announce);
}
