#ifndef BORDER_QUADRA_SCORE_H
#define BORDER_QUADRA_SCORE_H

int lineScore[3][30];
int anglScore[3][30];
int spaceScore[3][30];
int areaScore[3][30];
int scoreCur[3] = { 0, 0, 0 };
vector<int> tLineScore;
vector<double> tAnglScore;
vector<int> tAreaScore;
vector<double> tSpaceScore;
int curphase = 0;
int topRank[90];
int finalRank[30];
int spaceRank[30];
int angleRank[30];
int spaceRankDic[30];
int angleRankDic[30];

int compareAngleScore(const void * a, const void * b) {
	int ai = *(int*) a;
	int bi = *(int*) b;

	int scorea = tAnglScore[topRank[ai]];
	int scoreb = tAnglScore[topRank[bi]];

	return scorea - scoreb;
}

int compareSpaceScore(const void * a, const void * b) {
	int ai = *(int*) a;
	int bi = *(int*) b;

	int scorea = tSpaceScore[topRank[ai]];
	int scoreb = tSpaceScore[topRank[bi]];

	return scoreb - scorea;
}

int compareFinalScore(const void * a, const void * b) {
	double weight = 3;
	//return 0;
	int ai = *(int*) a;
	int bi = *(int*) b;

	double scorea = spaceRankDic[ai] * (1.0+tAnglScore[topRank[ai]]);
	double scoreb = spaceRankDic[bi] * (1.0+tAnglScore[topRank[bi]]);

	if(scorea<scoreb) return -1;
	if(scorea>scoreb) return 1;
	return 0;
}

int compareTopScore(const void * a, const void * b) {
	int ai = *(int*) a;
	int bi = *(int*) b;
	//Currently nonsense
//	if (tAreaScore[ai] < tAreaScore[bi])
//		return 1;
//	if (tAreaScore[ai] > tAreaScore[bi])
//		return -1;

	//only compare line scores
	if (tLineScore[ai] < tLineScore[bi])
		return 1;
	if (tLineScore[ai] > tLineScore[bi])
		return -1;

	return 0;
}

int calcAreaScore(float p, float r) {
	if (p == 0 || r == 0)
		return 0;
	double fv = 2 * p * r / (p + r);
	int fvi = (int) (fv * 100);
	return fvi;
}

double myAngleScore(vector<double> lineAngles, double a1, double a2, double a3, double a4) {

	double ZEROTHRESH = CV_PI/36;
	if(angleToHori(lineAngles[0])<ZEROTHRESH&&angleToHori(lineAngles[1])<ZEROTHRESH){
		if(angleToVert(lineAngles[2])<ZEROTHRESH&&angleToVert(lineAngles[3])<ZEROTHRESH)
			return 0.0;
		else
			return fabs(angleToVert(lineAngles[2])-angleToVert(lineAngles[3]));
	}

	if(angleToVert(lineAngles[2])<ZEROTHRESH&&angleToVert(lineAngles[3])<ZEROTHRESH)
		return fabs(angleToHori(lineAngles[0])-angleToHori(lineAngles[1]));

	return fabs(angleToVert(lineAngles[2])-angleToVert(lineAngles[3]))+fabs(angleToHori(lineAngles[0])-angleToHori(lineAngles[1]))+fabs(angleToHori(lineAngles[0]));
}

#endif
