#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "node.h"

void init_segment(segment * segment, char label)
{
    segment->label=label;
    segment->num_portals=0;
    for (int i=0; i<MAX_PORTALS_PER_SEGMENT; i++)
    {
        for (int j=0; j<MAX_PORTALS_PER_SEGMENT; j++)
        {
            segment->portal_to_portal_distances[i][j]=UNDEFINED_LENGTH;
        }
    }
}

void add_portal_to_segment(segment * segment, char * portal_name)
{
    if (segment->num_portals==MAX_PORTALS_PER_SEGMENT)
    {
        fprintf(stderr, "Attempting to exceed the number of portals per segment for segment %c. Increase number and recompile\n", segment->label);
        exit(1);
    }
    strncpy(segment->portals[segment->num_portals], portal_name, PORTAL_LENGTH+1);
    segment->num_portals++;
}

void set_portal_to_portal_distance(segment * segment, char * from_portal, char * to_portal, int distance)
{
    int from_index=-1;
    int to_index=-1;
    for (int i=0; i<segment->num_portals; i++)
    {
        if (strncmp(segment->portals[i], from_portal, PORTAL_LENGTH) == 0)
            from_index=i;
        if (strncmp(segment->portals[i], to_portal, PORTAL_LENGTH) == 0)
            to_index=i;
    }
    if (from_index == -1 || to_index == -1)
    {
        if (from_index == -1)
            fprintf(stderr, "In segment %c, the from_portal of %s is not found\n", segment->label, from_portal);
        if (to_index == -1)
            fprintf(stderr, "In segment %c, the to_portal of %s is not found\n", segment->label, to_portal);
        exit(-1);
    }
    segment->portal_to_portal_distances[from_index][to_index]=distance;
}

void init_map_node(map_node * n)
{
    n->display_ch=MAP_BLANK;
    n->segment_label=SEGMENT_BLANK;
    n->is_portal=0;
    n->portal[0]='\0';
    n->portal_direction=0;
    n->tmp_length=UNDEFINED_LENGTH;
}

void init_map(map * m, int rows, int cols)
{
    m->num_rows=rows;
    m->num_cols=cols;
    printf("initializing a row=%d col=%d map\n", rows, cols);
    for (int row=0; row<rows; row++)
    {
        for (int col=0; col<cols; col++)
        {
            init_map_node(&m->layout[row][col]);
        }
    }
}

int isPassage(char ch)
{
    return (ch==MAP_PASSAGE?1:0);
}

void display_map(map * m, int print_type)
{
    for (int row=0; row<m->num_rows; row++)
    {
        for (int col=0; col<m->num_cols; col++)
        {
            char ch=m->layout[row][col].display_ch;
            if (isPassage(ch) && print_type==WITH_SEGMENTS)
                ch=m->layout[row][col].segment_label;
            if (m->layout[row][col].is_portal==1 && print_type==WITH_PORTALS)
                ch='*';
            if (m->layout[row][col].is_portal==1 && print_type==WITH_PORTAL_DIRECTIONS)
            {
                if (m->layout[row][col].portal_direction==OUTSIDE)
                    ch='O';
                else if (m->layout[row][col].portal_direction==INSIDE)
                    ch='I';
                else
                    ch='?';
            }
            printf("%c", ch);
        }
        printf("\n");
    }
}

void init_universe(universe * u)
{
    u->num_segments=0;
    u->num_portals=0;
    u->start_segment=NULL;
    u->end_segment=NULL;
    for (int i=0; i<MAX_PORTALS; i++)
    {
        u->portals[i][0]='\0';
        u->portal_segments[i][0]=NULL;
        u->portal_segments[i][1]=NULL;
        u->portal_directions[i][0]=0;
        u->portal_directions[i][1]=0;        
    }
}

segment * next_universe_segment(universe * u)
{
    if (u->num_segments==MAX_SEGMENTS)
    {
        fprintf(stderr, "Attempting to exceed the number of segments in the universe. Increase number and recompile\n");
        exit(1);
    }
    segment * next=&u->segments[u->num_segments];
    char label=(u->num_segments<26?'a'+u->num_segments:'A'+u->num_segments-26);
    u->num_segments++;
    init_segment(next, label);
    return next;
}

