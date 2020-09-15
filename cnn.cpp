#include <stdio.h>
#include <stdlib.h>
#include <cnn.h>
#include <iostream>
#include <string.h>
#include <random>

/*
	ʵ��cnn����ػ��Ĺ���
	�������ݣ�ͼƬ����, �����
	������ݣ�����䡢������ػ��������
	author: loyoung
	create time: 2020/09/12
	version:0.1
*/

// ��ָ����������ڴ�
int** fun_malloc(int h, int w) {
	int i;
	int** inputMap = (int**)malloc(h * sizeof(int*));
	for (i = 0; i < h; i++)
		inputMap[i] = (int*)malloc(w * sizeof(int));
	return inputMap;
}

// same�������
// inputMap:��Ҫ�����ͼ��map:��Ҫ�����ͼ��Ĵ�С��mat:����ˣ�matsize������˵Ĵ�С��padh:�������ĸ߶�
// ����ֵ��same��������ľ�������
int** conv_same(int** inputMap, mapsize map, int** mat, mapsize matsize, int padh, int padw)
{
	int i, j, m, n;
	int outputMap_h = map.h - 2 * padh;   
	int outputMap_w = map.w - 2 * padw;
	int matsize_h = matsize.h;   
	int matsize_w = matsize.w;
	int** outputData = fun_malloc(outputMap_h, outputMap_w);

	// same���,����Ϊ1��������i++��j++
	for (int i = 0; i < map.h - matsize_h + 1; i++) {  
		for (int j = 0; j < map.w - matsize_w + 1; j++) {
			int temp = 0;
			for (int k = 0; k < matsize_h; k++) {
				for (int l = 0; l < matsize_w; l++) {
					temp += inputMap[i + k][j + l] * mat[k][l];  // Ҫ��ת180�ȣ�δ��ת����
				}
			}
			outputData[i][j] = temp;
		}
	}
	return outputData;
}


// ����ά������䣬����addw��С��0ֵ��,���ĳ���,padh�Ǹߣ�padw�ǿ�
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


// valid���
// inputMapΪ�����ͼ�����ݣ�matΪ����ˣ�����ֵΪ������󣬶�άָ������
// ����ֵ��valid��������ľ���
int** conv_valid(int** inputMap, mapsize inputMapSize, int** mat, mapsize matSize) {
	int inputh = inputMapSize.h;
	int inputw = inputMapSize.w;
	int math = matSize.h;
	int matw = matSize.w;
	int** outputData = (int**)malloc(inputh * sizeof(int*));   
	for (int i = 0; i < inputh; i++)
		outputData[i] = (int*)malloc(inputw * sizeof(int));
	// valid���,����Ϊ1��������i++��j++
	for (int i = 0; i < inputh - math + 1; i++) {  
		for (int j = 0; j < inputw - matw + 1; j++) {   
			int temp = 0;
			for (int k = 0; k < math; k++) {
				for (int l = 0; l < matw; l++) {
					temp += inputMap[i + k][j + l] * mat[k][l];  // Ҫ��ת180�ȣ�δ��ת����
				}
			}
			outputData[i][j] = temp;
		}
	}
	return outputData;
}


