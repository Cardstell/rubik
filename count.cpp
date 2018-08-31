#include <iostream>
#include <vector>
#include <string>
#include <array>
#include <ctime>
#include <queue>
#include <unistd.h>
#include <thread>
#include <mutex>
using namespace std;

const int N = 4;
const int maxn = 10;
const int queue_size = 10000;
const int threads = 4;

struct cube {
	char F = ' ', B = ' ';
	char L = ' ', R = ' ';
	char U = ' ', D = ' '; 
};

queue<string> q;
string z;
cube cubes[N*N*N];
int mx = 0, thrrun = 0;
mutex locker;

void fillcubes(cube *cubes) {
	for (int i = 0;i<N;++i) {
		for (int j = 0;j<N;++j) {
			for (int h = 0;h<N;++h) {
				int id = i*N*N + j*N + h;
				if (i == 0) cubes[id].U = 'U';
				if (i == N-1) cubes[id].D = 'D';
				if (j == 0) cubes[id].B = 'B';
				if (j == N-1) cubes[id].F = 'F';
				if (h == 0) cubes[id].L = 'L';
				if (h == N-1) cubes[id].R = 'R';
			}
		}
	}
}

void printcubes(cube *cubes) {
	for (int i = 0;i<N;++i) {
		for (int h = 0;h<N+1;++h) cout<<" ";
		for (int j = 0;j<N;++j) cout<<cubes[i*N+j].U;
		cout<<endl;
	}
	cout<<endl;
	for (int i = 0;i<N;++i) {
		for (int j = 0;j<N;++j) cout<<cubes[i*N*N + j*N].L;
		cout<<" ";
		for (int j = 0;j<N;++j) cout<<cubes[i*N*N + j + N*(N-1)].F;
		cout<<" ";
		for (int j = 0;j<N;++j) cout<<cubes[i*N*N + (N-j-1)*N + N-1].R;
		cout<<" ";
		for (int j = 0;j<N;++j) cout<<cubes[i*N*N + (N-j-1)].B;
		cout<<endl;
	}
	cout<<endl;
	for (int i = 0;i<N;++i) {
		for (int h = 0;h<N+1;++h) cout<<" ";
		for (int j = 0;j<N;++j) cout<<cubes[(N-i-1)*N+j+N*N*(N-1)].D;
			cout<<endl;
	}
}

cube rotateCube(cube curcube, char direction) {
	cube newcube;
	switch (direction) {
	case 'U':
		newcube.U = curcube.U;
		newcube.D = curcube.D;
		newcube.L = curcube.F;
		newcube.B = curcube.L;
		newcube.R = curcube.B;
		newcube.F = curcube.R;
		break;
	case 'D':
		newcube.U = curcube.U;
		newcube.D = curcube.D;
		newcube.F = curcube.L;
		newcube.L = curcube.B;
		newcube.B = curcube.R;
		newcube.R = curcube.F;
		break;
	case 'L':
		newcube.L = curcube.L;
		newcube.R = curcube.R;
		newcube.U = curcube.B;
		newcube.B = curcube.D;
		newcube.D = curcube.F;
		newcube.F = curcube.U;
		break;
	case 'R':
		newcube.L = curcube.L;
		newcube.R = curcube.R;
		newcube.B = curcube.U;
		newcube.D = curcube.B;
		newcube.F = curcube.D;
		newcube.U = curcube.F;
		break;
	case 'F':
		newcube.F = curcube.F;
		newcube.B = curcube.B;
		newcube.U = curcube.L;
		newcube.L = curcube.D;
		newcube.D = curcube.R;
		newcube.R = curcube.U;
		break;
	case 'B':
		newcube.F = curcube.F;
		newcube.B = curcube.B;
		newcube.L = curcube.U;
		newcube.D = curcube.L;
		newcube.R = curcube.D;
		newcube.U = curcube.R;
		break;
	}
	return newcube;
}

void copy(cube *cubes, cube *newcubes) {
	for (int i = 0;i<N*N*N;++i) {
		newcubes[i] = cubes[i];
	}
}

