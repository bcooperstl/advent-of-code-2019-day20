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