// ���ػ�
// inputmapΪ������Ҫ�ػ������ݣ�pΪ�ػ����ڵĴ�С��mapΪ��Ҫ�ػ��ľ���ĳ���,sΪ�ػ�����������ֵΪ�ػ���ľ���
// ����ֵ���ػ���ľ���
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
	mapsize inputmapsize;  // ����ͼ��
	mapsize matsize;	// �����
	mapsize mapPaddingsize;  // ����ľ����С
	mapsize outputMapsize;  // �����ľ����С
	mapsize maxPoolingMapsize;	// ���ػ���ͼ���С

	mapsize mapPaddingsize_seclay;	// �ڶ��ξ�������
	mapsize outputMapsize_seclay;	// �ڶ��ξ����ľ���
	mapsize maxPoolingMapsize_seclay;	// �ڶ��γػ��ľ���
	// ���ݳ�ʼ��
	int p = 2, s = 2, padh = 1, padw = 1;	// p�ػ����ڴ�С��sΪ�ػ��Ĳ���, ���
	// ����ͼ��;���˵Ĵ�С
	//scanf("%d",&map.h);
	//scanf("%d",&map.w);
	//scanf("%d",&mat.h);
	//scanf("%d",&mat.w);
	inputmapsize.h = 16;  // ����ͼ��Ĵ�С
	inputmapsize.w = 16;
	matsize.h = 3;  // ����˵Ĵ�С
	matsize.w = 3;
	mapPaddingsize.h = inputmapsize.h + 2 * padh;  // ���
	mapPaddingsize.w = inputmapsize.w + 2 * padw;
	outputMapsize.h = (inputmapsize.h + 2 * padh - matsize.h) / 1 + 1;  
	outputMapsize.w = (inputmapsize.w + 2 * padw - matsize.w) / 1 + 1; 
	maxPoolingMapsize.h = (outputMapsize.h - p) / s + 1;
	maxPoolingMapsize.w = (outputMapsize.w - p) / s + 1;
	
	// �ڶ��ξ���ͳػ�
	mapPaddingsize_seclay.h = maxPoolingMapsize.h + 2 * padh;
	mapPaddingsize_seclay.w = maxPoolingMapsize.w + 2 * padw;
	outputMapsize_seclay.h = (maxPoolingMapsize.h + 2 * padh - matsize.h) / 1 + 1;
	outputMapsize_seclay.w = (maxPoolingMapsize.w + 2 * padw - matsize.w) / 1 + 1;
	maxPoolingMapsize_seclay.h = (outputMapsize_seclay.h - p) / s + 1;
	maxPoolingMapsize_seclay.w = (outputMapsize_seclay.w - p) / s + 1;

	int i, j;
	// �����ڴ�
	int** inputMapData = fun_malloc(inputmapsize.h, inputmapsize.w);	
	int** matData = fun_malloc(matsize.h, matsize.w);	
	int** inputMapPaddingData = fun_malloc(mapPaddingsize.h, mapPaddingsize.w);	
	int** outputMapData = fun_malloc(outputMapsize.h, outputMapsize.w);	
	int** maxPoolingMapData = fun_malloc(maxPoolingMapsize.h, maxPoolingMapsize.w);	// �ػ�
	// �ڶ��ξ���ͳػ�
	int** inputMapPaddingData_seclay = fun_malloc(mapPaddingsize_seclay.h, mapPaddingsize_seclay.w);	// ���
	int** outputMapData_seclay = fun_malloc(outputMapsize_seclay.h, outputMapsize_seclay.w);	// �������������
	int** maxPoolingMapData_seclay = fun_malloc(maxPoolingMapsize_seclay.h, maxPoolingMapsize_seclay.w);

	// ͼ��;���˳�ʼ��
	//printf("��������Ҫ����ľ���\n");
	for (i = 0; i < inputmapsize.h; i++)
	{
		for (j = 0; j < inputmapsize.w; j++)
		{
			//scanf_s("%d", &inputMap[i][j]);
			inputMapData[i][j] = rand() % 3 + 1;
		}
	}
	printf("�����ͼ�����Ϊ��\n");
	for (i = 0; i < inputmapsize.h; i++)
	{
		for (j = 0; j < inputmapsize.w; j++)
		{
			printf("%d ", inputMapData[i][j]);
		}
		printf("\n");
	}
	//printf("�������ˣ�\n");
	for (i = 0; i < matsize.h; i++)
	{
		for (j = 0; j < matsize.w; j++)
		{
			//scanf_s("%d", &matData[i][j]);
			matData[i][j] = rand() % 3;
		}
	}
	printf("����ˣ�\n");
	for (i = 0; i < matsize.h; i++)
	{
		for (j = 0; j < matsize.w; j++)
		{
			printf("%d ", matData[i][j]);
		}
		printf("\n");
	}

	inputMapPaddingData = matpadding(inputMapData, inputmapsize, 1, 1);  // ���
	//outputData = conv_valid(inputMap, map, matData, mat);	// valid���
	outputMapData = conv_same(inputMapPaddingData, mapPaddingsize, matData, matsize, 1, 1);	// same���
	maxPoolingMapData = maxPooling(outputMapData, outputMapsize, p, s);  // �ػ�

	// �ڶ��ξ���ͳػ�
	inputMapPaddingData_seclay = matpadding(maxPoolingMapData, maxPoolingMapsize, 1, 1);
	outputMapData_seclay = conv_same(inputMapPaddingData_seclay, mapPaddingsize_seclay, matData, matsize, 1, 1);
	maxPoolingMapData_seclay = maxPooling(outputMapData_seclay, outputMapsize_seclay, p, s);

	printf("\n������������Ϊ��\n");
	for (i = 0; i < mapPaddingsize.h; i++) {
		for (j = 0; j < mapPaddingsize.w; j++) {
			printf("%d ", inputMapPaddingData[i][j]);
		}
		printf("\n");
	}
	printf("\n��һ�ξ������������Ϊ��\n");
	for (i = 0; i < outputMapsize.h; i++) {
		for (j = 0; j < outputMapsize.w; j++)
		{
			printf("%4d", outputMapData[i][j]);
		}
		printf("\n");
	}
	printf("�ػ����ڴ�СΪ��%d��\n�ػ��Ĳ���Ϊ��%d\n", p, s);
	printf("�ػ�����������Ϊ��\n");
	for (i = 0; i < maxPoolingMapsize.h; i++) {
		for (j = 0; j < maxPoolingMapsize.w; j++)
		{
			printf("%4d", maxPoolingMapData[i][j]);
		}
		printf("\n");
	}

	// �ڶ��ξ���ػ�
	printf("\n�ڶ��ξ������������Ϊ��\n");
	for (i = 0; i < outputMapsize_seclay.h; i++) {
		for (j = 0; j < outputMapsize_seclay.w; j++)
		{
			printf("%4d", outputMapData_seclay[i][j]);
		}
		printf("\n");
	}
	printf("�ػ����ڴ�СΪ��%d��\n�ػ��Ĳ���Ϊ��%d\n", p, s);
	printf("�ػ�����������Ϊ��\n");
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