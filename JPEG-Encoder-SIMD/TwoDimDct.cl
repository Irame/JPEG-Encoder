void kernel twoDimDct(global float* image, int w)
{
	#define s0 0.353553385;
	#define s1 0.254897773;
	#define s2 0.270598054;
	#define s3 0.300672442;
	#define s4 0.353553385;
	#define s5 0.449988097;
	#define s6 0.653281510;
	#define s7 1.28145778;

	#define a1 0.707106769;
	#define a2 0.541196108;
	#define a3 a1;
	#define a4 1.30656302;
	#define a5 0.382683426;

	int x = get_global_id(0)*8;
	int y = get_global_id(1)*8;

	for (size_t row = y*w; row < y + 8 * w; row += w) {
		float temp1[8];
		temp1[0] = image[row + x + 0] + image[row + x + 7];
		temp1[1] = image[row + x + 1] + image[row + x + 6];
		temp1[2] = image[row + x + 2] + image[row + x + 5];
		temp1[3] = image[row + x + 3] + image[row + x + 4];
		temp1[4] = image[row + x + 3] - image[row + x + 4];
		temp1[5] = image[row + x + 2] - image[row + x + 5];
		temp1[6] = image[row + x + 1] - image[row + x + 6];
		temp1[7] = image[row + x + 0] - image[row + x + 7];

		float temp2[8];
		temp2[0] = temp1[0] + temp1[3];
		temp2[1] = temp1[1] + temp1[2];
		temp2[2] = temp1[1] - temp1[2];
		temp2[3] = temp1[0] - temp1[3];
		temp2[4] = -temp1[4] - temp1[5];
		temp2[5] = temp1[5] + temp1[6];
		temp2[6] = temp1[6] + temp1[7];
		temp2[7] = temp1[7];

		float temp3[8];
		temp3[0] = temp2[0] + temp2[1];
		temp3[1] = temp2[0] - temp2[1];
		temp3[2] = temp2[2] + temp2[3];
		temp3[3] = temp2[3];
		temp3[4] = temp2[4];
		temp3[5] = temp2[5];
		temp3[6] = temp2[6];
		temp3[7] = temp2[7];


		float temp6plus4 = temp3[4] + temp3[6];
		temp3[2] *= a1;
		temp3[4] = -temp3[4] * a2 - temp6plus4 * a5;
		temp3[5] *= a3;
		temp3[6] = temp3[6] * a4 - temp6plus4 * a5;

		float temp = temp3[2];
		temp3[2] += temp3[3];
		temp3[3] -= temp;
		temp = temp3[5];
		temp3[5] += temp3[7];
		temp3[7] -= temp;

		image[row + x + 0] = temp3[0] * s0;
		image[row + x + 4] = temp3[1] * s4;
		image[row + x + 2] = temp3[2] * s2;
		image[row + x + 6] = temp3[3] * s6;
		image[row + x + 5] = (temp3[4] + temp3[7]) * s5;
		image[row + x + 1] = (temp3[5] + temp3[6]) * s1;
		image[row + x + 7] = (temp3[5] - temp3[6]) * s7;
		image[row + x + 3] = (temp3[7] - temp3[4]) * s3;
	}


	for (size_t col = y; col < x + 8; col++) {
		float temp1[8];
		temp1[0] = image[col + 0 * w] + image[col + 7 * w];
		temp1[1] = image[col + 1 * w] + image[col + 6 * w];
		temp1[2] = image[col + 2 * w] + image[col + 5 * w];
		temp1[3] = image[col + 3 * w] + image[col + 4 * w];
		temp1[4] = image[col + 3 * w] - image[col + 4 * w];
		temp1[5] = image[col + 2 * w] - image[col + 5 * w];
		temp1[6] = image[col + 1 * w] - image[col + 6 * w];
		temp1[7] = image[col + 0 * w] - image[col + 7 * w];

		float temp2[8];
		temp2[0] = temp1[0] + temp1[3];
		temp2[1] = temp1[1] + temp1[2];
		temp2[2] = temp1[1] - temp1[2];
		temp2[3] = temp1[0] - temp1[3];
		temp2[4] = -temp1[4] - temp1[5];
		temp2[5] = temp1[5] + temp1[6];
		temp2[6] = temp1[6] + temp1[7];
		temp2[7] = temp1[7];

		float temp3[8];
		temp3[0] = temp2[0] + temp2[1];
		temp3[1] = temp2[0] - temp2[1];
		temp3[2] = temp2[2] + temp2[3];
		temp3[3] = temp2[3];
		temp3[4] = temp2[4];
		temp3[5] = temp2[5];
		temp3[6] = temp2[6];
		temp3[7] = temp2[7];


		float temp6plus4 = temp3[4] + temp3[6];
		temp3[2] *= a1;
		temp3[4] = -temp3[4] * a2 - temp6plus4 * a5;
		temp3[5] *= a3;
		temp3[6] = temp3[6] * a4 - temp6plus4 * a5;

		float temp = temp3[2];
		temp3[2] += temp3[3];
		temp3[3] -= temp;
		temp = temp3[5];
		temp3[5] += temp3[7];
		temp3[7] -= temp;

		image[col + 0 * w] = temp3[0] * s0;
		image[col + 4 * w] = temp3[1] * s4;
		image[col + 2 * w] = temp3[2] * s2;
		image[col + 6 * w] = temp3[3] * s6;
		image[col + 5 * w] = (temp3[4] + temp3[7]) * s5;
		image[col + 1 * w] = (temp3[5] + temp3[6]) * s1;
		image[col + 7 * w] = (temp3[5] - temp3[6]) * s7;
		image[col + 3 * w] = (temp3[7] - temp3[4]) * s3;
	}


	// https://en.wikipedia.org/wiki/JPEG#Discrete_cosine_transform
	// subtract 1024 from the DC coefficient, which is mathematically equivalent
	// to center the provided data around zero.
	image[x + y*w] -= 1024;
}
