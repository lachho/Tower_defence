// This program was written by Lachlan Ho (z5361811)
// on 28/03/2023
//
// Simulates a Tower Defence Game by continuously taking in set commands from 
// the user. Initial data such as initial conditions (lives, money, enemies),
// map details (start, end, path, lake) is requested. Then specific game 
// commands to simulate attacks, defence and other map environment changes. 
// This program is limited to a single step iterations from the user and cannot
// run automatically like modern tower defence games.   

#include <stdio.h>

#define MAP_ROWS 6
#define MAP_COLUMNS 12
#define OUT_OF_LIVES 0
#define MONEY_EARNED 5
#define ENEMIES 'e'
#define TOWER 't'
#define MOVE 'm'
#define UPGRADE 'u'
#define ATTACK 'a'
#define RAIN 'r'
#define FLOOD 'f'
#define TELEPORT 'c'
#define RIGHT 'r'
#define LEFT 'l'
#define UP 'u'
#define DOWN 'd'

////////////////////////////////////////////////////////////////////////////////
/////////////////////////// USER DEFINED TYPES  ////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
enum land_type {
    GRASS,
    WATER,
    PATH_START,
    PATH_END,
    PATH_UP,
    PATH_RIGHT,
    PATH_DOWN,
    PATH_LEFT,
    TELEPORTER
};

enum entity {
    EMPTY,
    ENEMY,
    BASIC_TOWER,
    POWER_TOWER,
    FORTIFIED_TOWER,
};

enum loop_condition {STOP, CONTINUE};

enum tower_cost {
    COST_BASIC = 200,
    COST_POWER = 300,
    COST_FORTIFIED = 500
};

enum tower_range {
    RANGE_BASIC = 1,
    RANGE_POWER = 1,
    RANGE_FORTIFIED = 2
};

enum tower_power {
    POWER_BASIC = 1,
    POWER_POWER = 2,
    POWER_FORTIFIED = 3
};

struct tower_data {
    int cost;
    int range;
    int power;
};

struct tile {
    enum land_type land;
    enum entity entity;

    int n_enemies;
};

