typedef struct {
	u32	scaler_type;
	u16	numTables;
	u16	searchRange;
	u16	entrySelector;
	u16	rangeShift;
} offset_subtable;


typedef struct {
	u16 platformID;
	u16 platformSpecificID;
	u32 offset;
} cmap_encoding_subtable;


typedef struct {
	u16 version;
	u16 numberSubtables;
	cmap_encoding_subtable* subtables;
} cmap;

typedef struct {
	u16  format;
 	u16  length;
 	u16  language;
 	u16  segCountX2;
 	u16  searchRange;
 	u16  entrySelector;
 	u16  rangeShift;
	u16  reservedPad;
	u16  *endCode;
	u16  *startCode;
	u16  *idDelta;
	u16  *idRangeOffset;
	u16  *glyphIdArray;
} format4;

typedef struct {
	union { 
		char tag_c[4];
		u32	tag;
	};
	u32	checkSum;
	u32	offset;
	u32	length;
} table_directory;

typedef struct  {
	offset_subtable off_sub;
	table_directory* tbl_dir;
	format4* f4;
	cmap* cmap;
	char* glyf;
	char* loca;
	char* head;
} font_directory; 

typedef union {
	typedef struct {
		u8 on_curve: 1;
		
		u8 x_short: 1;
		u8 y_short: 1;
		
		u8 repeat: 1;
		
		u8 x_short_pos: 1;
		u8 y_short_pos: 1;
		
		u8 reserved1: 1;
		u8 reserved2: 1;
	};
	u8 flag;
} glyph_flag;


typedef struct {
	u16 numberOfContours;
	i16 xMin;
	i16 yMin;
	i16 xMax;
	i16 yMax;
	u16 instructionLength;
	u8* instructions;
	glyph_flag* flags;
	i16* xCoordinates;
	i16* yCoordinates;
	u16* endPtsOfContours;
} glyph_outline;



void read_offset_subtable(char** mem, offset_subtable* off_sub) {
	char* m = *mem;
	off_sub->scaler_type = READ_BE32_MOVE(m);
	off_sub->numTables = READ_BE16_MOVE(m);
	off_sub->searchRange = READ_BE16_MOVE(m);
	off_sub->entrySelector = READ_BE16_MOVE(m);
	off_sub->rangeShift = READ_BE16_MOVE(m);

	*mem = m;
}


void read_cmap(char* mem, cmap* c) {
	char *m = mem;
	c->version = READ_BE16_MOVE(m);
	c->numberSubtables = READ_BE16_MOVE(m);

	c->subtables = (cmap_encoding_subtable*) calloc(1, sizeof(cmap_encoding_subtable)*c->numberSubtables);
	for(int i = 0; i < c->numberSubtables; ++i) {
		cmap_encoding_subtable* est = c->subtables + i;
		est->platformID = READ_BE16_MOVE(m);
		est->platformSpecificID = READ_BE16_MOVE(m);
		est->offset = READ_BE32_MOVE(m);
	}

}

void print_cmap(cmap* c) {
	printf("#)\tpId\tpsID\toffset\ttype\n");
	for(int i = 0; i < c->numberSubtables; ++i) {
		cmap_encoding_subtable* cet = c->subtables + i;
		printf("%d)\t%d\t%d\t%d\t", i+1, cet->platformID, cet->platformSpecificID, cet->offset);
		switch(cet->platformID) {
			case 0: printf("Unicode"); break;
			case 1: printf("Mac"); break;
			case 2: printf("Not Supported"); break;
			case 3: printf("Microsoft"); break;
		}
		printf("\n");
	}
}

