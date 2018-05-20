//
//  main.cpp
//  social_sensor
//
//  Created by Jurim Lee on 2018. 4. 28..
//  Copyright © 2018년 Jurim Lee. All rights reserved.
//
//
//
#include <cstdio>
#include <map>
#include <cmath>
#include <algorithm>
#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include <ilcplex/ilocplex.h>
//#include "celf++_code_release/MC.cc"
//#include "celf++_code_release/InfluenceModels.cpp"

#define NUM_SS 2
#define RC_EPS 1.0e-6
#define MAX_ITR 10
#define MAX_INP 30

using namespace std;
//  STL : standard template library
ILOSTLBEGIN

int run_columngeneration(int num, int num_mode, int mode);

float process_query(int type, string inp);

void pricing_algorithm(int num_ss, IloNumArray dv);

void process_userdata();

IloNumArray run_iteration(IloEnv env,IloModel curOpt, IloNumVarArray choose, IloRangeArray constraints, IloNumVarArray cover, int num_tweet,int num_user, int num_pattern);

void report1 (IloCplex& zSolver, IloNumVarArray prob);

void report2 (IloAlgorithm& patSolver, IloNumVarArray ss,
              IloObjective obj);



//FILE *query_in = fopen("query.txt", "r");
//FILE *query_out = fopen("query_out.txt", "w");
FILE *in = fopen("data_more_than_one_node.txt", "r");
int n;
int num_ss;
float max_z;
string new_ss[NUM_SS];

//userid의 list가 소셜 센서 set이다.
//userid list가 각각 하나의 S_i (소셜센서 셋)이 된다.
//coverd(W,S_i) : W번째 트윗이 S_i 소셜센서 셋에 걸려있는지 아닌지를 리턴하는 함수.


struct _tweet {
    int type;//root인지 아닌지.
    string user_id;
    long long time;
    string body;
    vector<int> topic_num;
    int rtw_count;
    int root_index;
    int is_covered = 0;
    int tree_size;//사실상...트리 전체의 사이즈
    double cover_point=0.1;//cover 점수
    vector<int> rtw;//자식의 index값을 가진다.
    string identifier;//this value will be matched with map key;
};
vector <_tweet> tweet;

struct _user {
    string user_id;
    int index;
    vector<int> tweets;//user의 tweet들의 index
};
map<string, _user> user;
vector<string> indexToUser;

struct _sensor_node {
    int index;//user의 index
    double point;
    int calced;
};

_sensor_node sheap[301004];

vector<string> first_ss;

vector<int> new_ss_idx;

// dd는 cost 주는 함수
double dd(int indexx, IloNumArray *dv) {
    return 0.0;//(((indexx + 100))*rand()) % 12;// +tweet[indexx].tree_size;
}

bool sensor_cmp(_sensor_node a, _sensor_node b) {
    if (a.point < b.point) return 1;
    return 0;
}


int main(int argc, char **argv) {
    run_columngeneration(2, 1, 1);
    cout << "the number of nodes in social sensor : " << num_ss << endl;
    cout << "Z value : " << max_z << endl;
    return 0;
}

void ev1_objective_function_comparison(int mode) {
    //  1. Objective Function Comparison
    //      4개의 다른 값 (covered, time, topic coverage, out-degree)의 비교
    //      x축 : 선택할 수 있는 social sensor의 개수
    //      y축 : reward (normalize한 값의 비교)
    
    for (int i=0;i<5;i++) {
        run_columngeneration(i+2, 1, mode);
        cout << "num_ss (x) : " << num_ss << " reward (y) : " << max_z <<endl;
    }
}

void ev2_objective_functions_comparison(int num_mode, int mode) {
    //  2. 1에서 한 실험에서 값을 2개 혹은 3개로 묶어서 비교
    //      x축 : 선택할 수 있는 social sensor의 개수
    //      y축 : reward (normalize한 값의 합?의 비교)
    //  input : num_mode > 몇 개의 숫자가 들어올지, mode > 12 (1번 OF과 2번 OF)
    
    for (int i=0;i<5;i++) {
        run_columngeneration(i+2, num_mode, mode);
        cout << "num_ss (x) : " << num_ss << " reward (y) : " << max_z << endl;
    }
    
}