void fill_segment_in_map(map * m, segment * s)
{
    int workToDo = 1;
    char label=s->label;
    while(workToDo != 0)
    {
        workToDo=0;
        for (int row=0; row<m->num_rows; row++)
        {
            for (int col=0; col<m->num_cols; col++)
            {
                if (m->layout[row][col].segment_label==label)
                {
                    // up
                    if (row > 0)
                    {
                        if (isPassage(m->layout[row-1][col].display_ch) && m->layout[row-1][col].segment_label==SEGMENT_BLANK)
                        {
                            workToDo=1;
                            m->layout[row-1][col].segment_label=label;
                        }
                    }
                    // down
                    if (row < m->num_rows-1)
                    {
                        if (isPassage(m->layout[row+1][col].display_ch) && m->layout[row+1][col].segment_label==SEGMENT_BLANK)
                        {
                            workToDo=1;
                            m->layout[row+1][col].segment_label=label;
                        }
                    }
                    // left
                    if (col > 0)
                    {
                        if (isPassage(m->layout[row][col-1].display_ch) && m->layout[row][col-1].segment_label==SEGMENT_BLANK)
                        {
                            workToDo=1;
                            m->layout[row][col-1].segment_label=label;
                        }
                    }
                    // right
                    if (col < m->num_cols-1)
                    {
                        if (isPassage(m->layout[row][col+1].display_ch) && m->layout[row][col+1].segment_label==SEGMENT_BLANK)
                        {
                            workToDo=1;
                            m->layout[row][col+1].segment_label=label;
                        }
                    }
                }
            }
        }
    }
}  

void segmentize(universe * u)
{
    map * m=&u->myMap;
    printf("Initial map:\n");
    display_map(m, NO_SEGMENTS);
    for (int row=0; row<m->num_rows; row++)
    {
        for (int col=0; col<m->num_cols; col++)
        {
            map_node * node=&m->layout[row][col];
            if (isPassage(node->display_ch) && node->segment_label==SEGMENT_BLANK)
            {
                // this is a passage that does not have a segment assigned.
                segment * s = next_universe_segment(u);
                node->segment_label=s->label;
                fill_segment_in_map(m, s);
                printf("After allocating segment %c:\n", s->label);
                display_map(m, WITH_SEGMENTS);
            }
        }
    }
    printf("Final segmented map:\n");
    display_map(m, WITH_SEGMENTS);
}

int get_segment_number(universe * u, char label)
{
    for (int i=0; i<u->num_segments; i++)
    {
        if (u->segments[i].label==label)
            return i;
    }
    fprintf(stderr, "Segment %c not found. Fix your code.\n", label);
    exit(-1);
}

int get_add_portal_number(universe * u, char * portal_name)
{
    for (int i=0; i<u->num_portals; i++)
    {
        if (strncmp(u->portals[i], portal_name, PORTAL_LENGTH) == 0)
            return i;
    }
    if (u->num_portals==MAX_PORTALS)
    {
        fprintf(stderr, "Attempting to exceed the number of portals with portal %s. Increase number and recompile\n", portal_name);
        exit(1);
    }
    printf("Adding portal %s to location %d\n", portal_name, u->num_portals);
    strncpy(u->portals[u->num_portals], portal_name, PORTAL_LENGTH+1);
    u->num_portals++;
    return u->num_portals-1;
}

int get_segment_portal_number(segment * s, char * portal_name)
{
    for (int i=0; i<s->num_portals; i++)
        if (strncmp(portal_name, s->portals[i], PORTAL_LENGTH)==0)
            return i;
    fprintf(stderr, "Portal %s not found in segment %c. Fix your code\n", portal_name, s->label);
}