void read_format4(char* mem, format4** format) {
	char* m = mem;

	u16 length = READ_BE16(m + 2);

	format4* f = NULL;

	f = (format4*) calloc(1, length + sizeof(u16*)*5);
	f->format = READ_BE16_MOVE(m);
	f->length = READ_BE16_MOVE(m);
	f->language = READ_BE16_MOVE(m);
	f->segCountX2 = READ_BE16_MOVE(m);
	f->searchRange = READ_BE16_MOVE(m);
	f->entrySelector = READ_BE16_MOVE(m);
	f->rangeShift = READ_BE16_MOVE(m);

	f->endCode = (u16*) ((u8*)f  + sizeof(format4));
	f->startCode = f->endCode + f->segCountX2/2;
	f->idDelta = f->startCode + f->segCountX2/2;
	f->idRangeOffset = f->idDelta + f->segCountX2/2;
	f->glyphIdArray = f->idRangeOffset + f->segCountX2/2;

	char* start_code_start = m + f->segCountX2 + 2;
	char* id_delta_start = m + f->segCountX2*2 + 2;
	char* id_range_start = m + f->segCountX2*3 + 2;

	for(int i = 0; i < f->segCountX2/2; ++i) {
		f->endCode[i] = READ_BE16(m + i*2);
		f->startCode[i] = READ_BE16(start_code_start + i*2);
		f->idDelta[i] = READ_BE16(id_delta_start + i*2);
		f->idRangeOffset[i] = READ_BE16(id_range_start + i*2);
	}

	P_MOVE(m, f->segCountX2*4 + 2);	

	int remaining_bytes = f->length - (m - mem);
	for(int i = 0; i < remaining_bytes/2; ++i) {
		f->glyphIdArray[i] = READ_BE16_MOVE(m);
	}

	*format = f;
}

void print_format4(format4 *f4) {
	printf("Format: %d, Length: %d, Language: %d, Segment Count: %d\n", f4->format, f4->length, f4->language, f4->segCountX2/2);
	printf("Search Params: (searchRange: %d, entrySelector: %d, rangeShift: %d)\n",
			f4->searchRange, f4->entrySelector, f4->rangeShift);
	printf("Segment Ranges:\tstartCode\tendCode\tidDelta\tidRangeOffset\n");
	for(int i = 0; i < f4->segCountX2/2; ++i) {
		printf("--------------:\t% 9d\t% 7d\t% 7d\t% 12d\n", f4->startCode[i], f4->endCode[i], f4->idDelta[i], f4->idRangeOffset[i]);
	}
}


void read_table_directory(char* file_start, char** mem, font_directory* ft) {
	char* m = *mem;
	ft->tbl_dir = (table_directory*)calloc(1, sizeof(table_directory)*ft->off_sub.numTables);

	for(int i = 0; i < ft->off_sub.numTables; ++i) {
		table_directory* t = ft->tbl_dir + i;
		t->tag = READ_BE32_MOVE(m);
		t->checkSum = READ_BE32_MOVE(m);
		t->offset = READ_BE32_MOVE(m);
		t->length = READ_BE32_MOVE(m);

		switch(t->tag) {
			case GLYF_TAG: ft->glyf = t->offset + file_start; break;
			case LOCA_TAG: ft->loca = t->offset + file_start; break;
			case HEAD_TAG: ft->head = t->offset + file_start; break;
			case CMAP_TAG: {
				ft->cmap = (cmap*) calloc(1, sizeof(cmap));
				read_cmap(file_start + t->offset, ft->cmap);
				read_format4(file_start + t->offset + ft->cmap->subtables[0].offset, &ft->f4);
			} break;
		}
	}

	*mem = m;
}

void print_table_directory(table_directory* tbl_dir, int tbl_size) {
	printf("#)\ttag\tlen\toffset\n");
	for(int i = 0; i < tbl_size; ++i) {
		table_directory* t = tbl_dir + i;
		printf("%d)\t%c%c%c%c\t%d\t%d\n", i+1,
				t->tag_c[3], t->tag_c[2],
				t->tag_c[1], t->tag_c[0],
				t->length, t->offset);
	}
}

void read_font_directory(char* file_start, char** mem, font_directory* ft) {
	read_offset_subtable(mem, &ft->off_sub); 
	read_table_directory(file_start, mem, ft);
}


int get_glyph_index(font_directory* ft, u16 code_point) {
	format4 *f = ft->f4;
	int index = -1;
	u16 *ptr = NULL;
	for(int i = 0; i < f->segCountX2/2; i++) {
		if(f->endCode[i] > code_point) {index = i; break;};
	}
	
	if(index == -1) return 0;

	if(f->startCode[index] < code_point) {
		if(f->idRangeOffset[index] != 0) {
			ptr = f->idRangeOffset + index + f->idRangeOffset[index]/2;
			ptr += code_point - f->startCode[index];
			if(*ptr == 0) return 0;
			return *ptr + f->idDelta[index];
		} else {
			return code_point + f->idDelta[index];
		}
	}

	return 0;
}

int read_loca_type(font_directory* ft) {
	return READ_BE16(ft->head + 50);
}

u32 get_glyph_offset(font_directory *ft, u32 glyph_index) {
	u32 offset = 0;
	if(read_loca_type(ft)) {
		//32 bit
		offset = READ_BE32((u32*)ft->loca + glyph_index);
	} else {
		offset =  READ_BE16((u16*)ft->loca + glyph_index)*2;
	}
	return offset;
}


