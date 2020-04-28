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

void display_map(map * m, int show_segments)
{
    for (int row=0; row<m->num_rows; row++)
    {
        for (int col=0; col<m->num_cols; col++)
        {
            char ch=m->layout[row][col].display_ch;
            if (isPassage(ch) && show_segments==1)
                ch=m->layout[row][col].segment_label;
            printf("%c", ch);
        }
        printf("\n");
    }
}