void ev3_train_test() {
    //  3. Jure 논문의 7번 실험 - 50% 데이터로 SS구하고 -> 나머지 50%로 reward
    //      x축 : cost (Jure에서는 cost로 함) 총 포함되는 데이터의 개수 (트윗 글의 개수)
    //      y축 : reward (normalize한 값의 합의 비교)
    
}




int run_columngeneration(int num, int num_mode, int mode) {
    IloEnv env;
    num_ss = num;
    int k, l=0;
    try {

        
        //  data load
        process_userdata();
        
        //  필요한 인티저 값들 변수에 할당하기 (패턴 수, 트윗 개수 등)
        IloInt num_pattern = 4;
        IloInt num_user = user.size();
        IloInt num_tweet = tweet.size();
        
        
        clock_t begin = clock();
        //constraint.add(IloSum(choose) <= 1);
        string temp_;
        
        first_ss.push_back(" unpacker binitamshah");
        first_ss.push_back(" IBMSecurity MickWilliamsPhD");
        first_ss.push_back(" dsadfeq11 Renaissancelre");
        first_ss.push_back(" NabeelAhmedBE malekal_morte");
        
        for (int j=0;j<MAX_ITR;j++) {
            // 기존에 있는 값으로 초기화 해주기

            //  모델 만들기 - 현재 variable 가지고 최선의 확률 구하는 모델
            IloModel curOpt(env);
            
            //  cutting-optimization 모델 만들기 (column-wise로 할 것)
            IloNumVarArray choose(env, num_pattern, 0.01, 0.25, ILOFLOAT);
            IloRangeArray constraint(env);
            IloNumVarArray cover(env, num_tweet, 0.0, 1.0, ILOFLOAT);
            IloNumVar z(env);
            
            IloNumArray dv = run_iteration(env, curOpt, choose, constraint, cover, num_tweet, num_user, num_pattern);
            
            // pricing algorithm 돌리기
            pricing_algorithm(num_ss, dv);
            
            
            string add_pattern = "";
            for (l=0;l<num_ss;l++)
                add_pattern += " " + indexToUser[new_ss_idx[l]];
            
            for (int k=0;k<first_ss.size();k++)
                cout << "new social sensor" << k << ": " << first_ss[k] << endl;
            first_ss.push_back(add_pattern);
            
            choose.end();
            constraint.end();
            cover.end();
            curOpt.end();
            
            
            
            num_pattern += 1;
            
        }
    }
    catch (IloException& ex) {
        cerr << "Error: " << ex << endl;
    }
    catch (...) {
        cerr << "Unknown Error" << endl;
    }
         
    return 0;
}