void rotate(cube *cubes, string direction) {
	int count = 1;
	if (direction[1] == '2') count = 2;
	else if (direction[1] == '\'') count = 3;
	for (int cnt = 0;cnt<count;++cnt) {
		cube newcubes[N*N*N];
		copy(cubes, newcubes);
		switch (direction[0])
		{
			case 'F':
				for (int i = 0;i<N;++i) {
					for (int j = 0;j<N;++j) {
						int id1 = i*N*N + j + N*(N-1);
						int id2 = (N-i-1) + j*N*N + N*(N-1);
						newcubes[id2] = rotateCube(cubes[id1], direction[0]);
					}
				}
				break;
			case 'f':
				for (int i = 0;i<N;++i) {
					for (int j = 0;j<N;++j) {
						int id1 = i*N*N + j + N*(N-1) - N;
						int id2 = (N-i-1) + j*N*N + N*(N-1) - N;
						newcubes[id2] = rotateCube(cubes[id1], direction[0] + 'A' - 'a');
					}
				}
				break;
			case 'B':
				for (int i = 0;i<N;++i) {
					for (int j = 0;j<N;++j) {
						int id1 = i*N*N + (N-j-1);
						int id2 = j*N*N + i;
						newcubes[id2] = rotateCube(cubes[id1], direction[0]);
					}
				}
				break;
			case 'b':
				for (int i = 0;i<N;++i) {
					for (int j = 0;j<N;++j) {
						int id1 = i*N*N + (N-j-1) + N;
						int id2 = j*N*N + i + N;
						newcubes[id2] = rotateCube(cubes[id1], direction[0] + 'A' - 'a');
					}
				}
				break;
			case 'L':
				for (int i = 0;i<N;++i) {
					for (int j = 0;j<N;++j) {
						int id1 = i*N*N + j*N;
						int id2 = j*N*N + (N-i-1)*N;
						newcubes[id2] = rotateCube(cubes[id1], direction[0]);
					}
				}
				break;
			case 'l':
				for (int i = 0;i<N;++i) {
					for (int j = 0;j<N;++j) {
						int id1 = i*N*N + j*N + 1;
						int id2 = j*N*N + (N-i-1)*N + 1;
						newcubes[id2] = rotateCube(cubes[id1], direction[0] + 'A' - 'a');
					}
				}
				break;
			case 'R':
				for (int i = 0;i<N;++i) {
					for (int j = 0;j<N;++j) {
						int id1 = i*N*N + (N-j-1)*N + N-1;
						int id2 = j*N*N + i*N + N-1;
						newcubes[id2] = rotateCube(cubes[id1], direction[0]);
					}
				}
				break;
			case 'r':
				for (int i = 0;i<N;++i) {
					for (int j = 0;j<N;++j) {
						int id1 = i*N*N + (N-j-1)*N + N-1 - 1;
						int id2 = j*N*N + i*N + N-1 - 1;
						newcubes[id2] = rotateCube(cubes[id1], direction[0] + 'A' - 'a');
					}
				}
				break;
			case 'U':
				for (int i = 0;i<N;++i) {
					for (int j = 0;j<N;++j) {
						int id1 = i*N + j;
						int id2 = j*N + (N-i-1);
						newcubes[id2] = rotateCube(cubes[id1], direction[0]);
					}
				}
				break;
			case 'u':
				for (int i = 0;i<N;++i) {
					for (int j = 0;j<N;++j) {
						int id1 = i*N + j + N*N;
						int id2 = j*N + (N-i-1) + N*N;
						newcubes[id2] = rotateCube(cubes[id1], direction[0] + 'A' - 'a');
					}
				}
				break;
			case 'D':
				for (int i = 0;i<N;++i) {
					for (int j = 0;j<N;++j) {
						int id1 = (N-i-1)*N + j + N*N*(N-1);
						int id2 = (N-j-1)*N + (N-i-1) + N*N*(N-1);
						newcubes[id2] = rotateCube(cubes[id1], direction[0]);
					}
				}
				break;
			case 'd':
				for (int i = 0;i<N;++i) {
					for (int j = 0;j<N;++j) {
						int id1 = (N-i-1)*N + j + N*N*(N-2);
						int id2 = (N-j-1)*N + (N-i-1) + N*N*(N-2);
						newcubes[id2] = rotateCube(cubes[id1], direction[0] + 'A' - 'a');
					}
				}
				break;


		}
		copy(newcubes, cubes);
	}
}

vector<string> split(string s) {
	vector<string> ans;
	int start = 0;
	for (int i = 0;i<s.size();++i) {
		if (s[i] == ' ') {
			string tmp = s.substr(start, i-start);
			if (tmp != "") ans.push_back(tmp);
			start = i + 1;
		}
	}
	if (start != s.size()) {
		string tmp = s.substr(start, s.size()-start);
		if (tmp != "") ans.push_back(tmp);
	}
	return ans;
}

void run(cube *cubes, string algo) {
	vector<string> rotations = split(algo);
	for (string i:rotations) {
		if (i.size() == 1) i += ' ';
		rotate(cubes, i);
	}
}

