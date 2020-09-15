#include <stdio.h>
#include <stdlib.h>
#include <cnn.h>
#include <iostream>
#include <string.h>
#include <random>

/*
	实现cnn卷积池化的过程
	输入数据：图片数据, 卷积核
	输出数据：经填充、卷积、池化后的数据
	author: loyoung
	create time: 2020/09/12
	version:0.1
*/

// 给指针数组分配内存
int** fun_malloc(int h, int w) {
	int i;
	int** inputMap = (int**)malloc(h * sizeof(int*));
	for (i = 0; i < h; i++)
		inputMap[i] = (int*)malloc(w * sizeof(int));
	return inputMap;
}

// same卷积过程
// inputMap:需要卷积的图像，map:需要卷积的图像的大小，mat:卷积核，matsize：卷积核的大小，padh:上下填充的高度
// 返回值：same卷积运算后的矩阵数据
int** conv_same(int** inputMap, mapsize map, int** mat, mapsize matsize, int padh, int padw)
{
	int i, j, m, n;
	int outputMap_h = map.h - 2 * padh;   
	int outputMap_w = map.w - 2 * padw;
	int matsize_h = matsize.h;   
	int matsize_w = matsize.w;
	int** outputData = fun_malloc(outputMap_h, outputMap_w);

	// same卷积,步长为1，体现在i++和j++
	for (int i = 0; i < map.h - matsize_h + 1; i++) {  
		for (int j = 0; j < map.w - matsize_w + 1; j++) {
			int temp = 0;
			for (int k = 0; k < matsize_h; k++) {
				for (int l = 0; l < matsize_w; l++) {
					temp += inputMap[i + k][j + l] * mat[k][l];  // 要翻转180度，未翻转。。
				}
			}
			outputData[i][j] = temp;
		}
	}
	return outputData;
}


// 给二维矩阵填充，增加addw大小的0值边,填充的长宽,padh是高，padw是宽
int** matpadding(int** inputMap, mapsize matSize, int padh, int padw)
{ 
	int i, j;
	int h = matSize.h;
	int w = matSize.w;
	int** res = (int**)malloc((h + 2 * padh) * sizeof(int*)); 
	for (i = 0; i < (h + 2 * padh); i++)
		res[i] = (int*)malloc((w + 2 * padw) * sizeof(int));
	for (i = 0; i < h + 2 * padh; i++) {
		for (j = 0; j < w + 2 * padw; j++) {
			if (i < padh || j < padw || i >= (h + padh) || j >= (w + padw))
				res[i][j] = 0;
			else
				res[i][j] = inputMap[i - padh][j - padw]; 
		}
	}
	return res;
}


// valid卷积
// inputMap为输入的图像数据；mat为卷积核；返回值为输出矩阵，二维指针数组
// 返回值：valid卷积计算后的矩阵
int** conv_valid(int** inputMap, mapsize inputMapSize, int** mat, mapsize matSize) {
	int inputh = inputMapSize.h;
	int inputw = inputMapSize.w;
	int math = matSize.h;
	int matw = matSize.w;
	int** outputData = (int**)malloc(inputh * sizeof(int*));   
	for (int i = 0; i < inputh; i++)
		outputData[i] = (int*)malloc(inputw * sizeof(int));
	// valid卷积,步长为1，体现在i++和j++
	for (int i = 0; i < inputh - math + 1; i++) {  
		for (int j = 0; j < inputw - matw + 1; j++) {   
			int temp = 0;
			for (int k = 0; k < math; k++) {
				for (int l = 0; l < matw; l++) {
					temp += inputMap[i + k][j + l] * mat[k][l];  // 要翻转180度，未翻转。。
				}
			}
			outputData[i][j] = temp;
		}
	}
	return outputData;
}