struct coord_data {
    int row;
    int col;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////  YOUR FUNCTION PROTOTYPE  /////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int scan_int(void);
struct coord_data scan_coords(void);
int test_point(int row, int col);
void add_enemies(struct tile map[MAP_ROWS][MAP_COLUMNS], 
                 struct coord_data start);
int test_lake (struct coord_data lake, int height, int width);
void create_lake(struct tile map[MAP_ROWS][MAP_COLUMNS]);
int test_path(struct coord_data position, struct coord_data end);
void create_path(struct tile map[MAP_ROWS][MAP_COLUMNS], int *path_length,
                 struct coord_data path[MAP_ROWS * MAP_COLUMNS], 
                 struct coord_data start, struct coord_data end);
void create_tower(struct tile map[MAP_ROWS][MAP_COLUMNS], int *money);
int move_enemies(struct tile map[MAP_ROWS][MAP_COLUMNS], int *lives,
                 struct coord_data path[MAP_ROWS * MAP_COLUMNS], 
                 struct coord_data start, struct coord_data end, 
                 int path_length, int money);
void test_upgrade_tower(struct tile map[MAP_ROWS][MAP_COLUMNS],
                        struct coord_data tower, int *money, int cost);
void upgrade_tower(struct tile map[MAP_ROWS][MAP_COLUMNS], int *money);
int attack_tower_type(struct tile map[MAP_ROWS][MAP_COLUMNS],
                      struct coord_data path[MAP_ROWS * MAP_COLUMNS], 
                      int tower, int range, int power, int i);
void attack_total(struct tile map[MAP_ROWS][MAP_COLUMNS], int path_length,
                  struct coord_data path[MAP_ROWS * MAP_COLUMNS], int *money);
int test_rain(int ordinate, int offset, int spacing);
void delete_tower(struct tile map[MAP_ROWS][MAP_COLUMNS],
                  int row, int col);
void create_rain(struct tile map[MAP_ROWS][MAP_COLUMNS]);
void copy_2d_array(struct tile map[MAP_ROWS][MAP_COLUMNS], 
                   struct tile map_copy[MAP_ROWS][MAP_COLUMNS]);
void flood_tile(int row, int col, struct tile map[MAP_ROWS][MAP_COLUMNS]);
void flood_surrouning(struct tile map[MAP_ROWS][MAP_COLUMNS], 
                      struct tile map_copy[MAP_ROWS][MAP_COLUMNS],
                      int row, int col);
void create_flood(struct tile map[MAP_ROWS][MAP_COLUMNS]);
void copy_1d_array(int *length, struct coord_data original[MAP_ROWS * MAP_COLUMNS], 
                   struct coord_data copy[MAP_ROWS * MAP_COLUMNS]);
void delete_path(struct tile map[MAP_ROWS][MAP_COLUMNS], int *path_length,
                 struct coord_data path[MAP_ROWS * MAP_COLUMNS]);
void create_tele_path(int *path_length, int start_tele, int end_tele,
                      struct coord_data path[MAP_ROWS * MAP_COLUMNS],
                      struct coord_data path_copy[MAP_ROWS * MAP_COLUMNS],
                      struct tile map[MAP_ROWS][MAP_COLUMNS],
                      struct tile map_copy[MAP_ROWS][MAP_COLUMNS], 
                      struct coord_data end);
void create_teleporter(struct tile map[MAP_ROWS][MAP_COLUMNS], int *path_length,
                       struct coord_data path[MAP_ROWS * MAP_COLUMNS],
                       struct coord_data end);
int game_over(void);

////////////////////////////////////////////////////////////////////////////////
////////////////////// PROVIDED FUNCTION PROTOTYPE  ////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void initialise_map(struct tile map[MAP_ROWS][MAP_COLUMNS]);
void print_map(struct tile map[MAP_ROWS][MAP_COLUMNS], int lives, int money);
void print_tile(struct tile tile, int entity_print);

int main(void) {
    // This `map` variable is a 2D array of `struct tile`s.
    // It is `MAP_ROWS` x `MAP_COLUMNS` in size (which is 6x12 for this
    // assignment!)
    struct tile map[MAP_ROWS][MAP_COLUMNS];

    // This will initialise all tiles in the map to have GRASS land and EMPTY
    // entity values.
    initialise_map(map);
    
    // This scans in lives, money and start/ending points.
    printf("Starting Lives: ");
    int lives = scan_int();
    printf("Starting Money($): ");
    int money = scan_int();
    printf("Start Point: ");
    struct coord_data start = scan_coords();
    printf("End Point: ");
    struct coord_data end = scan_coords();

    // This changes the land value for the start and end points on the camp.
    map[start.row][start.col].land = PATH_START;
    map[end.row][end.col].land = PATH_END;

    print_map(map, lives, money);

    // This scans in number of initial enemies after checking it is valid
    printf("Initial Enemies: ");
    add_enemies(map, start);

    print_map(map, lives, money);
    
    // This creates the lake after checking it is valid
    printf("Enter Lake: "); 
    create_lake(map);
    
    print_map(map, lives, money);       

    // This updates the map with the provided path
    // And also stores the path length, and coordinates of the path route
    int path_length = 0;
    struct coord_data path[MAP_ROWS * MAP_COLUMNS];
    create_path(map, &path_length, path, start, end);

    print_map(map, lives, money);     

    // Loops through the commands provided by the user
    printf("Enter Command: ");
    int game_condition = CONTINUE;
    char command;
    while (game_condition == CONTINUE && scanf(" %c", &command) != EOF) {
        // Adds enemies to the starting square.
        if (command == ENEMIES) {
            add_enemies(map, start);
        }
        // Creates a Tower and adds it to the map. 
        else if (command == TOWER) {
            create_tower(map, &money);
        }
        // Moves the enemies down the path.
        else if (command == MOVE) {
            game_condition = move_enemies(map, &lives, path, start, end, 
                                          path_length, money);
        }
        // Upgrades the tower. 
        else if (command == UPGRADE) {
            upgrade_tower(map, &money);
        }
        // The towers deal damage and reduces the number of enemies in range. 
        else if (command == ATTACK) {
            attack_total(map, path_length, path, &money);
        }
        // creates a pattern of water tiles on the map
        else if (command == RAIN) {
            create_rain(map);
        }
        // Changes tiles adjacent to water into water tiles.
        else if (command == FLOOD) {
            create_flood(map);
        }
        else if (command == TELEPORT) {
            create_teleporter(map, &path_length, path, end);
        }
        if (game_condition) {
            print_map(map, lives, money);
            printf("Enter Command: ");
        }
    }

    return game_over();
}
////////////////////////////////////////////////////////////////////////////////
/////////////////////////////  YOUR FUNCTIONS //////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/**
 * Scans in an integer input, which will be used as a game statistic
 *
 * Parameters:
 *     none
 * Returns:
 *     input - an integer scanned in from the compiler
 */
int scan_int(void) {
    int input;
    scanf(" %d", &input);
    return input;
}

/**
 * This struct stores the x and y coordinates of a specified position
 *
 * Parameters:
 *     none
 * Returns:
 *     data - a struct of 2 integers scanned in from the compiler
 */
struct coord_data scan_coords(void) {
    struct coord_data data;
    data.row = scan_int();
    data.col = scan_int();
    return data;
}

/**
 * Adds enemies to the starting position, if number of enemies is valid

 * 
 * Parameters:
 *     map - The map to initialise.
 *     spawn - number to spawn in
 *    
 * Returns:
 *     Nothing.
 */
void add_enemies(struct tile map[MAP_ROWS][MAP_COLUMNS], 
                 struct coord_data start) {
    int spawn = scan_int();
    // Check to ensure number of enemies is valid
    if (spawn > 0) {
        // Adds the enemies and sets entity to ENEMY
        if (map[start.row][start.col].entity == ENEMY) {
            map[start.row][start.col].n_enemies += spawn;
        } else {
            map[start.row][start.col].entity = ENEMY;
            map[start.row][start.col].n_enemies = spawn;
        }
    }
}

/**
 * Tests a set of coordinates to see if they lie within the map boundaries
 * 
 * Parameters:
 *     row - index 0 component of coordinate
 *     col - index 1 component of coordinate
 * Returns:
 *     1 - if within the bounds
 *     0 - if out of bounds
 */
int test_point(int row, int col) {
    return row >= 0 && 
           col >= 0 &&
           row < MAP_ROWS &&
           col < MAP_COLUMNS;
}

/**
 * Tests if the lake can exists by testing if boundary points sit in map
 * 
 * Parameters:
 *     lake - start coordinates of the lake
 *     lake_size - dimensions of the lake
 * Returns:
 *     1 - if points within boundary.
 *     0 - if not.
 */
int test_lake(struct coord_data lake, int height, int width) {
    int lake_edge_row = lake.row + height - 1;
    int lake_edge_col = lake.col + width - 1;
    return test_point(lake.row, lake.col)  &&
           test_point(lake_edge_row, lake_edge_col);
}

/**
 * Creates the lake and updates it on the map
 * 
 * Parameters:
 *     map - The map to initialise.
 *     lake - start coordinates of the lake
 *     lake_size - dimensions of the lake
 * Returns:
 *     Nothing.
 */
void create_lake(struct tile map[MAP_ROWS][MAP_COLUMNS]) {
    struct coord_data lake = scan_coords();
    int height = scan_int();
    int width = scan_int(); 

    // Tests if boundary points lie within the map
    if (test_lake(lake, height, width)) {
        int row = lake.row;
        while (row < lake.row + height) {
            int col = lake.col;
            while (col < lake.col + width) {
                map[row][col].land = WATER;
                col++;
            } 
            row++;
        }
    } else {
        printf("Error: Lake out of bounds, ignoring...\n");
    }
}

/**
 * Checks if current position is end point, returns 0 is at end
 * 
 * Parameters:
 *     position - current coordinates of the path
 *     end - cordinates of the end square
 * Returns:
 *     1 - if current position is not end point.
 *     0 - if it is.
 */
int test_path(struct coord_data position, struct coord_data end) { 
    return position.row != end.row || position.col != end.col;
}

/**
 * Reads the path and changes the tiles to the path direction
 * 
 * Parameters:
 *     map - map of the tiles
 *     path_length - counts length of the path
 *     path - struct array of the coordinates of the path route
 *     start - coordinates of the start tile
 *     end - coordinates of the end tile
 * Returns:
 *     nothing
 */
void create_path(struct tile map[MAP_ROWS][MAP_COLUMNS], int *path_length,
                  struct coord_data path[MAP_ROWS * MAP_COLUMNS], 
                  struct coord_data start, struct coord_data end) {
    printf("Enter Path: ");
    // Sets up current position as we work through the path
    struct coord_data position;
    position.row = start.row;
    position.col = start.col;
    // stores the coordinates of the path in an array
    path[*path_length].row = start.row;
    path[*path_length].col = start.col;