string exec(const char* cmd) {
    char buffer[128];
    string result = "";
    FILE* pipe = popen(cmd, "r");
    try {
        while (!feof(pipe)) {
            if (fgets(buffer, 128, pipe) != NULL)
                result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}

string getStringCube(cube *cubes) {
	string ans = "";
	for (int i = 0;i<N;++i) {
		for (int j = 0;j<N;++j) ans += cubes[i*N + j].U;
	}
	for (int i = 0;i<N;++i) {
		for (int j = 0;j<N;++j) ans += cubes[i*N*N + (N-j-1)*N + N-1].R;
	}
	for (int i = 0;i<N;++i) {
		for (int j = 0;j<N;++j) ans += cubes[i*N*N + j + N*(N-1)].F;
	}
	for (int i = 0;i<N;++i) {
		for (int j = 0;j<N;++j) ans += cubes[(N-i-1)*N + j + N*N*(N-1)].D;
	}
	for (int i = 0;i<N;++i) {
		for (int j = 0;j<N;++j) ans += cubes[i*N*N + j*N].L;
	}
	for (int i = 0;i<N;++i) {
		for (int j = 0;j<N;++j) ans += cubes[i*N*N + (N-j-1)].B;
	}
	return ans;
}

string generateRandomRotations(int count) {
	string tmp, tmp2 = " '2";
	if (N <= 3) tmp = "UDFBLR";
	else tmp = "UDFBLRudfblr";
	string ans = "";
	int prev = -1;
	for (int i = 0;i<count;++i) {
		int t = rand()%tmp.size();
		while (t == prev) {t = rand()%tmp.size();}
		ans += tmp[t];
		ans += tmp2[rand()%tmp2.size()];
		ans += " ";
	}
	return ans;
}

bool check(cube *cubes1, cube *cubes2) {
	for (int i = 0;i<N*N*N;++i) {
		if ((cubes1[i].U != cubes2[i].U) || (cubes1[i].L != cubes2[i].L) || (cubes1[i].F != cubes2[i].F) ||
			(cubes1[i].D != cubes2[i].D) || (cubes1[i].R != cubes2[i].R) || (cubes1[i].B != cubes2[i].B)) return 0;
	}
	return 1;
}

void stress() {
	if (N != 3) {
		cout<<"Stress supported only for N=3"<<endl;
		return;
	}
	const int count = 1;
	cube completed[N*N*N];
	fillcubes(completed);
	for (int i = 0;i<count;++i) {
		fillcubes(cubes);
		run(cubes, generateRandomRotations(50));
		string cmd = "kociemba " + getStringCube(cubes);
		string algo = exec(cmd.c_str());
		run(cubes, algo);
		if (check(cubes, completed)) cout<<"Test passed"<<endl;
		else cout<<"Test failed"<<endl;
	}
}

int getCount(string algo) {
	cube completed[N*N*N], current[N*N*N];
	fillcubes(completed);
	fillcubes(current);
	int ans = 1;
	for (;;++ans) {
		run(current, algo);
		if (check(current, completed)) return ans;
	}
}

void rq(string algo, int depth) {
	depth--;
	for (int i = 0;i<z.size();i += 2) {
		string newalgo = algo + z.substr(i,2);
		if (newalgo.size()>4 && newalgo[newalgo.size()-2] == newalgo[newalgo.size()-5]) continue;
		if (depth) rq(newalgo + " ", depth);
		else {
			while (q.size() >= queue_size) {
				usleep(1000);
			} 
			q.push(newalgo);
		}
	}
}

void generateRotations() {
	if (N<=3) z = "U U'U2D D'D2L L'L2R R'R2B B'B2F F'F2";
		else z = "U U'U2D D'D2L L'L2R R'R2B B'B2F F'F2u u'u2d d'd2l l'l2r r'r2b b'b2f f'f2";
	for (int n = 1;n<=maxn;++n) {
		rq("", n);
	}
}

void thrf() {
	usleep(100000);
	while (1) {
		locker.lock();
		if (q.empty()) {
			cout<<"Queue empty"<<endl;
			thrrun--;
			locker.unlock();
			return;
		}
		string algo = q.front();
		q.pop();
		locker.unlock();
		int count = getCount(algo);
		locker.lock();
		if (count > mx) {
			mx = count;
			cout<<count<<": "<<algo<<endl;
		}
		locker.unlock();
	}
}

int main() {
	srand(time(NULL));
	string algo;
	getline(cin, algo);
	if (algo == "max") {
		for (int i = 0;i<threads;++i) {
			thread thr(thrf);
			thr.detach();
			thrrun++;
		}
		generateRotations();
		while (thrrun) {usleep(10000);}
	}
	else cout << getCount(algo)<<endl;
	return 0;
}