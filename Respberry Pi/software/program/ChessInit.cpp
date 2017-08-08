#include<stdio.h>
#include<unistd.h>
#include<cv.h>
#include<highgui.h>
using namespace cv;
struct Chess{
	short state[13][13];
	Point left[13][13];
	Point right[13][13];
}chess;
void chessInit(char* left, char* right);
void filled(int x, int y, int &sum, int &xsum, int &ysum, Mat &image);
double distance(Point p1, Point p2);
bool findNerbor(Point* point, int psum, bool *used, int &a, int &b, int ppdispra);
bool isOnLine(Point* pline, int plnum, Point p, double pldispra);
void makeChess(Point* point, int psum, Mat &image, bool lfrt);
int getChessPoint(Mat image, bool lfrt);
void writeToFile();
void showLeft(Mat old);
void showRight(Mat old);
void show(char* left, char* right);
int main(){
	//	system("rm -rf ../source/left0.jpg ../source/right0.jpg");
	//	system("../campic0.sh");
	char left[] = "../source/left0.jpg";
	char right[] = "../source/right0.jpg";
	if (access(left, R_OK) == 0 && access(right, R_OK) == 0){
		chessInit(left, right);
		show(left, right);
		writeToFile();
		return 0;
	}
	return 1;
}
void chessInit(char* left, char* right){
	int ksize = 3, blockSize = 10;
	int k;
	Mat leftGray = imread(left, CV_8U);
	Mat leftCornerStrength;
	Mat se(6, 6, CV_8U, Scalar(1));
	do{
		cornerHarris(leftGray, leftCornerStrength, blockSize, ksize, 0.05);
		threshold(leftCornerStrength, leftCornerStrength, 0.002, 255, THRESH_BINARY);
		imwrite("../source/outputLeft.jpg", leftCornerStrength);
		Mat imageLeft = imread("../source/outputLeft.jpg", CV_8U);
		//��ͼ����������
		Mat openedLeft;
		morphologyEx(imageLeft, openedLeft, MORPH_OPEN, se);
		k = getChessPoint(openedLeft, true);
		if (k >= 200){
			if (k > 2){
				ksize -= 2;
			}
			else if (blockSize > 0){
				blockSize -= 2;
			}
			else{
				exit(2);
			}
		}
		else if (k < 169){
			if (ksize < 8){
				ksize += 2;
			}
			else{
				blockSize += 2;
			}
		}
		else{
			break;
		}
	} while (true);
	ksize = 3, blockSize = 10;
	Mat rightGray = imread(right, CV_8U);
	Mat rightCornerStrength;
	do{
		cornerHarris(rightGray, rightCornerStrength, blockSize, ksize, 0.05);
		threshold(rightCornerStrength, rightCornerStrength, 0.002, 255, THRESH_BINARY);
		imwrite("../source/outputRight.jpg", rightCornerStrength);
		Mat imageRight = imread("../source/outputRight.jpg", CV_8U);
		//��ͼ����������
		Mat openedRight;
		morphologyEx(imageRight, openedRight, MORPH_OPEN, se);
		k = getChessPoint(openedRight, false);
		if (k >= 200){
			if (k > 2){
				ksize -= 2;
			}
			else if (blockSize > 0){
				blockSize -= 2;
			}
			else{
				exit(2);
			}
		}
		else if (k < 169){
			if (ksize < 8){
				ksize += 2;
			}
			else{
				blockSize += 2;
			}
		}
		else{
			break;
		}
	} while (true);
}
void filled(int x, int y, int &sum, int &xsum, int &ysum, Mat &image){
	image.at<uchar>(x, y) = 0;
	xsum += x;
	ysum += y;
	sum++;
	if (!(x == 0 || image.at<uchar>(x - 1, y) < 127))
		filled(x - 1, y, sum, xsum, ysum, image);
	if (!(x == image.rows - 1 || image.at<uchar>(x + 1, y)<127))
		filled(x + 1, y, sum, xsum, ysum, image);
	if (!(y == 0 || image.at<uchar>(x, y - 1)<127))
		filled(x, y - 1, sum, xsum, ysum, image);
	if (!(y == image.cols - 1 || image.at<uchar>(x, y + 1)<127))
		filled(x, y + 1, sum, xsum, ysum, image);
}
double distance(Point p1, Point p2){
	return sqrt((p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y));
}
bool findNerbor(Point* point, int psum, bool *used, int &a, int &b, int ppdispra){	//�ҵ��������ڵĵ�
	int count = 0;	//5�������ڼ�������ھ�
	for (int i = 0; i < psum; i++){
		if (!used[i]){
			for (int j = i + 1; j < psum; j++){
				if (!used[j]){
					if (distance(point[i], point[j]) < ppdispra){
						a = i;
						b = j;
						return true;
					}
					if (count++ == 5){
						break;
					}
				}
			}
		}
	}
	return false;
}
bool isOnLine(Point* pline, int plnum, Point p, double dispra){		//�жϵ�p�Ƿ���pline���ڵ�ֱ����
	//��С���˷����ֱ��y=bx+a
	double a, b, avex, avey;
	int sumx = 0, sumy = 0, sumxy = 0, sumx2 = 0;
	for (int i = 0; i < plnum; i++){
		sumx += pline[i].x;
		sumy += pline[i].y;
		sumxy += pline[i].x*pline[i].y;
		sumx2 += pline[i].x*pline[i].x;
	}
	avex = (double)sumx / plnum;
	avey = (double)sumy / plnum;
	b = (sumxy - plnum*avex*avey) / (sumx2 - plnum*avex*avex);
	a = avey - b*avex;
	//�㵽ֱ�ߵľ���
	double A = b, B = -1, C = a;
	double dis = abs((A*p.x + B*p.y + C)) / sqrt(A*A + B*B);
	if (dis > dispra)
		return false;
	else
		return true;
}
void makeChess(Point* point, int psum, Mat &image, bool lfrt){
	bool* used = (bool*)calloc(psum, sizeof(bool));		//���ʹ��״̬
	double dispra = 0.0;		//�㵽ֱ�ߵľ������
	int ppdispra;			//��������ʱ�ľ������
	Point pline[13];		//��ʱ�洢���ڹ�����һ�е�
	int plnum;				//��¼��ǰ�����еĵ����Ŀ
	int changed[13];		//��¼��������Щ��
	int col = 0;			//��¼�������к�
	while (col < 13){
		ppdispra = 45;
		int a, b;			//�����ҵ�������
		while (!findNerbor(point, psum, used, a, b, ppdispra))
			ppdispra++;
		pline[0] = point[a];
		pline[1] = point[b];
		used[a] = used[b] = true;
		changed[0] = a;
		changed[1] = b;
		//��������ʵĵ�
		bool overflow;
		do{
			plnum = 2;
			overflow = false;
			for (int i = 0; i < psum; i++){
				if (!used[i] && isOnLine(pline, plnum, point[i], dispra)){		//�����������
					if (plnum == 13){
						overflow = true;
						break;
					}
					pline[plnum] = point[i];
					used[i] = true;
					changed[plnum] = i;
					plnum++;
				}
			}
			//�ҵ��ĵ���Ŀ����ȷ�����޸Ĳ���������Ѱ��
			if (overflow){
				dispra -= 0.05;
				for (int i = 2; i < 13; i++){
					used[changed[i]] = false;
				}
			}
			if (plnum < 13){
				dispra += 0.05;
				for (int i = 2; i < plnum; i++){
					used[changed[i]] = false;
				}
			}
		} while (plnum != 13 && !overflow);
		//���ҵ��ĵ�����
		for (int i = 0; i < 12; i++){
			int t = i;
			for (int j = i + 1; j < 13; j++){
				if (pline[t].x > pline[j].x){
					t = j;
				}
			}
			if (t != i){
				Point ptemp = pline[t];
				pline[t] = pline[i];
				pline[i] = ptemp;
			}
		}
		//����col��
		if (lfrt){
			for (int i = 0; i < 13; i++){
				chess.left[col][i] = pline[i];
			}
		}
		else{
			for (int i = 0; i < 13; i++){
				chess.right[col][i] = pline[i];
			}
		}
		col++;
	}
	return;
}
int getChessPoint(Mat image, bool lfrt){
	Point point[200];
	int xsum, ysum, sum;
	int k = 0;
	for (int j = 1; j < image.cols; j++){
		for (int i = 1; i < image.rows; i++){
			if (image.at<uchar>(i, j)>127){
				xsum = 0; ysum = 0; sum = 0;
				filled(i, j, sum, xsum, ysum, image);
				point[k++] = Point(xsum / sum, ysum / sum);
				if (k == 200){
					printf("error\n");
					return k;
				}
			}
		}
	}
	if (k < 169){
		return k;
	}
	makeChess(point, k, image, lfrt);
	return k;
}
void writeToFile(){
	FILE* chessFile = fopen("../source/chessFile.txt", "w");
	for (int i = 0; i < 13; i++){
		for (int j = 0; j < 13; j++){
			fprintf(chessFile, " %d %d %d %d %d\n", chess.state[i][j], chess.left[i][j].x, chess.left[i][j].y, chess.right[i][j].x, chess.right[i][j].y);
		}
	}
}
void show(char* left, char* right){
	Mat imageLeft = imread(left, CV_8U);
	Mat imageRight = imread(right, CV_8U);
	showLeft(imageLeft);
	showRight(imageRight);
	waitKey();
}
void showLeft(Mat old){
	Mat left = Mat(old.size(), CV_8U, Scalar(0));
	for (int i = 0; i < 13; i++){
		for (int j = 0; j < 13; j++){
			left.at<uchar>(chess.left[i][j].x, chess.left[i][j].y) = 255;
			if (j > 0){
				line(left, Point(chess.left[i][j - 1].y, chess.left[i][j - 1].x), Point(chess.left[i][j].y, chess.left[i][j].x), Scalar(255));
				line(left, Point(chess.left[j - 1][i].y, chess.left[j - 1][i].x), Point(chess.left[j][i].y, chess.left[j][i].x), Scalar(255));
			}
		}
	}
	//	imshow("left", left);
	imwrite("../source/outChessLeft.jpg", left);
}
void showRight(Mat old){
	Mat right = Mat(old.size(), CV_8U, Scalar(0));
	for (int i = 0; i < 13; i++){
		for (int j = 0; j < 13; j++){
			right.at<uchar>(chess.right[i][j].x, chess.right[i][j].y) = 255;
			if (j > 0){
				line(right, Point(chess.right[i][j - 1].y, chess.right[i][j - 1].x), Point(chess.right[i][j].y, chess.right[i][j].x), Scalar(255));
				line(right, Point(chess.right[j - 1][i].y, chess.right[j - 1][i].x), Point(chess.right[j][i].y, chess.right[j][i].x), Scalar(255));
			}
		}
	}
	//	imshow("right", right);
	imwrite("../source/outChessRight.jpg", right);
}