    int reach_end = CONTINUE;
    char direction;
    while (reach_end == CONTINUE) {
        scanf(" %c", &direction);
    
        // Updates the land space with direction and moves to the next position.
        if (direction == RIGHT) {
            map[position.row][position.col].land = PATH_RIGHT;
            map[position.row][position.col].land = PATH_RIGHT;
            position.col++;
        }
        else if (direction == LEFT) {
            map[position.row][position.col].land = PATH_LEFT;
            position.col--;
        }
        else if (direction == UP) {
            map[position.row][position.col].land = PATH_UP;
            position.row--;
        } 
        else if (direction == DOWN) {
            map[position.row][position.col].land = PATH_DOWN;
            position.row++;
        }
        
        *path_length = *path_length + 1;
        path[*path_length].row = position.row;
        path[*path_length].col = position.col;
        
        reach_end = test_path(position, end);
    }
}

/**
 * Creates a tower at a point, after checking if its allowed. 
 * 
 * Parameters:
 *     map - map of the tiles
 *     *money - total amount of money
 * Returns:
 *     nothing
 */
void create_tower(struct tile map[MAP_ROWS][MAP_COLUMNS], int *money) {
    // Takes in a set of coordinates
    struct coord_data tower = scan_coords();
    // Checks all the conditions for creating a tower is passed
    if (
        *money >= COST_BASIC &&
        test_point(tower.row, tower.col) &&
        map[tower.row][tower.col].land == GRASS &&
        map[tower.row][tower.col].entity == EMPTY
    ) {
        map[tower.row][tower.col].entity = BASIC_TOWER;
        *money -= COST_BASIC;
        printf("Tower successfully created!\n");
    } else { 
        printf("Error: Tower creation unsuccessful. "
                "Make sure you have at least $200 and that the tower "
                "is placed on a grass block with no entity.\n");
    }
}

/**
 * Moves the enemies depending on the input from the user
 * Then removes lives, depending on how many enemies made it to the end tile
 * 
 * Parameters:
 *     map - map of the tiles
 *     *lives - number of lives
 *     path - array of structs of the path
 *     start - start row and column
  *    end - end row and column
 *     money - total amount of money
 * Returns:
 *     CONTINUE - if lives are more than 0
 *     STOP - if lives are less than or equal to 0
 */
int move_enemies(struct tile map[MAP_ROWS][MAP_COLUMNS], int *lives,
                 struct coord_data path[MAP_ROWS * MAP_COLUMNS], 
                 struct coord_data start, struct coord_data end, 
                 int path_length, int money) {
    int lives_lost = 0;
    // Scans in the number of advances to make
    int repeat = scan_int();
    int iteration = 0;
    // Repeat for the number of advances required by the input. 
    while (iteration < repeat) {

        // Start from the end of the path, set the current tile's number of 
        // enemies to the prior path tile's number of enemies
        int i = path_length;
        while (i > 0) {
            struct coord_data current = path[i];
            struct coord_data previous = path[i - 1];
            map[current.row][current.col].entity = 
                map[previous.row][previous.col].entity;
            map[current.row][current.col].n_enemies = 
                map[previous.row][previous.col].n_enemies;
            i--;
        }

        // Start cell after moving enemies will be empty and 0
        map[start.row][start.col].entity = EMPTY;
        map[start.row][start.col].n_enemies = 0;
        
        // Checks if enemies made it to the end tile and decreases total lives. 
        if (map[end.row][end.col].entity == ENEMY) {
            *lives -= map[end.row][end.col].n_enemies;
            lives_lost += map[end.row][end.col].n_enemies;
            map[end.row][end.col].entity = EMPTY;
            map[end.row][end.col].n_enemies = 0;
        }
        iteration++;
    }
    
    printf("%d enemies reached the end!\n", lives_lost);
    
    // This checks if the game is out of lives. 
    if (*lives <= OUT_OF_LIVES) {
        print_map(map, *lives, money);
        printf("Oh no, you ran out of lives!"); 
        return STOP;
    } else {
        return CONTINUE;
    }
}

/**
 * Checks if there is enough money for the upgrade, and completes the upgrade
 * 
 * Parameters:
 *     map - map of the tiles
 *     tower - cordinates of the end square
 *     *money - remaining money
 *     cost - cost of the upgrade
 * Returns:
 *     nothing
 */
void test_upgrade_tower(struct tile map[MAP_ROWS][MAP_COLUMNS],
                        struct coord_data tower, int *money, int cost) {
    // Ensures there is enough money for upgrade cost
    if (*money >= cost) {
        // Subtracks money from running total and updates the tower on the map. 
        *money -= cost;
        map[tower.row][tower.col].entity++;
        printf("Upgrade Successful!\n");
    } else {
        printf("Error: Insufficient Funds.\n");
    }
}

/**
 * Creates a tower, after checking if its allowed. 
 * 
 * Parameters:
 *     map - map of the tiles
 *     *money - total amount of money
 * Returns:
 *     nothing
 */
void upgrade_tower(struct tile map[MAP_ROWS][MAP_COLUMNS], int *money) {
    // Takes in a set of coordinates
    struct coord_data tower = scan_coords();
    // Checks to ensure all conditions pass. 
    if (!test_point(tower.row, tower.col)) {
        printf("Error: Upgrade target is out-of-bounds.\n");
    }
    else if (
        map[tower.row][tower.col].entity == ENEMY ||
        map[tower.row][tower.col].entity == EMPTY
    ) {
        printf("Error: Upgrade target contains no tower entity.\n");
    }
    else if (map[tower.row][tower.col].entity == FORTIFIED_TOWER) {
        printf("Error: Tower cannot be upgraded further.\n");
    }
    else if (map[tower.row][tower.col].entity == BASIC_TOWER) {
        test_upgrade_tower(map, tower, money, COST_POWER);
    }
    else if (map[tower.row][tower.col].entity == POWER_TOWER) {
        test_upgrade_tower(map, tower, money, COST_FORTIFIED);
    }
}

/**
 * Checks the surrounding cells of a tile for a specific tower type that is in 
 * range, and calculates the damage that tile takes from that type.
 * 
 * Parameters:
 *     map - map of the tiles
 *     path - array of structs of the path
 *     tower - specifies the tower type to check
 *     range - the range of the tower it can reach
 *     power - the damage the tower can deal
 * Returns:
 *     damage - total damage all the surrounding tower of that type did. 
 */
int attack_tower_type(struct tile map[MAP_ROWS][MAP_COLUMNS],
               struct coord_data path[MAP_ROWS * MAP_COLUMNS], 
               int tower, int range, int power, int i) {
    int damage = 0;
    int row_count = 0;
    // Start at the top corner of the towers range
    int row = path[i].row - range;
    // Uses formula to convert given range into  
    int check_range = range * 2 + 1;
    while (row_count < check_range) {
        int col_count = 0;
        int col = path[i].col - range;
        while (col_count < check_range) {
            if (test_point(row, col) && map[row][col].entity == tower) {
                damage += power; 
            }
            col_count++;
            col++;
        }
        row_count++;
        row++;
    }
    return damage;
}

/**
 * Checks every path tile for surrounding towers that can deal damage and 
 * repeats the attacks, the number of times from the input.
 * 
 * Parameters:
 *     map - map of the tiles
 *     path_length - number of tiles that are path tiles
 *     path - array of structs of the path
 *     *money - amount of money remaining
 * Returns:
 *     nothing
 */
void attack_total(struct tile map[MAP_ROWS][MAP_COLUMNS], int path_length,
                  struct coord_data path[MAP_ROWS * MAP_COLUMNS], int *money) {
    int total_destroyed = 0;
    int repeat = scan_int();
    int iteration = 0;
    while (iteration < repeat) {
        int i = 0;
        // We loop through each tile along the path
        while (i < path_length) {
            // Determines if the tile is in range of the surrounding tiles
            // and sums up the total damage taken from these towers
            int total_damage = 0;
            total_damage += attack_tower_type(map, path, BASIC_TOWER, 
                                              RANGE_BASIC, POWER_BASIC, i);
            total_damage += attack_tower_type(map, path, POWER_TOWER, 
                                              RANGE_POWER, POWER_POWER, i);
            total_damage += attack_tower_type(map, path, FORTIFIED_TOWER, 
                                              RANGE_FORTIFIED, POWER_FORTIFIED, 
                                              i);
            
            // Caps total damage to the amount of enemies at that tile
            struct coord_data current = path[i];
            if (total_damage >= map[current.row][current.col].n_enemies) {
                total_damage = map[current.row][current.col].n_enemies;
                map[current.row][current.col].entity = EMPTY;
            }

            // updates lives and money
            map[current.row][current.col].n_enemies -= total_damage;
            *money += total_damage * MONEY_EARNED;
            total_destroyed += total_damage;
            i++;
        }
        iteration++;
    }
    printf("%d enemies destroyed!\n", total_destroyed);
}

/**
 * Checks if an ordinate fits in the offset and spacing rules
 * 
 * Parameters:
 *     ordinate - ordinate to test
 *     offset - offset of the rain
 *     spacing - spacing of the rain
 * Returns:
 *     1 - if current position is not end point.
 *     0 - if it is.
 */
int test_rain(int ordinate, int offset, int spacing) {
    return (ordinate - offset) % spacing == 0;
}
 
/**
 * Checks if tower exists on the tile and removes if true. 
 * 
 * Parameters:
 *     map - map of the tiles
 * Returns:
 *     nothing
 */
void delete_tower(struct tile map[MAP_ROWS][MAP_COLUMNS],
                  int row, int col) {
    if (
        map[row][col].entity == BASIC_TOWER ||
        map[row][col].entity == POWER_TOWER
    ) {
        map[row][col].entity = EMPTY;
    }
}

/**
 * Creates water tiles from grass tiles in a pattern
 * 
 * Parameters:
 *     map - map of the tiles
 * Returns:
 *     nothing
 */
void create_rain(struct tile map[MAP_ROWS][MAP_COLUMNS]) {
    struct coord_data spacing;
    spacing = scan_coords();
    struct coord_data offset;
    offset = scan_coords();

    int row = 0;
    while (row < MAP_ROWS) {
        int col = 0;
        while (col < MAP_COLUMNS) {
            // Checks if tile fits in the offset and spacing
            if (
                test_rain(row, offset.row, spacing.row) &&
                test_rain(col, offset.col, spacing.col) &&
                map[row][col].land == GRASS
            ) {
                map[row][col].land = WATER;
                delete_tower(map, row, col);
            }
            col++;
        }
        row++;
    }
}

/**
 * Creates a copy of a 2d array. 
 * 
 * Parameters:
 *     original - original array to copy
 *     copy - blank array to copy into from original
 * Returns:
 *     nothing
 */
void copy_2d_array(struct tile original[MAP_ROWS][MAP_COLUMNS], 
                   struct tile copy[MAP_ROWS][MAP_COLUMNS]) {
    int row = 0;
    while (row < MAP_ROWS) {
        int col = 0;
        while (col < MAP_COLUMNS) {
            copy[row][col] = original[row][col];
            col++;
        }
        row++;
    }
}

/**
 * Checks if the given tile is floodable.
 * If yes, then floods, and breaks un-fortified towers.
 * Otherwise does nothing.
 * 
 * Parameters:
 *     row - tile row
 *     col - tile col
 *     map - map of the tiles
 * Returns:
 *     nothing
 */
void flood_tile(int row, int col, struct tile map[MAP_ROWS][MAP_COLUMNS]) {
    if (test_point(row, col) && map[row][col].land == GRASS) {
        map[row][col].land = WATER;
        if (
            map[row][col].entity == BASIC_TOWER || 
            map[row][col].entity == POWER_TOWER
        ) {
            map[row][col].entity = EMPTY;
        }
    }
}

/**
 * Converts the surrouding tiles into water land types. 
 * 
 * Parameters:
 *     row - tile row
 *     col - tile col
 *     map - map of the tiles
 *     map_copy - copy of map
 * Returns:
 *     nothing
 */
void flood_surrouning(struct tile map[MAP_ROWS][MAP_COLUMNS], 
                      struct tile map_copy[MAP_ROWS][MAP_COLUMNS],
                      int row, int col) {
    if (map_copy[row][col].land == WATER) {
        flood_tile(row - 1, col, map);
        flood_tile(row + 1, col, map);
        flood_tile(row, col - 1, map);
        flood_tile(row, col + 1, map);
    }
}

/**
 * Creates a flood from the given input.
 * 
 * Parameters:
 *     map - map of the tiles
 * Returns:
 *     nothing
 */
void create_flood(struct tile map[MAP_ROWS][MAP_COLUMNS]) {
    int repeat = scan_int();
    int iteration = 0;
    while (iteration < repeat) {
        struct tile map_copy[MAP_ROWS][MAP_COLUMNS];
        copy_2d_array(map, map_copy);
        
        // checks each tile for water and if true, floods surroundings.
        int row = 0;
        while (row < MAP_ROWS) {
            int col = 0;
            while (col < MAP_COLUMNS) {
                flood_surrouning(map, map_copy, row, col);
                col++;
            }
            row++;
        }
        iteration++;
    }
}

/**
 * Creates a copy of the given array.
 * 
 * Parameters:
 *     length - length of the path
 *     original - original array to copy and then convert to 0s.
 *     copy - blank array to copy into from original
 * Returns:
 *     nothing
 */
void copy_1d_array(int *length, struct coord_data original[MAP_ROWS * MAP_COLUMNS], 
                   struct coord_data copy[MAP_ROWS * MAP_COLUMNS]) {
    int i = 0;
    while (i < *length + 1) {
        copy[i] = original[i];
        i++;
    }
}

/**
 * deletes the current given path from the path array,
 * and removes the path and enemies from the map.  
 * 
 * Parameters:
 *     length - length of the path
 *     original - original array to copy
 *     copy - blank array to copy into from original
 * Returns:
 *     nothing
 */
void delete_path(struct tile map[MAP_ROWS][MAP_COLUMNS], int *path_length,
                 struct coord_data path[MAP_ROWS * MAP_COLUMNS]) {
    int i = 0;
    while (i < *path_length) {
        struct coord_data current = path[i];
        map[current.row][current.col].land = GRASS;
        map[current.row][current.col].entity = EMPTY;
        map[current.row][current.col].n_enemies = 0; 
        path[i].row = 0;
        path[i].col = 0;
        i++;
    }
}

/**
 * Creates a new path with the teleporters. When the path reaches the start
 * teleporter, the code skips the path until it reaches the end teleporter. 
 * 
 * Parameters:
 *     *path_length - length of the created path
 *     start_tele - position along the path where the teleporter starts
 *     end_tele - position along the path where the teleporter ends
 *     path - struct array of the coordinates of the path route
 *     path_copy - copy of the array of the path
 *     map - map of the tiles 
 *     map_copy - copy of the map of the tiles 
 *     end - coordinates of the end tile
 * Returns:
 *     nothing
 */
void create_tele_path(int *path_length, int start_tele, int end_tele,
                      struct coord_data path[MAP_ROWS * MAP_COLUMNS],
                      struct coord_data path_copy[MAP_ROWS * MAP_COLUMNS],
                      struct tile map[MAP_ROWS][MAP_COLUMNS],
                      struct tile map_copy[MAP_ROWS][MAP_COLUMNS], 
                      struct coord_data end) {
    int new_path_count = 0;
    int i = 0;
    while (i < *path_length + 1) {
        struct coord_data current = path_copy[i];

        path[new_path_count] = current;
        map[current.row][current.col] = map_copy[current.row][current.col];
        if (i == start_tele) {
            map[current.row][current.col].land = TELEPORTER;
            i = end_tele;
        } else {
            if (i == end_tele) {
                map[current.row][current.col].land = TELEPORTER;
            }
            i++;
        }
        new_path_count++;
    }
    path[i] = end;
    *path_length = new_path_count - 1;
}

/**
 * Creates a teleporter on the path. Enemies travel from one teleporter 
 * to the next. 
 * 
 * Parameters:
 *     map - map of the tiles 
 *     *path_length - length of the created path
 *     path - struct array of the coordinates of the path route
 *     end - coordinates of the end tile
 * Returns:
 *     nothing
 */
void create_teleporter(struct tile map[MAP_ROWS][MAP_COLUMNS], int *path_length,
                       struct coord_data path[MAP_ROWS * MAP_COLUMNS],
                       struct coord_data end) {
    struct coord_data tele_1;
    tele_1 = scan_coords();
    struct coord_data tele_2;
    tele_2 = scan_coords();     
    