glyph_outline get_glyph_outline(font_directory* ft, u32 glyph_index) {
	u32 offset = get_glyph_offset(ft, glyph_index);
	unsigned char* glyph_start = ft->glyf + offset;
	glyph_outline outline = {0};
	outline.numberOfContours = READ_BE16_MOVE(glyph_start);
	outline.xMin = READ_BE16_MOVE(glyph_start);
	outline.yMin = READ_BE16_MOVE(glyph_start);
	outline.xMax = READ_BE16_MOVE(glyph_start);
	outline.yMax = READ_BE16_MOVE(glyph_start);

	outline.endPtsOfContours = (u16*) calloc(1, outline.numberOfContours*sizeof(u16));
	for(int i = 0; i < outline.numberOfContours; ++i) {
		outline.endPtsOfContours[i] = READ_BE16_MOVE(glyph_start);
	}

	outline.instructionLength = READ_BE16_MOVE(glyph_start);
	outline.instructions = (u8*)calloc(1, outline.instructionLength);
	memcpy(outline.instructions, glyph_start, outline.instructionLength);
	P_MOVE(glyph_start, outline.instructionLength);

	int last_index = outline.endPtsOfContours[outline.numberOfContours-1];
	outline.flags = (glyph_flag*) calloc(1, last_index + 1);

	for(int i = 0; i < (last_index + 1); ++i) {
		outline.flags[i].flag = *glyph_start;
		glyph_start++;
		if(outline.flags[i].repeat) {
			u8 repeat_count = *glyph_start;
			while(repeat_count-- > 0) {
				i++;
				outline.flags[i] = outline.flags[i-1];
			}
			glyph_start++;
		}
	}


	outline.xCoordinates = (i16*) calloc(1, (last_index+1)*2);
	i16 prev_coordinate = 0;
	i16 current_coordinate = 0;
	for(int i = 0; i < (last_index+1); ++i) {
		int flag_combined = outline.flags[i].x_short << 1 | outline.flags[i].x_short_pos;
		switch(flag_combined) {
			case 0: {
				current_coordinate = READ_BE16_MOVE(glyph_start);
			} break;
			case 1: { current_coordinate = 0; }break;
			case 2: { current_coordinate = (*glyph_start++)*-1; }break;
			case 3: { current_coordinate = (*glyph_start++); } break;
		}

		outline.xCoordinates[i] = current_coordinate + prev_coordinate;
		prev_coordinate = outline.xCoordinates[i];
	}

	outline.yCoordinates = (i16*) calloc(1, (last_index+1)*2);
	current_coordinate = 0;
	prev_coordinate = 0;
	for(int i = 0; i < (last_index+1); ++i) {
		int flag_combined = outline.flags[i].y_short << 1 | outline.flags[i].y_short_pos;
		switch(flag_combined) {
			case 0: {
				current_coordinate = READ_BE16_MOVE(glyph_start);
			} break;
			case 1: { current_coordinate = 0; }break;
			case 2: { current_coordinate = (*glyph_start++)*-1; }break;
			case 3: { current_coordinate = (*glyph_start++); } break;
		}

		outline.yCoordinates[i] = current_coordinate + prev_coordinate;
		prev_coordinate = outline.yCoordinates[i];
	}

	return outline;
}

void print_glyph_outline(glyph_outline *outline) {
	printf("#contours\t(xMin,yMin)\t(xMax,yMax)\tinst_length\n");
	printf("%9d\t(%d,%d)\t\t(%d,%d)\t%d\n", outline->numberOfContours,
			outline->xMin, outline->yMin,
			outline->xMax, outline->yMax,
			outline->instructionLength);

	printf("#)\t(  x  ,  y  )\n");
	int last_index = outline->endPtsOfContours[outline->numberOfContours-1];
	for(int i = 0; i <= last_index; ++i) {
		printf("%d)\t(%5d,%5d)\n", i, outline->xCoordinates[i], outline->yCoordinates[i]);
	}
}

typedef struct point2 {
	float x,y;
} point2;

typedef struct color {
	float r,g,b,a;	
} color;

typedef struct line {
	point2 p1, p2;
} line;

typedef struct rect {
	i32 x,y,width,height;
} rect;