void assign_portals_to_segments(universe * u)
{
    for (int row=0; row<u->myMap.num_rows; row++)
    {
        for (int col=0; col<u->myMap.num_cols; col++)
        {
            map_node * n=&u->myMap.layout[row][col];
            if (n->is_portal==1)
            {
                int segment_number=get_segment_number(u, n->segment_label);
                segment * s=&u->segments[segment_number];
                int portal_number=get_add_portal_number(u, n->portal);
                add_portal_to_segment(s, n->portal);
                if (u->portal_segments[portal_number][0]==NULL)
                {
                    printf("Setting first segment for %s to %c\n", n->portal, s->label);
                    u->portal_segments[portal_number][0]=s;
                    u->portal_directions[portal_number][0]=n->portal_direction;
                }
                else
                {
                    printf("Setting second segment for %s to %c\n", n->portal, s->label);
                    u->portal_segments[portal_number][1]=s;
                    u->portal_directions[portal_number][1]=n->portal_direction;
                }
                if (strncmp(n->portal, START_PORTAL, PORTAL_LENGTH)==0)
                {
                    printf("Setting start segment to segment %c\n", s->label);
                    u->start_segment=s;
                }
                if (strncmp(n->portal, END_PORTAL, PORTAL_LENGTH)==0)
                {
                    printf("Setting end segment to segment %c\n", s->label);
                    u->end_segment=s;
                }                
            }
        }
    }
}

void set_portal_to_portal_distances_in_segments(universe * u)
{
    map * m=&u->myMap;

    for (int outerRow=0; outerRow<m->num_rows; outerRow++)
    {
        for (int outerCol=0; outerCol<m->num_cols; outerCol++)
        {
            map_node * n=&u->myMap.layout[outerRow][outerCol];
            if (n->is_portal==1)
            {
                segment * s=&u->segments[get_segment_number(u, n->segment_label)];
                int sp=get_segment_portal_number(s, n->portal);
                printf("Working through map to find distances from portal %s (row=%d, col=%d) in segment %c\n", n->portal, outerRow, outerCol, s->label);
                
                for (int i=0; i<MAX_MAP_SIZE; i++)
                {
                    for (int j=0; j<MAX_MAP_SIZE; j++)
                    {
                        m->layout[i][j].tmp_length=UNDEFINED_LENGTH;
                    }
                }
                
                m->layout[outerRow][outerCol].tmp_length=0;
                int workToDo = 1;
                int current_distance = 0;
                char label=s->label;
                while(workToDo != 0)
                {
                    workToDo=0;
                    for (int row=0; row<m->num_rows; row++)
                    {
                        for (int col=0; col<m->num_cols; col++)
                        {
                            map_node * curr = &m->layout[row][col];
                            if (curr->segment_label==label && curr->tmp_length==current_distance)
                            {
                                // check if this one is a portal
                                if (curr->is_portal==1)
                                {
                                    int other_sp=get_segment_portal_number(s, curr->portal);
                                    printf("Setting distance of %d from %s to %s in segment %c\n", current_distance, n->portal, curr->portal, label);
                                    s->portal_to_portal_distances[sp][other_sp]=current_distance;
                                }

                                // up
                                if (row > 0)
                                {
                                    if (isPassage(m->layout[row-1][col].display_ch) && m->layout[row-1][col].tmp_length==UNDEFINED_LENGTH)
                                    {
                                        workToDo=1;
                                        m->layout[row-1][col].tmp_length=current_distance+1;
                                    }
                                }
                                // down
                                if (row < m->num_rows-1)
                                {
                                    if (isPassage(m->layout[row+1][col].display_ch) && m->layout[row+1][col].tmp_length==UNDEFINED_LENGTH)
                                    {
                                        workToDo=1;
                                        m->layout[row+1][col].tmp_length=current_distance+1;
                                    }
                                }
                                // left
                                if (col > 0)
                                {
                                    if (isPassage(m->layout[row][col-1].display_ch) && m->layout[row][col-1].tmp_length==UNDEFINED_LENGTH)
                                    {
                                        workToDo=1;
                                        m->layout[row][col-1].tmp_length=current_distance+1;
                                    }
                                }
                                // right
                                if (col < m->num_cols-1)
                                {
                                    if (isPassage(m->layout[row][col+1].display_ch) && m->layout[row][col+1].tmp_length==UNDEFINED_LENGTH)
                                    {
                                        workToDo=1;
                                        m->layout[row][col+1].tmp_length=current_distance+1;
                                    }
                                }
                            }
                        }
                    }
                    current_distance++;
                }
            }
        }
    }
}