    int i = 0;
    int tele_path_1 = EOF;
    int tele_path_2 = EOF;
    // mins one since we dont want to test the end tile
    // determines where on the path the teleporters lie. 
    while (i < *path_length) {
        if (path[i].row == tele_1.row && path[i].col == tele_1.col) {
            tele_path_1 = i;
        }
        else if (path[i].row == tele_2.row && path[i].col == tele_2.col) {
            tele_path_2 = i;
        }
        i++;
    }
    // If above loop didn't find teleporters on the path, then print error. 
    if (tele_path_1 == EOF || tele_path_2 == EOF) {
        printf("Error: Teleporters can only be created on path tiles.\n");
    } else {
        struct coord_data path_copy[MAP_ROWS * MAP_COLUMNS];
        copy_1d_array(path_length, path, path_copy);      
        
        struct tile map_copy[MAP_ROWS][MAP_COLUMNS];
        copy_2d_array(map, map_copy);

        delete_path(map, path_length, path);

        // teleporter that appears earlier in the path is set as start tele.
        if (tele_path_1 < tele_path_2) {
            create_tele_path(path_length, tele_path_1, tele_path_2, path, 
                             path_copy, map, map_copy, end);
        } else {
            create_tele_path(path_length, tele_path_2, tele_path_1, path, 
                             path_copy, map, map_copy, end);
        }            
    }


}

/**
 * Prints Game Over and ends the program 
 * 
 * Parameters:
 *     none.
 * Returns:
 *     0 - return nothing to end main.
 */
int game_over(void) {
    printf("\nGame Over!\n");
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
/////////////////////////// PROVIDED FUNCTIONS  ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Written and provided by University of New South Wales (2023)
/**
 * Initialises map tiles to contain GRASS land and EMPTY entity.
 * 
 * Parameters:
 *     map - The map to initialise.
 * Returns:
 *     Nothing.
 */
void initialise_map(struct tile map[MAP_ROWS][MAP_COLUMNS]) {
    for (int row = 0; row < MAP_ROWS; ++row) {
        for (int col = 0; col < MAP_COLUMNS; ++col) {
            map[row][col].land = GRASS;
            map[row][col].entity = EMPTY;
            map[row][col].n_enemies = 0;
        }
    }
}

/**
 * Prints either the land or entity component of a single tile, based on
 * the `land_print` parameter;
 * 
 * Parameters:
 *     tile - The tile to print the land/entity from
 *     land_print - Whether to print the land part of the tile or the entity
 *         part of the tile. If this value is 0, it prints the land, otherwise
 *         it prints the entity.
 * Returns:
 *     Nothing.
 */
void print_tile(struct tile tile, int land_print) {
    if (land_print) {
        if (tile.land == GRASS) {
            printf(" . ");
        } else if (tile.land == WATER) {
            printf(" ~ ");
        } else if (tile.land == PATH_START) {
            printf(" S ");
        } else if (tile.land == PATH_END) {
            printf(" E ");
        } else if (tile.land == PATH_UP) {
            printf(" ^ ");
        } else if (tile.land == PATH_RIGHT) {
            printf(" > ");
        } else if (tile.land == PATH_DOWN) {
            printf(" v ");
        } else if (tile.land == PATH_LEFT) {
            printf(" < ");
        } else if (tile.land == TELEPORTER) {
            printf("( )");
        } else {
            printf(" ? ");
        }
    } else {
        if (tile.entity == EMPTY) {
            printf("   ");
        } else if (tile.entity == ENEMY) {
            printf("%03d", tile.n_enemies);
        } else if (tile.entity == BASIC_TOWER) {
            printf("[B]");
        } else if (tile.entity == POWER_TOWER) {
            printf("[P]");
        } else if (tile.entity == FORTIFIED_TOWER) {
            printf("[F]");
        } else {
            printf(" ? ");
        }
    }
}


/**
 * Prints all map tiles based on their value, with a header displaying lives
 * and money.
 * 
 * Parameters:
 *     map   - The map to print tiles from.
 *     lives - The number of lives to print with the map.
 *     money - The amount of money to print with the map.
 * Returns:
 *     Nothing.
 */
void print_map(struct tile map[MAP_ROWS][MAP_COLUMNS], int lives, int money) {
    printf("\nLives: %d Money: $%d\n", lives, money);
    for (int row = 0; row < MAP_ROWS * 2; ++row) {
        for (int col = 0; col < MAP_COLUMNS; ++col) {
            print_tile(map[row / 2][col], row % 2);
        }
        printf("\n");
    }
}


