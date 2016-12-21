void kernel quantize(global float* image, int w, constant float* qTable) { 
    #define BLOCK_SIZE (8)
	
	int x_block = get_group_id(0);
    int y_block = get_group_id(1);

	int x_local = get_local_id(0);
	int y_local = get_local_id(1);

	int image_pos = x_block * BLOCK_SIZE + y_block * w * BLOCK_SIZE + x_local + y_local * w;
	int qTable_pos = x_local + y_local * BLOCK_SIZE;

	image[image_pos] = round(image[image_pos] / qTable[qTable_pos]);
}