// this is a complete path if the last step ends at the end portal
int is_complete_path(path * p)
{
    return strncmp(p->steps[p->num_steps-1].to_portal, END_PORTAL, PORTAL_LENGTH)==0 ? 1 : 0;
}

int calculate_path_length(path * p)
{
    int length=0;
    
    for (int i=0; i<p->num_steps; i++)
    {
        segment * s = p->steps[i].segment;
        int from_num=get_segment_portal_number(s, p->steps[i].from_portal);
        int to_num=get_segment_portal_number(s, p->steps[i].to_portal);
        length+=s->portal_to_portal_distances[from_num][to_num];
    }
    
    // add 1 for all of the portals traversed (num of steps - 1)
    length+=(p->num_steps-1);
    return length;
}

void find_best_path(universe * u, path * best_path)
{
    best_path->num_steps=-1;
    path current_path;
    current_path.num_steps=0;
    current_path.steps[0].segment=u->start_segment;
    strncpy(current_path.steps[0].from_portal, START_PORTAL, PORTAL_LENGTH+1);
    recursive_work_segments(u, &current_path, best_path);
}

void depth_find_best_path(universe * u, path * best_path)
{
    best_path->num_steps=-1;
    path * current_path = (path *)malloc(sizeof(path));
    current_path->num_steps=0;
    current_path->steps[0].segment=u->start_segment;
    current_path->steps[0].depth=0;
    current_path->is_complete=0;
    current_path->is_terminated=0;
    strncpy(current_path->steps[0].from_portal, START_PORTAL, PORTAL_LENGTH+1);
    int num_to_eval;
    do
    {
        num_to_eval=breadth_work_path_list(u, current_path, best_path);
    }
    while (num_to_eval > 0);
    
    path * next;
    while (current_path != NULL)
    {
        next=current_path->next;
        free(current_path);
        current_path=next;
    }
}

void dupe_path(path * to, path * from)
{
    to->num_steps=from->num_steps;
    for (int i=0; i<MAX_STEPS; i++)
    {
        to->steps[i].segment=from->steps[i].segment;
        to->steps[i].depth=from->steps[i].depth;
        strncpy(to->steps[i].from_portal, from->steps[i].from_portal, PORTAL_LENGTH+1);
        strncpy(to->steps[i].to_portal, from->steps[i].to_portal, PORTAL_LENGTH+1);
    }
    to->is_complete=from->is_complete;
    to->is_terminated=from->is_terminated;
    to->next=NULL;
}

void print_path(path * p)
{
    printf("%d steps ", p->num_steps);
    for (int i=0; i<p->num_steps; i++)
    {
        printf("segment %c from %s to %s ", p->steps[i].segment->label, p->steps[i].from_portal, p->steps[i].to_portal);
        if (i!=p->num_steps-1)
            printf("warp to ");
    }
    printf("\n");
}

void depth_print_path(path * p)
{
    printf("%d steps ", p->num_steps);
    for (int i=0; i<p->num_steps; i++)
    {
        printf("segment %c at depth %d from %s to %s ", p->steps[i].segment->label, p->steps[i].depth, p->steps[i].from_portal, p->steps[i].to_portal);
        if (i!=p->num_steps-1)
            printf("warp to ");
    }
    printf("\n");
}