float process_query(int type, string inp) {
    
    char t_str[1004];
    // 인풋 스트링 처리
    // type=1 : inp = "tweet_num the_num_of_ppl_in_ss a b c"
    // type=2 : inp = "the_num_of_ppl_in_ss a b c"
    int idx = 0;
    string tokens[MAX_INP];
    stringstream ssin(inp);
    while (ssin.good() && idx < MAX_INP) {
        ssin >> tokens[idx];
        ++idx;
    }
    if (type == 1) {//coverd(W,S_i)를 질의하는 쿼리
        int tweet_index, user_cnt;
        int i, j;
        string sensor_id;
        tweet_index = stoi(tokens[0]);
        user_cnt = stoi(tokens[1]);
        
        //fscanf(query_in, "%d", &tweet_index);
        //fscanf(query_in, "%d", &user_cnt);
        int flag = 0;
        if (tweet_index >= tweet.size()) {
            //fprintf(query_out, "2\n");//Out of index
            flag = 1;
        }
        for (int it = 0; it < user_cnt; it++) {
            strncpy(t_str, tokens[it+2].c_str(), sizeof(t_str)-1);
            if(sizeof(t_str)>0)
                t_str[sizeof(t_str)-1] = 0;
            //fscanf(query_in, "%s", t_str);
            if (flag == 0) {
                sensor_id = t_str;
                int len = user[sensor_id].tweets.size();
                if (!user.count(t_str)) {
                    //fprintf(query_out, "3\n");//no user found
                    flag = 1; continue;
                }
                for (i = 0; i < len; i++)
                {
                    if (tweet[user[sensor_id].tweets[i]].identifier == tweet[tweet_index].identifier) {
                        //fprintf(query_out, "1\n");//Covered
                        flag = 1;
                        return 1.0;
                    }
                }
            }
        }
        if (flag == 0) return 0.0;//fprintf(query_out, "0\n");//Not Covered
        return 0.0;
    }
    else if (type == 2) {//coverd(W_total,S_i)을 질의하는 쿼리
        map<string, int> covering;
        string sensor_id;
        int i, user_cnt, cnt = 0;
        int flag = 0;
        user_cnt = stoi(tokens[0]);
        //fscanf(query_in, "%d", &user_cnt);
        for (int it = 0; it < user_cnt; it++) {
            strncpy(t_str, tokens[it+1].c_str(), sizeof(t_str)-1);
            if(sizeof(t_str)>0)
                t_str[sizeof(t_str)-1] = 0;
            //fscanf(query_in, "%s", t_str);
            if (flag == 0)
            {
                sensor_id = t_str;
                if (!user.count(t_str)) {
                    //fprintf(query_out, "0\n0\n");//no user found
                    flag = 1;
                    continue;
                }
                int len = user[sensor_id].tweets.size();
                for (i = 0; i < len; i++)
                {
                    covering[tweet[user[sensor_id].tweets[i]].identifier] = tweet[user[sensor_id].tweets[i]].tree_size;
                }
            }
        }
        for (map<string, int>::iterator it = covering.begin(); it != covering.end(); ++it) {
            cnt += it->second;
        }
        //fprintf(query_out, "1\n%lf\n", (double)cnt / tweet.size());
        return (double)cnt / tweet.size();
    }
    else if (type == 3) {// Total Tweet Count를 질의하는 쿼리
        //fprintf(query_out, "%d\n", tweet.size());//Not Covered
        return tweet.size();
    }
    else if (type == 4) {// Total User Count를 질의하는 쿼리
        //fprintf(query_out, "%d\n", user.size());//Not Covered
        return user.size();
    }
    else if (type == 5) {//1500000000
        //delay(W,S_i)를 질의하는 쿼리
        int tweet_index, user_cnt;
        int i, j;
        string sensor_id;
        tweet_index = stoi(tokens[0]);
        user_cnt = stoi(tokens[1]);
        //fscanf(query_in, "%d", &tweet_index);
        //fscanf(query_in, "%d", &user_cnt);
        int flag = 0;
        if (tweet_index >= tweet.size()) {
            flag = 1;//out of index
            //fprintf(query_out, "-2\n");//out of index
            return -2.0;
        }
        long long minv = 1500000000;
        for (int it = 0; it < user_cnt; it++) {
            strncpy(t_str, tokens[it+2].c_str(), sizeof(t_str)-1);
            if(sizeof(t_str)>0)
                t_str[sizeof(t_str)-1] = 0;
            //fscanf(query_in, "%s", t_str);
            if (flag == 1) {
                continue;
            }
            sensor_id = t_str;
            int len = user[sensor_id].tweets.size();
            if (!user.count(t_str)) {
                //fprintf(query_out, "-1\n");//no user found
                flag = 1;
                continue;
            }
            for (i = 0; i < len; i++)
            {
                if (tweet[user[sensor_id].tweets[i]].identifier == tweet[tweet_index].identifier) {
                    //Covered
                    if (minv > tweet[user[sensor_id].tweets[i]].time - tweet[tweet[user[sensor_id].tweets[i]].root_index].time) {
                        minv = tweet[user[sensor_id].tweets[i]].time - tweet[tweet[user[sensor_id].tweets[i]].root_index].time;
                    }
                }
            }
        }
        //fprintf(query_out, "%lld\n", 1500000000 - minv);
        return (1500000000 - minv)/1500000000.0;
    }
    else if (type == 6) {//
        int i, j, k;
        FILE *query_out = fopen("/Users/jurimlee/Desktop/cplex_example/social_sensor/social_sensor/celf++_code_release/datasets/celf_training.txt", "w");
        //fprintf(query_out, "%d\n",NUM_SS);
        for (i = 0; i < indexToUser.size(); i++)
        {
            map<string, double> userToUserWeight;
            for (j = 0; j < user[indexToUser[i]].tweets.size(); j++) {
                if (tweet[user[indexToUser[i]].tweets[j]].type == 1) {
                    for (k = 0; k < tweet[user[indexToUser[i]].tweets[j]].rtw.size(); k++) {
                        if (!userToUserWeight.count(tweet[tweet[user[indexToUser[i]].tweets[j]].rtw[k]].user_id))
                        {
                            userToUserWeight[tweet[tweet[user[indexToUser[i]].tweets[j]].rtw[k]].user_id] = 0;
                        }
                        userToUserWeight[tweet[tweet[user[indexToUser[i]].tweets[j]].rtw[k]].user_id] += tweet[user[indexToUser[i]].tweets[j]].cover_point;
                    }
                }
            }
            for (map<string, double>::iterator it = userToUserWeight.begin(); it != userToUserWeight.end(); ++it) {
                fprintf(query_out, "%d %d %lf\n", i, user[it->first].index, it->second);
            }
        }
        //fprintf(query_out, "user_weight_end\n");
    }
    else if (type == 7) {
        FILE *CELF_in = fopen("/Users/jurimlee/Desktop/cplex_example/social_sensor/social_sensor/celf++_code_release/output/IC_CelfPlus_Greedy.txt", "r");
        int i, j;
        int size,uid;
        //fscanf(CELF_in, "%d", &size);
        size = 2;
        
        
        //fprintf(query_out, "user_index_start\n");
        for (i = 0; i < size; i++)
        {
            fscanf(CELF_in, "%d", &uid);
            //fprintf(query_out, "%s %d\n", indexToUser[uid].c_str(), uid);
            new_ss[i] = indexToUser[uid].c_str();
            for (j = 0; j < 9; j++)
            {
                double trash;
                fscanf(CELF_in, "%lf", &trash);
            }
        }
        //fprintf(query_out, "user_index_end\n");
    }
    return 0.0;
}

