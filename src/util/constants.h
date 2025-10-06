#ifndef CONSTANTS_H
#define CONSTANTS_H

// solver
#define ROWS 4 // Max scalar rows an individual constraint can have (3D contact = 3n)
#define PENALTY_MIN 1000.0f // Minimum penalty parameter
#define PENALTY_MAX 1e9f // Maximum penalty parameter

#define STICK_THRESH 0.02f

// collision
#define COLLISION_MARGIN 1e-4f
#define GJK_ITERATIONS 15
#define EPA_ITERATIONS 15

#define SHOW_CONTACTS true

#endif