void recursive_work_segments(universe * u, path * current_path, path * best_path)
{
    step * current_step=&current_path->steps[current_path->num_steps];
    step * next_step=&current_path->steps[current_path->num_steps+1];
    segment * s=current_step->segment;
    //printf("working through %d options in %c\n", s->num_portals, s->label);
    for (int i=0; i<s->num_portals; i++)
    {
        //printf("checking portal %s in %c\n", s->portals[i], s->label);
        if (strncmp(current_step->from_portal, s->portals[i], PORTAL_LENGTH) == 0) // skip the current portal`
        {
            printf("skipping current portal %s\n", current_step->from_portal);
            continue;
        }
        int should_use=1;
        // make sure to not go back through a portal we've been through
        for (int j=0; j<current_path->num_steps; j++)
        {
            if (strncmp(s->portals[i], current_path->steps[j].from_portal, PORTAL_LENGTH) == 0)
            {
                should_use=0;
            }
        }
        if (should_use==0)
        {
            printf("skipping already used protal %s\n", s->portals[i]);
            continue;
        }
            
        strncpy(current_step->to_portal, s->portals[i], PORTAL_LENGTH+1);
        current_path->num_steps++;
        if (is_complete_path(current_path))
        {
            int length=calculate_path_length(current_path);
            printf("Path: ");
            print_path(current_path);
            printf("  has length %d\n", length);
            if (best_path->num_steps == -1 || length < calculate_path_length(best_path))
            {
                printf("This is the new best path\n");
                best_path->num_steps=current_path->num_steps;
                for (int j=0; j<current_path->num_steps; j++)
                {
                    best_path->steps[j].segment=current_path->steps[j].segment;
                    strncpy(best_path->steps[j].from_portal, current_path->steps[j].from_portal, PORTAL_LENGTH+1);
                    strncpy(best_path->steps[j].to_portal, current_path->steps[j].to_portal, PORTAL_LENGTH+1);
                }
            }
        }
        else
        {
            strncpy(next_step->from_portal, s->portals[i], PORTAL_LENGTH+1);
            int universe_portal_number=get_add_portal_number(u, s->portals[i]);
            if(current_step->segment==u->portal_segments[universe_portal_number][0])
                next_step->segment=u->portal_segments[universe_portal_number][1];
            else
                next_step->segment=u->portal_segments[universe_portal_number][0];
            printf("recursively working from path: ");
            print_path(current_path);
            recursive_work_segments(u, current_path, best_path);
        }
        
        current_path->num_steps--;
    }    
}

int is_in_worse_loop(path * p)
{
    step * current_step=&p->steps[p->num_steps-1];
    step * prior_match=NULL;
    int prior_step_number=0;
    step * prior_prior_match=NULL;
    int prior_prior_step_number=0;
    for (int i=p->num_steps-2; i>=0; i--)
    {
        if ((strncmp(current_step->from_portal, p->steps[i].from_portal, PORTAL_LENGTH) == 0) &&
            (strncmp(current_step->to_portal, p->steps[i].to_portal, PORTAL_LENGTH) == 0))
        {
            if (prior_match==NULL)
            {
                prior_match=&p->steps[i];
                prior_step_number=i;
            }
            else if (prior_prior_match==NULL)
            {
                prior_prior_match=&p->steps[i];
                prior_prior_step_number=i;
                break; // done with our loop when both have been set
            }
        }
    }
    if (prior_prior_match==NULL)
        return 0;
    printf("checking for loop - step %d goes from %s to %s at depth %d; step %d goes from %s to %s at depth %d; current step %d goes from %s to %s at depth %d\n",
            prior_prior_step_number, prior_prior_match->from_portal, prior_prior_match->to_portal, prior_prior_match->depth,
            prior_step_number, prior_match->from_portal, prior_match->to_portal, prior_match->depth,
            p->num_steps-1, current_step->from_portal, current_step->to_portal, current_step->depth);
    
    // short circuit this
    
    if (prior_prior_match->depth <= prior_match->depth && prior_match->depth <= current_step->depth)
    {
        printf("This keeps getting deeper. Return that it is a loop.\n");
        return 1;
    }
    return 0;
    
    
    int is_worse_loop=1;
    int step_offset=prior_step_number-(p->num_steps-1); // should be negative
    
    for (int i=p->num_steps-1; i>=prior_step_number; i--)
    {
        int prior_idx=i;
        int prior_prior_idx=prior_idx+step_offset;
        if ((strncmp(p->steps[prior_idx].from_portal, p->steps[prior_prior_idx].from_portal, PORTAL_LENGTH) == 0) &&
            (strncmp(p->steps[prior_idx].to_portal, p->steps[prior_prior_idx].to_portal, PORTAL_LENGTH) == 0))
        {
            if (p->steps[prior_idx].depth<=p->steps[prior_prior_idx].depth)
            {
                printf("not a loop - getting closer to surface. step %d goes from %s to %s at depth %d. step %d goes from %s to %s at depth %d\n",
                       prior_prior_idx, p->steps[prior_prior_idx].from_portal, p->steps[prior_prior_idx].to_portal, p->steps[prior_prior_idx].depth,
                       prior_idx, p->steps[prior_idx].from_portal, p->steps[prior_idx].to_portal, p->steps[prior_idx].depth);
                is_worse_loop=0;
                break;
            }
                
        }
        else
        {
            printf("not a loop - portal mismatch. step %d goes from %s to %s. step %d goes from %s to %s\n",
                   prior_prior_idx, p->steps[prior_prior_idx].from_portal, p->steps[prior_prior_idx].to_portal,
                   prior_idx, p->steps[prior_idx].from_portal, p->steps[prior_idx].to_portal);
            is_worse_loop=0;
            break;
        }
    }
    if (is_worse_loop==1)
    {
        printf("After checking, worse loop is found\n");
    }
    return is_worse_loop;
}

