#ifndef CONSTANTS_H
#define CONSTANTS_H

#define ROWS 4 // Max scalar rows an individual constraint can have (3D contact = 3n)
#define PENALTY_MIN 1000.0f // Minimum penalty parameter
#define PENALTY_MAX 1e9f // Maximum penalty parameter
#define COLLISION_MARGIN 1e-4f
#define STICK_THRESH 0.02f
#define SHOW_CONTACTS true
#define GJK_ITERATIONS 15

#endif