void pricing_algorithm(int num_ss, IloNumArray dv) {
    new_ss_idx.clear();
    map<string, IloNum> covering;
    int i, j;
    int n;
    //fscanf(query_in, "%d", &n);
    n = num_ss;
    int usernum = indexToUser.size();
    int hsize = 0;
    for (i = 0; i < usernum; i++) { // initial heap 채우기
        sheap[hsize].index = i;
        sheap[hsize].point = 0;
        sheap[hsize].calced = 0;
        int len = user[indexToUser[i]].tweets.size();
        for (j = 0; j < len; j++)
        {
            if (!tweet[user[indexToUser[i]].tweets[j]].is_covered)
            {
                covering[tweet[user[indexToUser[i]].tweets[j]].identifier] = dv[user[indexToUser[i]].tweets[j]];
            }
        }
        for (map<string, double>::iterator it = covering.begin(); it != covering.end(); ++it) {
            sheap[hsize].point += it->second;
        }
        hsize++;
        std::push_heap(sheap, sheap + hsize, sensor_cmp);
        covering.clear();
    }
    if (n > hsize) {
        cout << "Sensor size is too big!" << endl;
        //fprintf(query_out, "Sensor Size Is Too Big!!!!!!!!!!\n");
    }
    for (i = 0; i < n; i++)//heap에서 하나씩 빼기
    {
        int forcheck = 0;
        while (sheap[0].calced!=i) {
            _sensor_node temp;
            temp = sheap[0];
            temp.point = 0;
            temp.calced = i;
            sheap[0] = sheap[hsize - 1];
            std::pop_heap(sheap, sheap + hsize, sensor_cmp);
            hsize--;
            string uid = indexToUser[temp.index];
            int len = user[uid].tweets.size();
            for (j = 0; j < len; j++)
            {
                if (!tweet[user[uid].tweets[j]].is_covered)
                {
                    covering[tweet[user[uid].tweets[j]].identifier] = dv[user[uid].tweets[j]];
                }
            }
            for (map<string, double>::iterator it = covering.begin(); it != covering.end(); ++it) {
                temp.point += it->second;
            }
            sheap[hsize] = temp;
            hsize++;
            std::push_heap(sheap, sheap + hsize, sensor_cmp);
            covering.clear();
            forcheck++;
        }
        printf("%d번만에 찾음\n", forcheck);
        new_ss_idx.push_back(sheap[0].index);
        //fprintf(query_out, "%d\n", sheap[0].index);
        //fprintf(query_out, "%d개의 트윗을 만듦\n", user[indexToUser[sheap[0].index]].tweets.size());
        std::pop_heap(sheap, sheap + hsize, sensor_cmp);
        hsize--;
    }
    for(i=0;i<tweet.size();i++){
        tweet[i].is_covered=0;
    }
}