void depth_recursive_work_segments(universe * u, path * current_path, path * best_path)
{
    step * current_step=&current_path->steps[current_path->num_steps];
    step * next_step=&current_path->steps[current_path->num_steps+1];
    segment * s=current_step->segment;
    printf("working through %d options in %c\n", s->num_portals, s->label);
    for (int i=0; i<s->num_portals; i++)
    {
        int direction=0;
        int universe_portal_number=get_add_portal_number(u, s->portals[i]);
        if(current_step->segment==u->portal_segments[universe_portal_number][0])
            direction=u->portal_directions[universe_portal_number][0];
        else
            direction=u->portal_directions[universe_portal_number][1];
        int next_depth=current_step->depth;
        if (direction==OUTSIDE)
        {
            printf("checking outside portal %s in %c.\n", s->portals[i], s->label);
            next_depth-=1;
        }
        else
        {
            next_depth+=1;
            printf("checking inside portal %s in %c.\n", s->portals[i], s->label);
        }
        
        // depth non-zero - skip the start and end portals
        if (current_step->depth !=0 &&
            (strncmp(s->portals[i], START_PORTAL, PORTAL_LENGTH) == 0 ||
             strncmp(s->portals[i], END_PORTAL, PORTAL_LENGTH) == 0))
        {
            printf("Skipping portal %s at non-zero depth\n", s->portals[i]);
            continue;
        }
        
        // depth zero - skip any outside portals other than start and end
        if (current_step->depth == 0)
        {
            if ((strncmp(s->portals[i], START_PORTAL, PORTAL_LENGTH) != 0 &&
                strncmp(s->portals[i], END_PORTAL, PORTAL_LENGTH) != 0) &&
                direction==OUTSIDE)
            {
                printf("Skipping outside portal %s at depth 0\n", s->portals[i]);
                continue;
            }
        }
            
        if (strncmp(current_step->from_portal, s->portals[i], PORTAL_LENGTH) == 0) // skip the current portal`
        {
            printf("skipping current portal %s\n", current_step->from_portal);
            continue;
        }
        //int should_use=1;
        //// make sure to not go back through a portal at the same depth we've been through
        //for (int j=0; j<current_path->num_steps; j++)
        //{
        //    if ((strncmp(s->portals[i], current_path->steps[j].from_portal, PORTAL_LENGTH) == 0) && (current_path->steps[j].depth==current_step->depth))
        //    {
        //        printf("skipping duplicate step for portal %s at depth %d\n", s->portals[i], current_step->depth);
        //        should_use=0;
        //    }
        //    // make sure not to do the same step twice at the same depth
        //    if (((strncmp(current_path->steps[j].from_portal, current_step->from_portal, PORTAL_LENGTH) == 0) &&
        //        (strncmp(current_path->steps[j].to_portal, s->portals[i], PORTAL_LENGTH) == 0)) &&
        //        current_path->steps[j].depth<=next_depth)
        //    {
        //        printf("skipping duplicate step from %s to %s\n", current_step->from_portal, current_step->to_portal);
        //        should_use=0;
        //    }
        //}
        //if (should_use==0)
        //{
        //    printf("skipping already used portal %s at depth %d\n", s->portals[i], current_step->depth);
        //    continue;
        //}
            
        strncpy(current_step->to_portal, s->portals[i], PORTAL_LENGTH+1);
        current_path->num_steps++;
        if (is_complete_path(current_path))
        {
            int length=calculate_path_length(current_path);
            printf("Path: ");
            depth_print_path(current_path);
            printf("  has length %d\n", length);
            if (best_path->num_steps == -1 || length < calculate_path_length(best_path))
            {
                printf("This is the new best path\n");
                best_path->num_steps=current_path->num_steps;
                for (int j=0; j<current_path->num_steps; j++)
                {
                    best_path->steps[j].segment=current_path->steps[j].segment;
                    strncpy(best_path->steps[j].from_portal, current_path->steps[j].from_portal, PORTAL_LENGTH+1);
                    strncpy(best_path->steps[j].to_portal, current_path->steps[j].to_portal, PORTAL_LENGTH+1);
                    best_path->steps[j].depth=current_path->steps[j].depth;
                }
            }
        }
        else if (is_in_worse_loop(current_path))
        {
            printf("Path: ");
            depth_print_path(current_path);
            printf("  has a loop that is getting worse. Breaking out of this path\n");
        }
        else
        {
            strncpy(next_step->from_portal, s->portals[i], PORTAL_LENGTH+1);
            if(current_step->segment==u->portal_segments[universe_portal_number][0])
            {
                next_step->segment=u->portal_segments[universe_portal_number][1];
                if (u->portal_directions[universe_portal_number][0]==OUTSIDE)
                    next_step->depth=current_step->depth-1;
                else
                    next_step->depth=current_step->depth+1;
            }
            else
            {
                next_step->segment=u->portal_segments[universe_portal_number][0];
                if (u->portal_directions[universe_portal_number][1]==OUTSIDE)
                    next_step->depth=current_step->depth-1;
                else
                    next_step->depth=current_step->depth+1;
            }
            printf("next step depth is %d\n", next_step->depth);
            if (next_step->depth!=-1)
            {
                printf("recursively working from path: ");
                depth_print_path(current_path);
                depth_recursive_work_segments(u, current_path, best_path);
            }
        }
        
        current_path->num_steps--;
    }    
}

