#ifndef __NODE_H__
#define __NODE_H__

#define MAX_SEGMENTS 36 // maximum number of segments in the map
#define MAX_PORTALS_PER_SEGMENT 10
#define PORTAL_LENGTH 2
#define MAX_MAP_SIZE 256
#define MAX_PORTALS 32
#define MAX_STEPS 1000

#define START_PORTAL "AA"
#define END_PORTAL "ZZ"

#define UNDEFINED_LENGTH -1

#define MAP_BLANK ' '
#define MAP_WALL '#'
#define MAP_PASSAGE '.'
#define SEGMENT_BLANK '?'

#define NO_SEGMENTS 0
#define WITH_SEGMENTS 1
#define WITH_PORTALS 2
#define WITH_PORTAL_DIRECTIONS 3

#define OUTSIDE 10
#define INSIDE 11

struct segment
{
    char label;
    int num_portals;
    char portals[MAX_PORTALS_PER_SEGMENT][PORTAL_LENGTH+1]; // the portal namespace
    int portal_to_portal_distances[MAX_PORTALS_PER_SEGMENT][MAX_PORTALS_PER_SEGMENT]; // making it bi-directional just to make things easier
};

typedef struct segment segment;

struct map_node
{
    char display_ch;
    char segment_label;
    int is_portal; // 0 if not portal, 1 if portal
    char portal[PORTAL_LENGTH+1];
    int portal_direction;
    int tmp_length; // used in calculating portal-to-portal distances. putting it here so I don't need to declare other structures later.
};

typedef struct map_node map_node;

struct map
{
    map_node layout[MAX_MAP_SIZE][MAX_MAP_SIZE]; // first index is row, second index is column. should make printing easier
    int num_rows;
    int num_cols;
};

typedef struct map map;

struct universe
{
    map myMap;
    segment segments[MAX_SEGMENTS];
    int num_segments;
    char portals[MAX_PORTALS][PORTAL_LENGTH+1];
    int num_portals;
    segment * start_segment;
    segment * end_segment;
    segment * portal_segments[MAX_PORTALS][2]; // for each portal in the list, this will point to the 2 segments that it connects to
    int portal_directions[MAX_PORTALS][2];
};

typedef struct universe universe;

struct step
{
    segment * segment;
    char from_portal[PORTAL_LENGTH+1];
    char to_portal[PORTAL_LENGTH+1];
    int depth;
};

typedef struct step step;

struct path
{
    int num_steps;
    step steps[MAX_STEPS];
};

typedef struct path path;

void init_segment(segment * segment, char label);
void add_portal_to_segment(segment * segment, char * portal_name);
void set_portal_to_portal_distance(segment * segment, char * from_portal, char * to_portal, int distance);

void init_map_node(map_node * n);
void init_map(map * m, int rows, int cols);
void display_map(map * m, int show_segments);

void init_universe(universe * u);
segment * next_universe_segment(universe * u);
void segmentize(universe * u);
void assign_portals_to_segments(universe * u);
int get_add_portal_number(universe * u, char * portal_name);
void set_portal_to_portal_distances_in_segments(universe * u);

int is_complete_path(path * p);
int calculate_path_length(path * p);
void recursive_work_segments(universe * u, path * current_path, path * best_path);
void find_best_path(universe * u, path * best_path);
void print_path(path * p);
void depth_recursive_work_segments(universe * u, path * current_path, path * best_path);
void depth_find_best_path(universe * u, path * best_path);
void depth_print_path(path * p);
int is_in_worse_loop(path * p);

#endif