IloNumArray run_iteration(IloEnv env,IloModel curOpt, IloNumVarArray choose, IloRangeArray constraints, IloNumVarArray cover, int num_tweet,int num_user, int num_pattern) {
    
    constraints.add(IloSum(choose) <= 1);
    
    for (int k=0;k<num_tweet;k++) {
        IloNumArray cover_w(env, num_pattern, 0.0, 1.0, ILOINT);
        for (int l=0;l<num_pattern;l++) {
            string temp;
            // concat initial user names
            //inp = "tweet_num the_num_of_ppl_in_ss a b c"
            
            temp = to_string(k) + " " + to_string(NUM_SS)+first_ss[l];
            cover_w[l] = (int)process_query(1, temp);
        }
        
        constraints.add(IloScalProd(choose, cover_w)-cover[k]>=0);
    }
    
    //constraint.add(IloSum(cover)-z>=0);
    cout << "Constraints added..." << endl;
    curOpt.add(constraints);
    curOpt.add(IloMaximize(env, IloSum(cover)));
    
 
    //  define solver
    IloCplex ssSolver(curOpt);
    
    
    //  pattern-generation
    IloModel patGen (env);
    IloNumArray size(env, num_user);
    for (int h=0;h<num_user;h++)
        size[h] = 1;
    
    IloObjective ReducedCost = IloAdd(patGen, IloMaximize(env, 1));
    IloNumVarArray ss(env, num_user, 0.0, 1.0, ILOINT);
    patGen.add(IloScalProd(size, ss) <= NUM_SS);
    
    IloCplex patSolver(patGen);
    IloNumArray newPatt(env, NUM_SS);
    
    //  column-generation - dual value구하기(itr에 횟수 제한 해서 그만큼 돌리기 먼저 10번? 짧게 돌려볼 것)
    //  MAX_ITR에서 정한 만큼 돌리기
    IloNumArray dv(env, num_tweet);
    
    ssSolver.solve();
    
    cout << "choose : " << choose << endl;
    report1(ssSolver,choose);
    cout << "get dual start" << endl;
    for (int m=0;m<num_tweet;m++)
        dv[m] = -ssSolver.getDual(constraints[m+1]);
    
    //cout << "setLinarCoefs start" << endl;
    ReducedCost.setLinearCoefs(ss, dv);
    
    //cout << "patSolver start" << endl;
    patSolver.solve();
    //report2 (patSolver, ss, ReducedCost);
    
    
    
    if (patSolver.getValue(ReducedCost) < RC_EPS) {
        cout << "DONE, patSolver.getValue : " << patSolver.getValue(ReducedCost) << endl;
        return dv;
    }
    
    cout << "patSolver.getValues" << endl;
    patSolver.getValues(newPatt, ss);
    
    ssSolver.end();
    return dv;
}