int breadth_work_path_list(universe * u, path * head, path * best_path)
{
    path * paths_to_append=NULL;
    path * last_to_append=NULL;
    
    path * current_path=head;
    path * last_path=NULL;
    int paths_to_eval=0;
    while (current_path!=NULL)
    {
        last_path=current_path;
        if (current_path->is_complete==1)
        {
            printf("Skipping already complete path %08X\n", current_path);
            current_path=current_path->next;
            continue;
        }
        else if (current_path->is_terminated==1)
        {
            printf("Skipping already terminated path %08X\n", current_path);
            current_path=current_path->next;
            continue;
        }
        
        printf("Evaluating path %08X: ", current_path);
        depth_print_path(current_path);
        current_path->length=calculate_path_length(current_path);
        if (best_path->num_steps > -1 && best_path->length<current_path->length) // already have a best path with a shorter length. done with this one
        {
            printf("  terminating this path. Has length %d, while best_path has lesser length %d\n", current_path->length, best_path->length);
            current_path->is_terminated=1;
            current_path=current_path->next;
            continue;
        }
        if (is_complete_path(current_path))
        {
            printf("  COMPLETE path detected. Has length %d\n", current_path->length);
            current_path->is_complete=1;
            if (best_path->num_steps == -1)
            {
                dupe_path(best_path, current_path);
                printf("    No existing best path. Setting this path to best path with length %d\n", best_path->length);
            }
            else if (current_path->length < best_path->length)
            {
                printf("    This path has length %d which is better than the best path length %d. Replacing best path with this one.\n", current_path->length, best_path->length);
                dupe_path(best_path, current_path);
            }
        }
        
        // at this point, we have a non-complete path. Need to make all of the next paths.
        step * current_step=&current_path->steps[current_path->num_steps];
        segment * s=current_step->segment;
        char next_steps[MAX_PORTALS_PER_SEGMENT][PORTAL_LENGTH+1];
        path * next_paths[MAX_PORTALS_PER_SEGMENT];
        int num_next_steps=0;
        for (int i=0; i<s->num_portals; i++)
        {
            int direction=0;
            int universe_portal_number=get_add_portal_number(u, s->portals[i]);
            if(current_step->segment==u->portal_segments[universe_portal_number][0])
                direction=u->portal_directions[universe_portal_number][0];
            else
                direction=u->portal_directions[universe_portal_number][1];

            if (current_step->depth !=0 &&
                (strncmp(s->portals[i], START_PORTAL, PORTAL_LENGTH) == 0 ||
                strncmp(s->portals[i], END_PORTAL, PORTAL_LENGTH) == 0))
            {
                printf("    Skipping possible next portal %s at non-zero depth\n", s->portals[i]);
                continue;
            }
            if (current_step->depth == 0)
            {
                if ((strncmp(s->portals[i], START_PORTAL, PORTAL_LENGTH) != 0 &&
                    strncmp(s->portals[i], END_PORTAL, PORTAL_LENGTH) != 0) &&
                    direction==OUTSIDE)
                {
                    printf("    Skipping possible next outside portal %s at depth 0\n", s->portals[i]);
                    continue;
                }
            }
            if (strncmp(current_step->from_portal, s->portals[i], PORTAL_LENGTH) == 0) // skip the current portal`
            {
                printf("    Skipping current portal %s\n", s->portals[i]);
                continue;
            }
            strncpy(next_steps[num_next_steps], s->portals[i], PORTAL_LENGTH+1);
            if (num_next_steps==0)
                next_paths[0]=current_path;
            else
            {
                next_paths[num_next_steps]=(path *)malloc(sizeof(path));
                printf("Set next_paths[%d] to new path %08X\n", num_next_steps, next_paths[num_next_steps]);
                dupe_path(next_paths[num_next_steps], next_paths[0]);
                if (paths_to_append==NULL)
                {
                    printf("setting paths_to_append to %08X\n", next_paths[num_next_steps]);
                    paths_to_append=next_paths[num_next_steps];
                }
                if (last_to_append!=NULL)
                {
                    printf("setting last_to_append->next to %08X\n", next_paths[num_next_steps]);
                    last_to_append->next=next_paths[num_next_steps];
                }
                printf("setting last_to_append to %08X\n", next_paths[num_next_steps]);
                last_to_append=next_paths[num_next_steps];
            }
            
            int next_depth=current_step->depth;
            if (direction==OUTSIDE)
            {
                next_depth-=1;
                printf("    setting path %08X with outside portal %s in %c at depth %d.\n", next_paths[num_next_steps], next_steps[num_next_steps], s->label, next_depth);
            }
            else
            {
                next_depth+=1;
                printf("    setting path %08X with inside portal %s in %c at depth %d.\n", next_paths[num_next_steps], next_steps[num_next_steps], s->label, next_depth);
            }
            
            strncpy(next_paths[num_next_steps]->steps[next_paths[num_next_steps]->num_steps].to_portal, next_steps[num_next_steps], PORTAL_LENGTH+1);
            
            next_paths[num_next_steps]->num_steps++;
            strncpy(next_paths[num_next_steps]->steps[next_paths[num_next_steps]->num_steps].from_portal, next_steps[num_next_steps], PORTAL_LENGTH+1);
            next_paths[num_next_steps]->steps[next_paths[num_next_steps]->num_steps].depth=next_depth;

            if(current_step->segment==u->portal_segments[universe_portal_number][0])
                next_paths[num_next_steps]->steps[next_paths[num_next_steps]->num_steps].segment=u->portal_segments[universe_portal_number][1];
            else
                next_paths[num_next_steps]->steps[next_paths[num_next_steps]->num_steps].segment=u->portal_segments[universe_portal_number][0];

            num_next_steps++;
            paths_to_eval++;
        }
        printf("    There are %d possible next steps in %c\n", num_next_steps, s->label);
        if (num_next_steps==0)
        {
            printf("    Setting as terminated since we cannot do anything\n");
            current_path->is_terminated=1;
            current_path=current_path->next;
            continue;
        }
        current_path=current_path->next;
    }
    last_path->next=paths_to_append;
    printf("There are %d paths to evaluate in the next round\n", paths_to_eval);
    return paths_to_eval;
}