void tessellate_bezier(point2 *output, i32 *output_size, point2 p0, point2 p1, point2 p2) {
    i32 subdiv_into = 2;
    f32 step_per_iter = 1.0/subdiv_into;
    i32 out_size = 0;
    for(int i = 1; i <= subdiv_into; i++) {
      f32 t = i*step_per_iter;
      f32 t1 = (1.0 - t);
      f32 t2 = t*t;
      f32 x = t1*t1*p0.x + 2*t1*t*p1.x + t2*p2.x;
      f32 y = t1*t1*p0.y + 2*t1*t*p1.y + t2*p2.y;
      output[out_size].x = x;
      output[out_size].y = y;
      out_size++;
    }
    
    *output_size = out_size;
 }

void generate_points(glyph_outline *outline, point2 *generated_points, i32 capacity, i32 *point_count, i32* gen_pts_end_indices) {
	i32 j = 0;
	i32 index = 0;
	for(i32 i = 0; i < outline->numberOfContours; i++) {
		i32 contour_start_index = j;
		i32 generated_points_start_index = index;
		i32 contour_start = 1;
		i32 contour_started_off = 0;
		for(;j <= outline->endPtsOfContours[i]; j++) {
			glyph_flag *flag = outline->flags + j;

			i32 x = outline->xCoordinates[j];
			i32 y = outline->yCoordinates[j];

			i32 contour_len = outline->endPtsOfContours[i] - contour_start_index + 1;
			i32 cur_index = j;
			i32 next_index = (j+1 - contour_start_index)%contour_len + contour_start_index;

			if(flag->on_curve) {
				//point on curve so add directly 
				generated_points[index].x = x;
				generated_points[index].y = y;
				index++;
			} else {
				// handle off curve

				if(contour_start) {
					contour_started_off = 1;
					if(outline->flags[next_index].on_curve) {
						generated_points[index].x = outline->xCoordinates[next_index];
						generated_points[index].y = outline->yCoordinates[next_index];
						index++;
						j++;
						continue;
					}

					x = x + (outline->xCoordinates[next_index] - x)/2.0;
					y = y + (outline->yCoordinates[next_index] - y)/2.0;

					generated_points[index].x = x;
					generated_points[index].y = y;
					index++;
				}

				point2 p0 = generated_points[index - 1]; // the latest generated point
				point2 p1 = {.x = x, .y = y};
				point2 p2 = {.x = outline->xCoordinates[next_index], .y = outline->yCoordinates[next_index] }; //we will need this point either way

				if(!(outline->flags + next_index)->on_curve) { //check next point
					p2.x = p1.x + (p2.x - p1.x)/2.0;
					p2.y = p1.y + (p2.y - p1.y)/2.0;
				} else {
					j++;
				}

				i32 out_size = 0;
				tessellate_bezier(generated_points + index, &out_size, p0, p1, p2);
				index += out_size;
			}

			contour_start = 0;
		}

		if(outline->flags[j-1].on_curve) {
			generated_points[index] = generated_points[generated_points_start_index];
			index++;
		}

		if(contour_started_off) {
			//handle first point if its off
			point2 p0 = generated_points[index-1]; //our latest added point, this will before the first point
			point2 p1 = {.x = outline->xCoordinates[contour_start_index], .y = outline->yCoordinates[contour_start_index]};
			point2 p2 = generated_points[generated_points_start_index];

			i32 out_size = 0;
			tessellate_bezier(generated_points + index, &out_size, p0, p1, p2);
			index += out_size;
		}


		gen_pts_end_indices[i] = index;

	}

	*point_count = index;
}

line* generate_edges(point2 *pts_gen, i32 *edge_count, i32 *contour_ends, i32 contour_count) {
    i32 j = 0;
    i32 edge_index = 0;
    line *edges = (line*) malloc(sizeof(line)*contour_ends[contour_count-1]);
    for(i32 i = 0; i < contour_count; i++) {
      for(; j < contour_ends[i]-1; j++) {
        line *edge = edges + edge_index;
        edge->p1.x = pts_gen[j].x;
        edge->p1.y = pts_gen[j].y;
        
        edge->p2.x = pts_gen[j+1].x;
        edge->p2.y = pts_gen[j+1].y;
        edge_index++;
      }
      j++; //we are the end of the contour, skip over this point to the start of the next contour
    }
    
    *edge_count = edge_index;

	return edges;
}

i32 cmp_floats(const void *a, const void *b) {
	f32 f_a = *(f32 *)a;
	f32 f_b = *(f32 *)b;

	if(f_a < f_b) return -1;
	else if (f_a > f_b) return 1;

	return 0;
}