// 最大池化
// inputmap为输入需要池化的数据，p为池化窗口的大小，map为需要池化的矩阵的长宽,s为池化步长，返回值为池化后的矩阵
// 返回值：池化后的矩阵
int** maxPooling(int** inputMap, mapsize inputMapsize, int p, int s)
{
	int i, j, m, n;
	int inputmap_h, inputmap_w;
	inputmap_h = inputMapsize.h;	
	inputmap_w = inputMapsize.w;
	int outputData_h = (inputmap_h - p) / s + 1;	
	int outputData_w = (inputmap_w - p) / s + 1;
	int** outputData = (int**)malloc(outputData_h * sizeof(int*)); 
	for (i = 0; i < outputData_h; i++)
		outputData[i] = (int*)malloc(outputData_w * sizeof(int));
	for (i = 0; i < inputmap_h; i = i + s) 
	{
		for (j = 0; j < inputmap_w; j = j + s)
		{
			int maxnum = inputMap[i][j]; 
			for (m = 0; m < p; m++)
				for (n = 0; n < p; n++)
					if ((maxnum < inputMap[i + m][j + n]) && (i + m < inputmap_h) && (j + n < inputmap_w))
						maxnum = inputMap[i + m][j + n];
			outputData[(i / s)][(j / s)] = maxnum; 
		}
	}
	return outputData;
}