//  트위터 데이터로 트리 만들기
void process_userdata()
{
    int i, j, k;
    _tweet temp;
    char t_str[1004];
    int t_tnum,t_int;
    int save;
    k = 1;
    while (true)
    {
        k++;
        fscanf(in, "%d", &temp.type);
        if (temp.type == 2)break;
        fscanf(in, "%s", t_str);//user_id:
        fscanf(in, "%s", t_str);//temp.user_id
        temp.user_id = t_str;
        fscanf(in, "%s", t_str);//user_id:
        fscanf(in, "%lld.0", &temp.time);
        if (temp.type == 1) {
            fscanf(in, "%s ", t_str);//body:
            save = tweet.size();//가장 최근에 나온 root tweet을 저장함.
            fgets(t_str, 10000, in);
            temp.body = t_str;//body 내용을 입력받음.
        }
        else {
            temp.body = "";
        }
        fscanf(in, "%s", t_str);//topic_num:
        fscanf(in, "%d", &t_tnum);//topic num 입력받음
        temp.topic_num.clear();
        for(i=0;i<t_tnum;i++){
            fscanf(in, "%d", &t_int);
            temp.topic_num.push_back(t_int);
        }
        fscanf(in, "%s", t_str);//rtw_count:
        fscanf(in, "%d", &temp.rtw_count);
        if (temp.type == 0) {//자식 tweet(RT)
            temp.identifier = tweet[save].identifier;
            if (!user.count(temp.user_id)) {
                _user temp_user;
                temp_user.user_id = temp.user_id;
                temp_user.index = indexToUser.size();
                indexToUser.push_back(temp.user_id);
                user[temp.user_id] = temp_user;
            }
            user[temp.user_id].tweets.push_back(tweet.size());
            temp.root_index = save;
            tweet.push_back(temp);
            tweet[save].rtw.push_back(tweet.size() - 1);//자식 node일 경우엔 부모에 갖다붙인다.
            for (i = save; i < tweet.size(); i++)
            {
                tweet[i].tree_size = tweet.size() - save;
            }
        }
        else {//부모일 경우엔 identifier을 입력받는다.
            fscanf(in, "%s", t_str);//identifier:
            fscanf(in, "%s", t_str);//temp.identifier
            temp.identifier = t_str;
            temp.root_index = save;
            if (!user.count(temp.user_id)) {
                _user temp_user;
                temp_user.user_id = temp.user_id;
                temp_user.index = indexToUser.size();
                indexToUser.push_back(temp.user_id);
                user[temp.user_id] = temp_user;
            }
            user[temp.user_id].tweets.push_back(tweet.size());
            tweet.push_back(temp);
        }
    }
    tweet[0].cover_point = 1;
    
    cout << "User Preprocessing End..." << endl;
}

void report1 (IloCplex& zSolver, IloNumVarArray choose) {
    
    cout << endl;
    cout << "Sum of Z " << zSolver.getObjValue() << endl;
    max_z = zSolver.getObjValue();
    cout << endl;
    for (IloInt j=0; j < choose.getSize(); j++) {
        cout << "   Z" << j << " = " << zSolver.getValue(choose[j]) << endl;
    }
    cout << endl;
}

void report2 (IloAlgorithm& patSolver, IloNumVarArray ss,
                     IloObjective obj)
{
    cout << endl;
    cout << "Reduced cost is " << patSolver.getValue(obj) << endl;
    cout << endl;
    if (patSolver.getValue(obj) >= RC_EPS) {
        for (IloInt i = 0; i < ss.getSize(); i++)  {
            cout << "  ss" << i << " = " << patSolver.getValue(ss[i]) << endl;
        }
        cout << endl;
    }
}