void rasterize_glyph(line *edges, i32 edge_count, u8 *bitmap, i32 bitmap_height, i32 bitmap_width) {

	f32 intersections[32] = {0};
	i32 intersection_count = 0;
	i32 scanline_subdiv = 5;
	f32 alpha_weight = 255.0/scanline_subdiv;
	f32 step_per_scanline = 1.0/5;

	for(i32 i = 0; i < bitmap_height; i++) {

		for(i32 x = 0; x < scanline_subdiv; x++) {
			intersection_count = 0;

			f32 scanline = i + x*step_per_scanline;

			for(i32 j = 0; j < edge_count; j++) {
				line *edge = edges + j;

				f32 bigger_y = MAX(edge->p1.y, edge->p2.y);
				f32 smaller_y = MIN(edge->p1.y, edge->p2.y);

				if(scanline <= smaller_y) continue;
				if(scanline > bigger_y) continue;

				f32 dx = edge->p2.x - edge->p1.x;
				f32 dy = edge->p2.y - edge->p1.y;

				if(dy == 0) continue;


				f32 intersection = -1;
				if(dx == 0) {
					intersection = edge->p1.x;
				} else {
					intersection = (scanline - edge->p1.y)*(dx/dy) + edge->p1.x;
					if(intersection < 0) intersection = edge->p1.x;
				}

				intersections[intersection_count] = intersection;
				intersection_count++;
			}


			qsort(intersections, intersection_count, sizeof(f32), cmp_floats);


			if(intersection_count > 1) {
				for(i32 m = 0; m < intersection_count; m += 2) {
					f32 start_intersection = intersections[m];
					i32 start_index = intersections[m];
					f32 start_covered = (start_index + 1) - start_intersection ;

					f32 end_intersection = intersections[m+1];
					i32 end_index = intersections[m+1];
					f32 end_covered = end_intersection - (end_index);

					if(start_index == end_index) {
						bitmap[start_index + i*bitmap_width] += alpha_weight*start_covered;
					} else {
						bitmap[start_index + i*bitmap_width] += alpha_weight*start_covered;
						bitmap[end_index + i*bitmap_width] += alpha_weight*end_covered;
					}

					for(i32 j = start_index+1; j < end_index; j++) {
						bitmap[j + i*bitmap_width] += alpha_weight;	
					}
				}
			}
		}
	}
}

void draw_texture(rect r, i32 vbo, u8 *texture_data, i32 tex_id, i32 width, i32 height) {
	point2 quad[12] = {
		//triangle 1
		{r.x, r.y},
		{r.x, r.y + r.height},
		{r.x + r.width, r.y + r.height},

		//triangle 2
		{r.x, r.y},
		{r.x + r.width, r.y + r.height},
		{r.x + r.width, r.y},

		// UV coordinates for triangle 1
		{0, 0},
		{0, 1},
		{1, 1},
		// UV coordinates for triangle 2
		{0, 0},
		{1, 1},
		{1, 0}
	};

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(point2)*12, quad, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void *)( sizeof(point2)*6 ));

	glBindTexture(GL_TEXTURE_2D, tex_id);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, texture_data);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void render_glyph(u8* temp_texture, i32 bitmap_width, i32 bitmap_height, font_directory ft, i32 vbo, i32 tex_id) {
	glyph_outline glyph = get_glyph_outline(&ft, get_glyph_index(&ft, '0'));
	point2 temp_points[512] = { 0};

	f32 scale = (f32)bitmap_height/(f32)(glyph.yMax - glyph.yMin);

	i32 index = 0;
	i32 *contour_end_pts = (i32 *) malloc(sizeof(i32)*glyph.numberOfContours);
	generate_points(&glyph, temp_points, 128, &index, contour_end_pts);

	for(int i = 0; i < index; i++) {
		temp_points[i].x = ( temp_points[i].x  - glyph.xMin)*scale;
		temp_points[i].y = ( temp_points[i].y  - glyph.yMin)*scale;
	}


	i32 edge_count = 0;
	line *edges = generate_edges(temp_points, &edge_count, contour_end_pts, glyph.numberOfContours);

	rasterize_glyph(edges, edge_count, temp_texture, bitmap_height, bitmap_width);

	rect r = {
		250, 250, 64, 64
	};

	draw_texture(r,vbo, temp_texture,tex_id, bitmap_width, bitmap_height);

}