int main() {
	mapsize inputmapsize;  // 输入图像
	mapsize matsize;	// 卷积核
	mapsize mapPaddingsize;  // 填充后的矩阵大小
	mapsize outputMapsize;  // 卷积后的矩阵大小
	mapsize maxPoolingMapsize;	// 最大池化的图像大小

	mapsize mapPaddingsize_seclay;	// 第二次卷积的填充
	mapsize outputMapsize_seclay;	// 第二次卷积后的矩阵
	mapsize maxPoolingMapsize_seclay;	// 第二次池化的矩阵
	// 数据初始化
	int p = 2, s = 2, padh = 1, padw = 1;	// p池化窗口大小，s为池化的步长, 填充
	// 输入图像和卷积核的大小
	//scanf("%d",&map.h);
	//scanf("%d",&map.w);
	//scanf("%d",&mat.h);
	//scanf("%d",&mat.w);
	inputmapsize.h = 16;  // 输入图像的大小
	inputmapsize.w = 16;
	matsize.h = 3;  // 卷积核的大小
	matsize.w = 3;
	mapPaddingsize.h = inputmapsize.h + 2 * padh;  // 填充
	mapPaddingsize.w = inputmapsize.w + 2 * padw;
	outputMapsize.h = (inputmapsize.h + 2 * padh - matsize.h) / 1 + 1;  
	outputMapsize.w = (inputmapsize.w + 2 * padw - matsize.w) / 1 + 1; 
	maxPoolingMapsize.h = (outputMapsize.h - p) / s + 1;
	maxPoolingMapsize.w = (outputMapsize.w - p) / s + 1;
	
	// 第二次卷积和池化
	mapPaddingsize_seclay.h = maxPoolingMapsize.h + 2 * padh;
	mapPaddingsize_seclay.w = maxPoolingMapsize.w + 2 * padw;
	outputMapsize_seclay.h = (maxPoolingMapsize.h + 2 * padh - matsize.h) / 1 + 1;
	outputMapsize_seclay.w = (maxPoolingMapsize.w + 2 * padw - matsize.w) / 1 + 1;
	maxPoolingMapsize_seclay.h = (outputMapsize_seclay.h - p) / s + 1;
	maxPoolingMapsize_seclay.w = (outputMapsize_seclay.w - p) / s + 1;

	int i, j;
	// 分配内存
	int** inputMapData = fun_malloc(inputmapsize.h, inputmapsize.w);	
	int** matData = fun_malloc(matsize.h, matsize.w);	
	int** inputMapPaddingData = fun_malloc(mapPaddingsize.h, mapPaddingsize.w);	
	int** outputMapData = fun_malloc(outputMapsize.h, outputMapsize.w);	
	int** maxPoolingMapData = fun_malloc(maxPoolingMapsize.h, maxPoolingMapsize.w);	// 池化
	// 第二次卷积和池化
	int** inputMapPaddingData_seclay = fun_malloc(mapPaddingsize_seclay.h, mapPaddingsize_seclay.w);	// 填充
	int** outputMapData_seclay = fun_malloc(outputMapsize_seclay.h, outputMapsize_seclay.w);	// 卷积后的输出矩阵
	int** maxPoolingMapData_seclay = fun_malloc(maxPoolingMapsize_seclay.h, maxPoolingMapsize_seclay.w);

	// 图像和卷积核初始化
	//printf("请输入需要运算的矩阵：\n");
	for (i = 0; i < inputmapsize.h; i++)
	{
		for (j = 0; j < inputmapsize.w; j++)
		{
			//scanf_s("%d", &inputMap[i][j]);
			inputMapData[i][j] = rand() % 3 + 1;
		}
	}
	printf("输入的图像矩阵为：\n");
	for (i = 0; i < inputmapsize.h; i++)
	{
		for (j = 0; j < inputmapsize.w; j++)
		{
			printf("%d ", inputMapData[i][j]);
		}
		printf("\n");
	}
	//printf("输入卷积核：\n");
	for (i = 0; i < matsize.h; i++)
	{
		for (j = 0; j < matsize.w; j++)
		{
			//scanf_s("%d", &matData[i][j]);
			matData[i][j] = rand() % 3;
		}
	}
	printf("卷积核：\n");
	for (i = 0; i < matsize.h; i++)
	{
		for (j = 0; j < matsize.w; j++)
		{
			printf("%d ", matData[i][j]);
		}
		printf("\n");
	}

	inputMapPaddingData = matpadding(inputMapData, inputmapsize, 1, 1);  // 填充
	//outputData = conv_valid(inputMap, map, matData, mat);	// valid卷积
	outputMapData = conv_same(inputMapPaddingData, mapPaddingsize, matData, matsize, 1, 1);	// same卷积
	maxPoolingMapData = maxPooling(outputMapData, outputMapsize, p, s);  // 池化

	// 第二次卷积和池化
	inputMapPaddingData_seclay = matpadding(maxPoolingMapData, maxPoolingMapsize, 1, 1);
	outputMapData_seclay = conv_same(inputMapPaddingData_seclay, mapPaddingsize_seclay, matData, matsize, 1, 1);
	maxPoolingMapData_seclay = maxPooling(outputMapData_seclay, outputMapsize_seclay, p, s);

	printf("\n填充后的输出矩阵为：\n");
	for (i = 0; i < mapPaddingsize.h; i++) {
		for (j = 0; j < mapPaddingsize.w; j++) {
			printf("%d ", inputMapPaddingData[i][j]);
		}
		printf("\n");
	}
	printf("\n第一次卷积后的输出矩阵为：\n");
	for (i = 0; i < outputMapsize.h; i++) {
		for (j = 0; j < outputMapsize.w; j++)
		{
			printf("%4d", outputMapData[i][j]);
		}
		printf("\n");
	}
	printf("池化窗口大小为：%d，\n池化的步长为：%d\n", p, s);
	printf("池化后的输出矩阵为：\n");
	for (i = 0; i < maxPoolingMapsize.h; i++) {
		for (j = 0; j < maxPoolingMapsize.w; j++)
		{
			printf("%4d", maxPoolingMapData[i][j]);
		}
		printf("\n");
	}

	// 第二次卷积池化
	printf("\n第二次卷积后的输出矩阵为：\n");
	for (i = 0; i < outputMapsize_seclay.h; i++) {
		for (j = 0; j < outputMapsize_seclay.w; j++)
		{
			printf("%4d", outputMapData_seclay[i][j]);
		}
		printf("\n");
	}
	printf("池化窗口大小为：%d，\n池化的步长为：%d\n", p, s);
	printf("池化后的输出矩阵为：\n");
	for (i = 0; i < maxPoolingMapsize_seclay.h; i++) {
		for (j = 0; j < maxPoolingMapsize_seclay.w; j++)
		{
			printf("%4d", maxPoolingMapData_seclay[i][j]);
		}
		printf("\n");
	}
	free(inputMapData);
	free(matData);
	free(inputMapPaddingData);
	free(outputMapData);
	free(maxPoolingMapData);
	return 0;
}