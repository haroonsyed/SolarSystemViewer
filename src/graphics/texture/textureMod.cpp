#include "textureMod.h"

#include <cstdio>
#include <string>
#include <iostream>

#include "../../../lib/std_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../../lib/stb_image_write.h"

earthTexture::earthTexture()
{
	std::string earthPath = "../assets/textures/a.jpg";
	std::string	cloudsPath = "../assets/textures/earth_clouds.jpg";

	int width, height = -1;
	int cwidth, cheight = -1;
	int nrChannels;
	int anrChannels;

	unsigned char* data = stbi_load(earthPath.c_str(), &width, &height, &nrChannels, 3);
	unsigned char* clouds = stbi_load(cloudsPath.c_str(), &cwidth, &cheight, &anrChannels, 1);

	unsigned char* newImage = new unsigned char[height * width * 3]();

	for (unsigned char* p = data, *pg = newImage, *c = clouds; p != data + width*height*3; p += 3, pg += 3, c += 1)
	{
		if ((uint8_t)(*c) < 100)
		{
			*pg = (uint8_t)(*p);
			*(pg + 1) = (uint8_t)(*(p + 1));
			*(pg + 2) = (uint8_t)(*(p + 2));
		}
		else
		{
			*pg = (uint8_t)(*c);
			*(pg + 1) = (uint8_t)(*c);
			*(pg + 2) = (uint8_t)(*c);
		}
		//*(pg + 1) = (uint8_t)(*(p + 1));
		//*(pg + 2) = (uint8_t)(*(p + 2));
		/*if ((uint8_t)(*c) < 254)
		{
			*pg = (uint8_t)(*p);
			*(pg+1) = (uint8_t)(*(p+1));
			*(pg+2) = (uint8_t)(*(p+2));
		}
		else
		{
			*pg = (uint8_t)(*c);
			*(pg+1) = (uint8_t)(*c);
			*(pg+2) = (uint8_t)(*c);
		}*/
	}

	/*for (unsigned char* p = data, *pg = newImage; p != data + width*height*3; p += 3, pg += 1) {
		* pg = (uint8_t)((*p + *(p + 1) + *(p + 2)) / 3.0);
	}*/

	stbi_write_jpg("../assets/textures/new2.jpg", width, height, 3, newImage, 100);












	//std::string a = "hello";
	//unsigned char* hel = new unsigned char[1023]();
	//hel[0] = 'a';
	//std::cout << hel << std::endl;

	//int s = strlen((char*)data);
	//std::cout << s << std::endl;
	//std::cout << height * width * 1 << std::endl;

	//int ** array = new int*[4096];

	//for (int i = 0; i < 4096; ++i)
	//{
	//	array[i] = new int[8192*4];
	//}
	//

	//for (int i = 0; i < 4096; ++i)
	//{
	//	for (int j = 0; j < 8192*4; j+=4)
	//	{
	//		int index = j + 8192*4 * i;
	//		//set transparency of first rgb value
	//		array[i][j] = (uint8_t)data[index];

	//		//set value of red
	//		array[i][j+1] = (uint8_t)data[index];
	//		std::cout << array[i][j+1] << std::endl;

	//		//set value of green
	//		array[i][j+2] = (uint8_t)data[j + (8192 * 4 * i)];

	//		//set value of blue
	//		array[i][j+3] = (uint8_t)data[j + (8192 * 4 * i)];

	//	
	//	}
	//}



}
