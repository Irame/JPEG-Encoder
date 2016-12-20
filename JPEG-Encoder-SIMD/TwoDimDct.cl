void kernel twoDimDct(global float* image, int w)
{
	#define s0 (0.353553385f)
	#define s1 (0.254897773f)
	#define s2 (0.270598054f)
	#define s3 (0.300672442f)
	#define s4 (0.353553385f)
	#define s5 (0.449988097f)
	#define s6 (0.653281510f)
	#define s7 (1.28145778f)

	#define a1 (0.707106769f)
	#define a2 (0.541196108f)
	#define a3 a1
	#define a4 (1.30656302f)
	#define a5 (0.382683426f)
	
	#define BLOCK_SIZE (8)

    int x_block = get_group_id(0);
    int y_block = get_group_id(1);

	int local_idx = get_local_id(0);

	int block_start = mad24(x_block, BLOCK_SIZE, y_block * w * BLOCK_SIZE);

	int row_start = mad24(local_idx, w, block_start);
	int col_start = local_idx + block_start;

	float block[8];
	
	block[0] = image[row_start + 0] + image[row_start + 7];
	block[1] = image[row_start + 1] + image[row_start + 6];
	block[2] = image[row_start + 2] + image[row_start + 5];
	block[3] = image[row_start + 3] + image[row_start + 4];
	block[4] = image[row_start + 3] - image[row_start + 4];
	block[5] = image[row_start + 2] - image[row_start + 5];
	block[6] = image[row_start + 1] - image[row_start + 6];
	block[7] = image[row_start + 0] - image[row_start + 7];

	float temp0 = block[0];
	float temp1 = block[1];
	block[0] =  block[0] + block[3];
	block[1] =  block[1] + block[2];
	block[2] =  temp1    - block[2];
	block[3] =  temp0    - block[3];
	block[4] = -block[4] - block[5];
	block[5] =  block[5] + block[6];
	block[6] =  block[6] + block[7];
	
	temp0 = block[0];
	block[0] = block[0] + block[1];
	block[1] = temp0          - block[1];
	block[2] = block[2] + block[3];

	float tempMinus6plus4 = -(block[4] + block[6]);
	block[2] *= a1;
	block[4] = mad(-block[4], a2, tempMinus6plus4 * a5);
	block[5] *= a3;
	block[6] = mad(block[6], a4, tempMinus6plus4 * a5);

	float temp2 = block[2];
	block[2] += block[3];
	block[3] -= temp2;
	temp2 = block[5];
	block[5] += block[7];
	block[7] -= temp2;

	image[row_start + 0] = block[0] * s0;
	image[row_start + 4] = block[1] * s4;
	image[row_start + 2] = block[2] * s2;
	image[row_start + 6] = block[3] * s6;
	image[row_start + 5] = (block[4] + block[7]) * s5;
	image[row_start + 1] = (block[5] + block[6]) * s1;
	image[row_start + 7] = (block[5] - block[6]) * s7;
	image[row_start + 3] = (block[7] - block[4]) * s3;

	barrier(CLK_GLOBAL_MEM_FENCE);

	block[0] = image[            col_start ] + image[mad24(7, w, col_start)];
	block[1] = image[mad24(1, w, col_start)] + image[mad24(6, w, col_start)];
	block[2] = image[mad24(2, w, col_start)] + image[mad24(5, w, col_start)];
	block[3] = image[mad24(3, w, col_start)] + image[mad24(4, w, col_start)];
	block[4] = image[mad24(3, w, col_start)] - image[mad24(4, w, col_start)];
	block[5] = image[mad24(2, w, col_start)] - image[mad24(5, w, col_start)];
	block[6] = image[mad24(1, w, col_start)] - image[mad24(6, w, col_start)];
	block[7] = image[            col_start ] - image[mad24(7, w, col_start)];

	temp0 = block[0];
	temp1 = block[1];
	block[0] =  block[0] + block[3];
	block[1] =  block[1] + block[2];
	block[2] =  temp1    - block[2];
	block[3] =  temp0    - block[3];
	block[4] = -block[4] - block[5];
	block[5] =  block[5] + block[6];
	block[6] =  block[6] + block[7];
	
	temp0 = block[0];
	block[0] = block[0] + block[1];
	block[1] = temp0          - block[1];
	block[2] = block[2] + block[3];

	tempMinus6plus4 = -(block[4] + block[6]);
	block[2] *= a1;
	block[4] = mad(-block[4], a2, tempMinus6plus4 * a5);
	block[5] *= a3;
	block[6] = mad(block[6], a4, tempMinus6plus4 * a5);

	temp2 = block[2];
	block[2] += block[3];
	block[3] -= temp2;
	temp2 = block[5];
	block[5] += block[7];
	block[7] -= temp2;

	image[            col_start ] = block[0] * s0;
	image[mad24(4, w, col_start)] = block[1] * s4;
	image[mad24(2, w, col_start)] = block[2] * s2;
	image[mad24(6, w, col_start)] = block[3] * s6;
	image[mad24(5, w, col_start)] = (block[4] + block[7]) * s5;
	image[mad24(1, w, col_start)] = (block[5] + block[6]) * s1;
	image[mad24(7, w, col_start)] = (block[5] - block[6]) * s7;
	image[mad24(3, w, col_start)] = (block[7] - block[4]) * s3;

	if (local_idx == 0)
		image[block_start] -= 1024;
}
