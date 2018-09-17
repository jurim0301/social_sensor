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
#include <fstream>
#include <sstream>
#include <ilcplex/ilocplex.h>


//#define NUM_SS 4
#define MAX_ITR 2
#define MAX_INP 30
#define MAX_OUT 15224

using namespace std;
//  STL : standard template library
ILOSTLBEGIN

void ev1(int mode, int d_mode);

void ev1_train(int mode, int d_mode);

void ev2(int mode, int d_mode);

void ev2_train(int mode, int d_mode);

void run_columngeneration(int num_ss, int num_itr,int num_mode,int mode,float percent, int total_data);

IloNumArray calculate_obj(IloEnv env, int num_tweet, int mode, int vec_i);

double process_query(int type, string inp, int topic_num);

void pricing_algorithm(int num_ss, IloNumArray dv);

void process_userdata(FILE* in);


IloNumArray run_iteration(IloEnv env, IloModel curOpt, IloObjective obj, IloRangeArray constraints, int num_tweet, int mode, IloNumVarArray choose);




//FILE *query_in = fopen("query.txt", "r");
//FILE *query_out = fopen("query_out.txt", "w");

// data for twitter dataset (experiment 1)
FILE *in = fopen("data_more_than_one_node_june_train.txt", "r");
FILE *test = fopen("data_more_than_one_node_june_test.txt", "r");

// data for bitcoin forum dataset (experiment 2)
//FILE *in = fopen("bitcoin/bitcoin_forum_train.txt", "r");
//FILE *test = fopen("bitcoin/bitcoin_forum_test.txt", "r");
float min_prob = 0.0;
float max_prob = 0.25;
int n;
int num_ss;
float max_z;
int ss;
int itr;
int d_mode;



// this array is for original CELF algorithm
// in this project, we are not gonna use this array
string new_ss[100];

//userid의 list가 소셜 센서 set이다.
//userid list가 각각 하나의 S_i (소셜센서 셋)이 된다.
//coverd(W,S_i) : W번째 트윗이 S_i 소셜센서 셋에 걸려있는지 아닌지를 리턴하는 함수.

// social sensor set (S_i) : a list of user ids
// covered(W, S_i) : whether W-th tweet is covered by social sensor set (S_i)


struct _tweet {
    int type; // whether root or not
    string user_id;
    long long time;
    string body;
    vector<double> topic;
    int rtw_count;
    int root_index;
    int is_covered = 0;
    int tree_size;// the size of a tree
    double cover_point=0.1;//cover point
    vector<int> rtw;// an index of child node (in the tree)
    string identifier;//this value will be matched with map key;
};
vector <_tweet> tweet;

struct _user {
    string user_id;
    int index;
    vector<int> tweets; // indices of user tweets
};
map<string, _user> user;
vector<string> indexToUser;

struct _sensor_node {
    int index; // an index of a user
    double point;
    int calced;
};

_sensor_node sheap[301004];

vector<string> first_ss;
vector<string> second_ss;//single

vector<int> new_ss_idx;



bool sensor_cmp(_sensor_node a, _sensor_node b) {
    if (a.point < b.point) return 1;
    return 0;
}




int main(int argc, char **argv) {
    //  input format : ./test.o [mode] [how many ids in one social sensor set] [iteration] [upper limit] [data num]
    //  [mode] : 1> covered 2> topic coverage 3> retweet num 4> time reward 1234> use all
    //  [how many ids in one social sensor set] : 5, 10, or 20
    //  [iteration] : fixed to 50 (50 social sensor sets will be calculated)
    //  [upper limit] : max_prob, 0.5 or 0.25
    //  [data num] : 0> twitter dataset 1> bitcoin forum
    
    cout << "start" << endl;
    cout << "argv[0] : " << argv[0] << endl;
    char *num = argv[1];
    cout << "mode : " << num << "," << endl;
    int mode = stoi(num);
    
    
    ss = stoi(argv[2]);
    itr = stoi(argv[3]);
    max_prob = stof(argv[4]);
    d_mode = stoi(argv[5]);
    
    if(d_mode==0) {
        in = fopen("data_more_than_one_node_june_train.txt", "r");
        test = fopen("data_more_than_one_node_june_test.txt", "r");
    }
    else if(d_mode==1) {
        in = fopen("bitcoin_forum_train.txt", "r");
        test = fopen("bitcoin_forum_test.txt", "r");
    }
    
    
    cout << "the number of sensors in one social sensor set : " << ss << endl;
    cout << "the number of iteration : " << itr << endl;
    
    //  1. run column generation algorithm (must be turn off when experiment code is on)
    //run_columngeneration(ss, itr,strlen(num), mode, 1.0, tweet.size());
    
    //  2.
    ev1(mode, d_mode);
    ev1_train(mode, d_mode);
    //ev4(mode);
    //ev4_train(mode);
    cout << "the number of nodes in social sensor : " << ss << endl;
    cout << "Z value : " << max_z << endl;
    
    return 0;
}

void run_columngeneration(int num_ss, int num_itr,int num_mode, int mode,float percent, int total_data) {
    
    cout << "mode :: " << mode << endl;
    IloEnv env;
    int k, l=0;
    try {
        
        //  data load
        
        process_userdata(in);
        
        cout << "COMPLETE" << endl;
        //  필요한 인티저 값들 변수에 할당하기 (패턴 수, 트윗 개수 등)
        //IloInt num_pattern = 4;
        IloInt num_tweet = tweet.size();
        
        
        
        //constraint.add(IloSum(choose) <= 1);
        string temp_;
        
        ofstream f_result;
        f_result.open("result_"+to_string(d_mode)+"_"+to_string(mode)+"_"+to_string(ss)+"_"+to_string(itr)+to_string(max_prob)+".txt");

        if (ss==5) {
            if(d_mode == 0) {
                first_ss.push_back(" CiscoSecurity EricMartorano RSAConference prodboct InfographicsIts");
                first_ss.push_back(" drajaykumar_ias msftmmpc RussTechMtIsa Seccom_Global SenseiEnt");
            }
            else if (d_mode==1) {
            // 비트코인 포럼 유저
            first_ss.push_back(" OmegaStarScream LiteCoinGuy Elwar cuddaloreappu evoorhees");
            first_ss.push_back(" CryptoCurrencyInc.com jonald_fyookball Bitcoinpro Anonymous Phinnaeus Gage");
            }
            
        }
        else if(ss==10) {
            
            if (d_mode==0) {
                first_ss.push_back(" CiscoSecurity EricMartorano RSAConference prodboct InfographicsIts drajaykumar_ias msftmmpc RussTechMtIsa Seccom_Global SenseiEnt");
                first_ss.push_back(" ncxgroup steven_ericksen EllipticSystems jdesai_storage CyberSecRicki cylanceinc smesecurity evankirstel MaribelLopez fjpanizo");
            }
            else if(d_mode==1) {
                // 비트코인 포럼 유저
                first_ss.push_back(" OmegaStarScream LiteCoinGuy Elwar cuddaloreappu evoorhees CryptoCurrencyInc.com jonald_fyookball Bitcoinpro Anonymous Phinnaeus Gage");
                first_ss.push_back(" the founder Coinbuddy RawDog remotemass the_poet hl5460 OROBTC BittBurger alyssa85 pawel7777");

            }
            
        }
        else if (ss==15) {
            if (d_mode==0) {
                first_ss.push_back(" CiscoSecurity EricMartorano RSAConference prodboct InfographicsIts drajaykumar_ias msftmmpc RussTechMtIsa Seccom_Global SenseiEnt ncxgroup steven_ericksen EllipticSystems jdesai_storage CyberSecRicki");
                first_ss.push_back(" cylanceinc smesecurity evankirstel MaribelLopez fjpanizo mistergmedina Sarahetodd BAESystems_AI STIDIA_Security jjsystems jokichu scopeme afyonluoglu erikremmelzwaal FlynnPartyof5");
            }
            else if(d_mode==1) {
                first_ss.push_back(" OmegaStarScream LiteCoinGuy Elwar cuddaloreappu evoorhees CryptoCurrencyInc.com jonald_fyookball Bitcoinpro Anonymous Phinnaeus Gage the founder Coinbuddy RawDog remotemass");
                first_ss.push_back(" the_poet hl5460 OROBTC BittBurger alyssa85 pawel7777 MicroGuy cbeast Bit_Happy casascius Minecache jubalix pereira4 commandrix spazzdla shamzblueworld");
            }
        
        }
        else if(ss==20) {
            
            if(d_mode==0) {
                first_ss.push_back(" CiscoSecurity EricMartorano RSAConference prodboct InfographicsIts drajaykumar_ias msftmmpc RussTechMtIsa Seccom_Global SenseiEnt ncxgroup steven_ericksen EllipticSystems jdesai_storage CyberSecRicki cylanceinc smesecurity evankirstel MaribelLopez fjpanizo");
                first_ss.push_back(" mistergmedina Sarahetodd BAESystems_AI STIDIA_Security jjsystems jokichu scopeme afyonluoglu erikremmelzwaal FlynnPartyof5 CorvilInc sophos_info pauleriksen struppigel NoEwS herkeictgroup mflynn2 Cyber_Veille ITbadr matthieugarin");
            }
            else if(d_mode==1) {
                // 비트코인 포럼 유저
                first_ss.push_back(" OmegaStarScream LiteCoinGuy Elwar cuddaloreappu evoorhees CryptoCurrencyInc.com jonald_fyookball Bitcoinpro Anonymous Phinnaeus Gage the founder Coinbuddy RawDog remotemass the_poet hl5460 OROBTC BittBurger alyssa85 pawel7777");
                first_ss.push_back(" MicroGuy cbeast Bit_Happy casascius Minecache jubalix pereira4 commandrix spazzdla shamzblueworld Kprawn cryptocurrencylive knight22 bbit Huobi-USD freightjoe TKeenan BitPay Business Solutions adamstgBit Yankee (BitInstant)");
            }
            

            
        }
        

        
        for (int vec_i=0;vec_i<first_ss.size();vec_i++)
            f_result << first_ss[vec_i] << "\n";
        
        IloInt num_pattern = first_ss.size();
        IloModel curOpt(env);
        // obj
        IloObjective obj = IloMaximize(env);

        // c
        IloRangeArray constraint(env);
        // x
        IloNumVarArray choose(env);
        
        // 이미 구축해놓은 소셜 센서에 대해 constraints 업데이트
        
        for (int twt_num=0; twt_num<num_tweet;twt_num++) {
            //  트윗 수만큼 constraint 추가해주기
            constraint.add(IloRange(env, 0.0, IloInfinity));
        }
        for (int vec_i=0; vec_i<first_ss.size();vec_i++) {
            IloNumArray cover_twt = calculate_obj(env, num_tweet, mode, vec_i);
            IloNumColumn expr(env);
            for (int t=0;t<constraint.getSize();t++) {
                expr += constraint[t](cover_twt[t]);
            }
            choose.add(IloNumVar(obj(IloSum(cover_twt))+expr, min_prob, max_prob));
        }
            
        

        for (int j=0;j<num_itr;j++) {
            cout << "num_pattern : " << num_pattern << endl;
            
            
            clock_t start = clock();
            // reduced master problem
            //IloNumArray dv = run_iteration(env, curOpt, choose, constraint, cover, num_tweet, num_user, num_pattern,num_mode, mode);
            IloNumArray dv = run_iteration(env, curOpt, obj, constraint, num_tweet, mode, choose);
        
            
            cout << "iteration time : " << (float)(clock() - start)/CLOCKS_PER_SEC << endl;
            
            // pricing algorithm 돌리기
            pricing_algorithm(num_ss, dv);
            
            
            
            string add_pattern = "";
            for (l=0;l<num_ss;l++)
                add_pattern += " " + indexToUser[new_ss_idx[l]];
            
            first_ss.push_back(add_pattern);
            for (int k=0;k<first_ss.size();k++) {
                cout << "new social sensor" << k << ": " << first_ss[k] << endl;
            }
            
            f_result << first_ss[first_ss.size()-1] << "\n";
            
            
            IloNumArray cover_twt = calculate_obj(env, num_tweet, mode, num_pattern);
            IloNumColumn expr(env);
            for (int t=0;t<constraint.getSize();t++) {
                expr += constraint[t](cover_twt[t]);
            }
            choose.add(IloNumVar(obj(IloSum(cover_twt))+expr, min_prob, max_prob));
            
            
            
            num_pattern += 1;
            
            
        }
        f_result.close();
    }
    catch (IloException& ex) {
        cerr << "Error: " << ex << endl;
    }
    catch (...) {
        cerr << "Unknown Error" << endl;
    }
    
}
IloNumArray calculate_obj(IloEnv env, int num_tweet, int mode, int vec_i) {
    clock_t start = clock();
    IloNum t = 0.0;
    IloNumArray cover_twt(env, num_tweet);
    for (int twt_num=0; twt_num<num_tweet;twt_num++) {
        string temp = to_string(twt_num) + " " + to_string(ss)+first_ss[vec_i];
        int mode_ = mode;
        int length = 1;
        float covered = (float)process_query(1, temp, 0);
        while (mode_ != 0) {
            int type = mode_ % 10;
            if (type==1) {
                //covered
                cover_twt[twt_num] += covered;
            }
            else if(type==2) {
                //topic covered
                if (covered == 1.0) {
                    for (int topic_num=0;topic_num<tweet[twt_num].topic.size();topic_num++)
                        cover_twt[twt_num] += (float)tweet[twt_num].topic[topic_num];
                }
            }
            else if(type==3) {
                //out-link
                if (covered == 1.0) {
                    cover_twt[twt_num] += (float)tweet[twt_num].rtw_count/MAX_OUT;
                }
            }
            else if(type==4) {
                //time reward
                cover_twt[twt_num] += (float)process_query(5, temp, 0);
            }
            mode_ /= 10;
            length += 1;
        }
        cover_twt[twt_num] /= (float)length-1.0;
    }
    cout << "calculate_obj time elapsed : " << (clock()-start)/CLOCKS_PER_SEC << endl;
    return cover_twt;
}


void ev1(int mode, int d_mode) {
    //  # Jure 논문의 7번 실험 - 70% (또는 일부) 데이터로 SS구하고 -> 나머지로 reward
    //      x축 : cost (소셜센서 내의 인원수)
    //      y축 : reward (normalize한 값의 합의 비교)
    
    //  # same as fig.7 experiment in Jure paper - train with partial data items, test with rest
    //      x-axis : cost (the number of ids in one social sensor set)
    //      y-axis : reward (the sum of normalized values)
    
    
    //  input : mode (<1 covered <2 topic coverage <3 retweet num. <4 time reward <1234 all)
    //          d_mode (data mode, <0 tweet data <1 bitcoin forum data)
    //  output : None (result will be printed on text file)
    
    //  test with test set
    process_userdata(test);
    
    
    //  load social sensors
    char t_str[1004];
    char c_str[100];
    
    //  result file : list of social sensor set, separated with '\n'
    //  choose file : list of percentage for each social sensor set, separated with '\n'
    string result_file = "";
    string choose_file = "";
    if (d_mode == 0) {
        result_file = "twt_result/result_"+to_string(d_mode)+"_"+to_string(mode)+"_"+to_string(ss)+"_"+to_string(itr)+to_string(max_prob)+".txt";
        choose_file = "twt_result/choose_"+to_string(d_mode)+"_"+to_string(mode)+"_"+to_string(ss)+"_"+to_string(itr)+to_string(max_prob)+".txt";
    }
    else if (d_mode == 1) {
        result_file = "bitcoin_result/result_"+to_string(d_mode)+"_"+to_string(mode)+"_"+to_string(ss)+"_"+to_string(itr)+to_string(max_prob)+".txt";
        choose_file = "bitcoin_result/choose_"+to_string(d_mode)+"_"+to_string(mode)+"_"+to_string(ss)+"_"+to_string(itr)+to_string(max_prob)+".txt";
    }
    else {
        cout << "ev1 file location error" << endl;
    }
    
    ofstream exp;
    
    string file_name = "ev_"+to_string(d_mode)+"_"+to_string(mode)+"_"+to_string(ss)+"_"+to_string(itr)+to_string(max_prob)+".txt";
    exp.open(file_name);
    
    ifstream result(result_file);
    ifstream cs(choose_file);
    
    vector<float> chs;
    while(cs.getline(c_str, 100)) {
        chs.push_back(stof(c_str));
    }

    int itr = 0;
    int num_tweet = tweet.size();
    float total_z = 0.0;
    float temp_z = 0.0;
    clock_t start = clock();
    IloNum t = 0.0;
    float cover_twt[num_tweet];
    
    fill_n(cover_twt, num_tweet, 0.0);
    
    while(result.getline(t_str, 1004)) {
        fill_n(cover_twt, num_tweet, 0.0);
        if((itr > 2) && (chs[itr]==0.0)) {
            itr += 1;
            continue;
        }
        //  iterate over the number of tweets
        for (int twt_num=0; twt_num<num_tweet;twt_num++) {
            string temp = to_string(twt_num) + " " + to_string(ss)+t_str;
            int mode_ = mode;
            int length = 1;
            float covered = (float)process_query(1, temp, 0);
            while (mode_ != 0) {
                int type = mode_ % 10;
                if (type==1) {
                    //covered
                    cover_twt[twt_num] += covered;
                }
                else if(type==2) {
                    //topic covered
                    if (covered == 1.0) {
                        for (int topic_num=0;topic_num<tweet[twt_num].topic.size();topic_num++)
                            cover_twt[twt_num] += (float)tweet[twt_num].topic[topic_num];
                    }
                }
                else if(type==3) {
                    //out-link
                    if (covered == 1.0) {
                        cover_twt[twt_num] += (float)tweet[twt_num].rtw_count/MAX_OUT;
                    }
                }
                else if(type==4) {
                    //time reward
                    cover_twt[twt_num] += (float)process_query(5, temp, 0);
                }
                mode_ /= 10;
                length += 1;
            }
            cover_twt[twt_num] /= (float)length-1.0;
            temp_z += cover_twt[twt_num];
            
        }
        exp << chs[itr] << " " << temp_z << "\n";
        cout << chs[itr] << " " << temp_z << endl;
        total_z += chs[itr]*temp_z;
        temp_z = 0.0;
        itr += 1;
    }
    
    
    
    cout << "calculate_obj time elapsed : " << (clock()-start)/CLOCKS_PER_SEC << endl;
    cout << "total_z : " << total_z << endl;
    cout << "the number of tweet : " << num_tweet << endl;
    exp << "cover : " << (float)total_z/num_tweet << "\n";
    cout << "cover : " << (float)total_z/num_tweet << endl;
    
    exp.close();
}

void ev1_train(int mode, int d_mode) {
    
    //  input : mode (<1 covered <2 topic coverage <3 retweet num. <4 time reward <1234 all)
    //          d_mode (data mode, <0 tweet data <1 bitcoin forum data)
    //  output : None (result will be printed on text file)
    
    //  test with train set (check whether algorithm is working properly or not)
    process_userdata(in);
    
    // social sensor 불러오기 -> 한 줄에 다 있다고 가정
    char t_str[1004];
    char c_str[100];
    //  result file : list of social sensor set, separated with '\n'
    //  choose file : list of percentage for each social sensor set, separated with '\n'
    string result_file = "";
    string choose_file = "";
    if (d_mode == 0) {
        result_file = "twt_result/result_"+to_string(d_mode)+"_"+to_string(mode)+"_"+to_string(ss)+"_"+to_string(itr)+to_string(max_prob)+".txt";
        choose_file = "twt_result/choose_"+to_string(d_mode)+"_"+to_string(mode)+"_"+to_string(ss)+"_"+to_string(itr)+to_string(max_prob)+".txt";
    }
    else if (d_mode == 1) {
        result_file = "bitcoin_result/result_"+to_string(d_mode)+"_"+to_string(mode)+"_"+to_string(ss)+"_"+to_string(itr)+to_string(max_prob)+".txt";
        choose_file = "bitcoin_result/choose_"+to_string(d_mode)+"_"+to_string(mode)+"_"+to_string(ss)+"_"+to_string(itr)+to_string(max_prob)+".txt";
    }
    else {
        cout << "ev1_train file location error" << endl;
    }
    
    ofstream exp;
    string file_name = "tev_"+to_string(d_mode)+"_"+to_string(mode)+"_"+to_string(ss)+"_"+to_string(itr)+to_string(max_prob)+".txt";
    exp.open(file_name);
    
    ifstream result(result_file);
    ifstream cs(choose_file);
    
    vector<float> chs;
    while(cs.getline(c_str, 100)) {
        chs.push_back(stof(c_str));
    }
    
    int itr = 0;
    int num_tweet = tweet.size();
    float total_z = 0.0;
    float temp_z = 0.0;
    clock_t start = clock();
    IloNum t = 0.0;
    float cover_twt[num_tweet];
    
    fill_n(cover_twt, num_tweet, 0.0);
    
    while(result.getline(t_str, 1004)) {
                fill_n(cover_twt, num_tweet, 0.0);
        
        if((itr > 2) && (chs[itr]==0.0)) {
            itr += 1;
            continue;
        }
        cout << itr << endl;
        
        for (int twt_num=0; twt_num<num_tweet;twt_num++) {
            string temp = to_string(twt_num) + " " + to_string(ss)+t_str;
            int mode_ = mode;
            int length = 1;
            float covered = (float)process_query(1, temp, 0);
            while (mode_ != 0) {
                int type = mode_ % 10;
                if (type==1) {
                    //covered
                    cover_twt[twt_num] += covered;
                }
                else if(type==2) {
                    //topic covered
                    if (covered == 1.0) {
                        for (int topic_num=0;topic_num<tweet[twt_num].topic.size();topic_num++)
                            cover_twt[twt_num] += (float)tweet[twt_num].topic[topic_num];
                    }
                }
                else if(type==3) {
                    //out-link
                    if (covered == 1.0) {
                        cover_twt[twt_num] += (float)tweet[twt_num].rtw_count/MAX_OUT;
                    }
                }
                else if(type==4) {
                    //time reward
                    cover_twt[twt_num] += (float)process_query(5, temp, 0);
                }
                mode_ /= 10;
                length += 1;
            }
            cover_twt[twt_num] /= (float)length-1.0;
            temp_z += cover_twt[twt_num];
            
        }
        exp << chs[itr] << " " << temp_z << "\n";
        cout << chs[itr] << " " << temp_z << endl;
        total_z += chs[itr]*temp_z;
        temp_z = 0.0;
        itr += 1;
    }
    
    
    
    cout << "calculate_obj time elapsed : " << (clock()-start)/CLOCKS_PER_SEC << endl;
    cout << "total_z : " << total_z << endl;
    cout << "the number of tweet : " << num_tweet << endl;
    exp << "cover : " << (float)total_z/num_tweet << "\n";
    cout << "cover : " << (float)total_z/num_tweet << endl;
    
    exp.close();
}


void ev2(int mode, int d_mode) {
    //  # check whether hedging function is working or not
    
    //  input : mode (<1 covered <2 topic coverage <3 retweet num. <4 time reward <1234 all)
    //          d_mode (data mode, <0 tweet data <1 bitcoin forum data)
    //  output : None (result will be printed on text file)
    
    //  test with test set
    process_userdata(test);
    
    char t_str[1004];
    char c_str[100];
    
    string result_file = "";
    string choose_file = "";
    if (d_mode == 0) {
        result_file = "twt_result/_result_"+to_string(d_mode)+"_"+to_string(mode)+"_"+to_string(ss)+"_"+to_string(itr)+to_string(max_prob)+".txt";
        choose_file = "twt_result/choose_"+to_string(d_mode)+"_"+to_string(mode)+"_"+to_string(ss)+"_"+to_string(itr)+to_string(max_prob)+".txt";
    }
    else if (d_mode == 1) {
        result_file = "bitcoin_result/_result_"+to_string(d_mode)+"_"+to_string(mode)+"_"+to_string(ss)+"_"+to_string(itr)+to_string(max_prob)+".txt";
        choose_file = "bitcoin_result/choose_"+to_string(d_mode)+"_"+to_string(mode)+"_"+to_string(ss)+"_"+to_string(itr)+to_string(max_prob)+".txt";
    }
    else {
        cout << "ev2 file location error!" << endl;
    }
    
    ofstream exp;
    string file_name = "_tev_"+to_string(d_mode)+"_"+to_string(mode)+"_"+to_string(ss)+"_"+to_string(itr)+to_string(max_prob)+".txt";
    exp.open(file_name);
    
    ifstream result(result_file);
    ifstream cs(choose_file);
    
    vector<float> chs;
    while(cs.getline(c_str, 100)) {
        chs.push_back(stof(c_str));
    }
    int itr = 0;
    int num_tweet = tweet.size();
    float total_z = 0.0;
    float temp_z = 0.0;
    clock_t start = clock();
    IloNum t = 0.0;
    float cover_twt[num_tweet];
    
    fill_n(cover_twt, num_tweet, 0.0);
    
    while(result.getline(t_str, 1004)) {
        cout << t_str << endl;
        
        fill_n(cover_twt, num_tweet, 0.0);
        
        if((itr > 2) && (chs[itr]==0.0)) {
            itr += 1;
            continue;
        }
    
        
        
        for (int twt_num=0; twt_num<tweet.size();twt_num++) {
            string temp = to_string(twt_num) + " " + t_str;
            int mode_ = mode;
            int length = 1;
            float covered = (float)process_query(1, temp, 0);
            while (mode_ != 0) {
                int type = mode_ % 10;
                if (type==1) {
                    //covered
                    cover_twt[twt_num] += covered;
                }
                else if(type==2) {
                    //topic covered
                    if (covered == 1.0) {
                        for (int topic_num=0;topic_num<tweet[twt_num].topic.size();topic_num++)
                            cover_twt[twt_num] += (float)tweet[twt_num].topic[topic_num];
                    }
                }
                else if(type==3) {
                    //out-link
                    if (covered == 1.0) {
                        cover_twt[twt_num] += (float)tweet[twt_num].rtw_count/MAX_OUT;
                    }
                }
                else if(type==4) {
                    //time reward
                    cover_twt[twt_num] += (float)process_query(5, temp, 0);
                }
                mode_ /= 10;
                length += 1;
            }
            cover_twt[twt_num] /= (float)length-1.0;
            temp_z += cover_twt[twt_num];
            
        }
        

        exp << chs[itr] << " " << temp_z << "\n";
        cout << chs[itr] << " " << temp_z << endl;
        
        total_z += chs[itr]*temp_z;
        temp_z = 0.0;
        itr += 1;
    }
    
    
    
    cout << "calculate_obj time elapsed : " << (clock()-start)/CLOCKS_PER_SEC << endl;
    cout << "total_z : " << total_z << endl;
    cout << "the number of tweet : " << num_tweet << endl;
    exp << "cover : " << (float)total_z/num_tweet << "\n";
    cout << "cover : " << (float)total_z/num_tweet << endl;
    
    exp.close();
}
void ev2_train(int mode, int d_mode) {
    //  input : mode (<1 covered <2 topic coverage <3 retweet num. <4 time reward <1234 all)
    //          d_mode (data mode, <0 tweet data <1 bitcoin forum data)
    //  output : None (result will be printed on text file)
    
    // test with train set (check whether algorithm is working properly or not)
    process_userdata(in);
    
    // load social sensor
    // we assume that members of one social sensor set is in one line
    char t_str[1004];
    char c_str[100];
    
    string result_file = "";
    string choose_file = "";
    if (d_mode == 0) {
        result_file = "twt_result/_result_"+to_string(d_mode)+"_"+to_string(mode)+"_"+to_string(ss)+"_"+to_string(itr)+to_string(max_prob)+".txt";
        choose_file = "twt_result/choose_"+to_string(d_mode)+"_"+to_string(mode)+"_"+to_string(ss)+"_"+to_string(itr)+to_string(max_prob)+".txt";
    }
    else if (d_mode == 1) {
        result_file = "bitcoin_result/_result_"+to_string(d_mode)+"_"+to_string(mode)+"_"+to_string(ss)+"_"+to_string(itr)+to_string(max_prob)+".txt";
        choose_file = "bitcoin_result/choose_"+to_string(d_mode)+"_"+to_string(mode)+"_"+to_string(ss)+"_"+to_string(itr)+to_string(max_prob)+".txt";
    }
    else {
        cout << "ev2_train file location error!" << endl;
    }
    
    ofstream exp;
    string file_name = "ev4/_tev_"+to_string(d_mode)+"_"+to_string(mode)+"_"+to_string(ss)+"_"+to_string(itr)+to_string(max_prob)+".txt";
    exp.open(file_name);
    
    ifstream result(result_file);
    ifstream cs(choose_file);
    
    vector<float> chs;
    while(cs.getline(c_str, 100)) {
        chs.push_back(stof(c_str));
    }
    
    int itr = 0;
    int num_tweet = tweet.size();
    float total_z = 0.0;
    float temp_z = 0.0;
    clock_t start = clock();
    IloNum t = 0.0;
    float cover_twt[num_tweet];
    
    fill_n(cover_twt, num_tweet, 0.0);
    
    while(result.getline(t_str, 1004)) {
        
        fill_n(cover_twt, num_tweet, 0.0);
        
        if((itr > 2) && (chs[itr]==0.0)) {
            itr += 1;
            continue;
        }
        cout << itr << endl;
        
        for (int twt_num=0; twt_num<num_tweet;twt_num++) {
            string temp = to_string(twt_num) + " " + t_str;
            int mode_ = mode;
            int length = 1;
            float covered = (float)process_query(1, temp, 0);
            while (mode_ != 0) {
                int type = mode_ % 10;
                if (type==1) {
                    //covered
                    cover_twt[twt_num] += covered;
                }
                else if(type==2) {
                    //topic covered
                    if (covered == 1.0) {
                        for (int topic_num=0;topic_num<tweet[twt_num].topic.size();topic_num++)
                            cover_twt[twt_num] += (float)tweet[twt_num].topic[topic_num];
                    }
                }
                else if(type==3) {
                    //out-link
                    if (covered == 1.0) {
                        cover_twt[twt_num] += (float)tweet[twt_num].rtw_count/MAX_OUT;
                    }
                }
                else if(type==4) {
                    //time reward
                    cover_twt[twt_num] += (float)process_query(5, temp, 0);
                }
                mode_ /= 10;
                length += 1;
            }
            cover_twt[twt_num] /= (float)length-1.0;
            temp_z += cover_twt[twt_num];
            
        }
        exp << chs[itr] << " " << temp_z << "\n";
        cout << chs[itr] << " " << temp_z << endl;
        total_z += chs[itr]*temp_z;
        temp_z = 0.0;
        itr += 1;
    }
    
    
    
    cout << "calculate_obj time elapsed : " << (clock()-start)/CLOCKS_PER_SEC << endl;
    cout << "total_z : " << total_z << endl;
    cout << "the number of tweet : " << num_tweet << endl;
    exp << "cover : " << (float)total_z/num_tweet << "\n";
    cout << "cover : " << (float)total_z/num_tweet << endl;
    
    exp.close();
}
double process_query(int type, string inp, int topic_num) {
    
    char t_str[1004];
    // input string processing
    // type=1 : inp = "tweet_num the_num_of_ppl_in_ss a b c"
    // type=2 : inp = "the_num_of_ppl_in_ss a b c"
    int idx = 0;
    string tokens[MAX_INP];
    stringstream ssin(inp);
    while (ssin.good() && idx < MAX_INP) {
        ssin >> tokens[idx];
        ++idx;
    }
    if (type == 1) {//coverd(W,S_i)
        int tweet_index, user_cnt;
        int i, j;
        string sensor_id;
        tweet_index = stoi(tokens[0]);
        user_cnt = stoi(tokens[1]);
        
        int flag = 0;
        if (tweet_index >= tweet.size()) {
            //Out of index
            flag = 1;
        }
        for (int it = 0; it < user_cnt; it++) {
            strncpy(t_str, tokens[it+2].c_str(), sizeof(t_str)-1);
            
            if(sizeof(t_str)>0)
                t_str[sizeof(t_str)-1] = 0;
            if (flag == 0) {
                sensor_id = t_str;
                int len = user[sensor_id].tweets.size();
                if (!user.count(t_str)) {
                    flag = 1; continue;
                }
                for (i = 0; i < len; i++)
                {
                    if (tweet[user[sensor_id].tweets[i]].identifier == tweet[tweet_index].identifier) {
                        flag = 1;
                        return 1.0;
                    }
                }
            }
        }
        if (flag == 0) return 0.0;//fprintf(query_out, "0\n");//Not Covered
        return 0.0;
    }
    else if (type == 2) {//coverd(W_total,S_i)
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
    else if (type == 3) {// Total Tweet Count
        //fprintf(query_out, "%d\n", tweet.size());//Not Covered
        return tweet.size();
    }
    else if (type == 4) {// Total User Count
        //fprintf(query_out, "%d\n", user.size());//Not Covered
        return user.size();
    }
    else if (type == 5) {
        // delay(W,S_i)를 질의하는 쿼리
        // input string 형식 :
        
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
            return 0.0;
        }
        string tw_identifier = tweet[tweet_index].identifier;
        long long minv = 7948725;
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
                if (tweet[user[sensor_id].tweets[i]].identifier == tw_identifier) {
                    //Covered
                    if (minv > (tweet[user[sensor_id].tweets[i]].time - tweet[tweet[user[sensor_id].tweets[i]].root_index].time)) {
                        minv = tweet[user[sensor_id].tweets[i]].time - tweet[tweet[user[sensor_id].tweets[i]].root_index].time;                    }
                }
            }
        }
        return 1.0-(double)minv/7948725.0;
    }
    else if (type == 6) {//
        int i, j, k;
        FILE *query_out = fopen("./celf_training.txt", "w");
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
        // run CELF algorithm
        
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
    else if (type == 8) {
        map<string, double> covering;
        string sensor_id;
        int i, user_cnt;
        double cnt = 0;
        int flag = 0;
        user_cnt = stoi(tokens[1]);
        //fscanf(query_in, "%d", &user_cnt);
        for (int it = 0; it < user_cnt; it++) {
            strncpy(t_str, tokens[it+2].c_str(), sizeof(t_str)-1);
            if(sizeof(t_str)>0)
                t_str[sizeof(t_str)-1] = 0;
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
                    covering[tweet[user[sensor_id].tweets[i]].identifier] = tweet[user[sensor_id].tweets[i]].topic[topic_num];
                }
            }
        }
        
        
        for (map<string, double>::iterator it = covering.begin(); it != covering.end(); ++it) {
            cnt += it->second;
        }
        
        return cnt;
    }
    
    return 0.0;
}

void pricing_algorithm(int n, IloNumArray dv) {
    new_ss_idx.clear();
    
    
    map<string, IloNum> covering;
    int i, j;
    //fscanf(query_in, "%d", &n);
    int usernum = indexToUser.size();
    int hsize = 0;
    for (i = 0; i < usernum; i++) { // fill in initial heap
        
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
        push_heap(sheap, sheap + hsize, sensor_cmp);
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
            pop_heap(sheap, sheap + hsize, sensor_cmp);
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
            push_heap(sheap, sheap + hsize, sensor_cmp);
            covering.clear();
            forcheck++;
        }
        printf("%d번만에 찾음\n", forcheck);
        new_ss_idx.push_back(sheap[0].index);
        cout << "User : " << indexToUser[sheap[0].index] << " point : " << sheap[0].point <<endl;
        //fprintf(query_out, "%d\n", sheap[0].index);
        //fprintf(query_out, "%d개의 트윗을 만듦\n", user[indexToUser[sheap[0].index]].tweets.size());
        pop_heap(sheap, sheap + hsize, sensor_cmp);
        hsize--;
    }
    for(i=0;i<tweet.size();i++){
        tweet[i].is_covered=0;
    }
}
IloNumArray run_iteration(IloEnv env, IloModel curOpt, IloObjective obj, IloRangeArray constraints, int num_tweet, int mode, IloNumVarArray choose) {
    IloNumArray dv(env, num_tweet);
    
    curOpt.add(obj);
    curOpt.add(constraints);
    curOpt.add(IloSum(choose)<=1);
    
    IloCplex cplex(curOpt);
    //cplex.exportModel("model.lp");
    cplex.solve();
    
    IloNumArray ch(env);
    cplex.getValues(ch, choose);
    
    // choose 확률
    ofstream res;
    res.open("choose_"+to_string(d_mode)+"_"+to_string(mode)+"_"+to_string(ss)+"_"+to_string(itr)+to_string(max_prob)+".txt");

    for (int t=0;t<ch.getSize();t++) {
        cout << "choose " << t << " : " << ch[t] << endl;
        res << ch[t] << "\n";
    }
    res << "\n";
    
    for (int t=0;t<num_tweet;t++) {
        IloNumArray csts(env, ch.getSize());
        int t_ = 0;
        for (IloExpr::LinearIterator it = constraints[t].getLinearIterator();it.ok();++it) {
            csts[t_] = it.getCoef();
            t_ ++;
        }
        dv[t] = IloNum(1) - IloScalProd(ch, csts);
    }
    
    //cplex.getDuals(dv, constraints);
    
    // dv 값 파일에 출력하기
    // print out dv values on text file (file_name)
    ofstream myfile;
    string file_name = "dv_"+to_string(mode)+"_"+to_string(ss)+"_"+to_string(itr);
    myfile.open(file_name);
    for (int h=0;h<dv.getSize();h++) {
        myfile << dv[h] << "\n";
    }
    myfile.close();
    
    
    return dv;
}


//  트위터 데이터로 트리 만들기
//  build a tree with tweet data
void process_userdata(FILE *in)
{
    // 초기화 해주기
    // initialization
    tweet.clear();
    user.clear();
    
    int i, j, k;
    _tweet temp;
    char t_str[1000004];
    int t_tnum;
    float t_int;
    int save=0;
    k = 1;
    while (true)
    {
        k++;
        //cout << save << endl;
        fscanf(in, "%d", &temp.type);
        if (temp.type == 2)break;
        fscanf(in, "%s", t_str);//user_id:
        if(strcmp(t_str,"user_id:")!=0)
        {
            cout<<"something is wrong ."<<endl;
            cout<<"user id: is "<<t_str<<endl;
            for(int pp=0;pp<100;pp++){
                fscanf(in, "%s", t_str);//topic:
                cout<<"\n "<<t_str<<endl;
            }
            for(;;);
        }
        fscanf(in, "%s", t_str);//temp.user_id
        temp.user_id = t_str;
        fscanf(in, "%s", t_str);//time:
        if(strcmp(t_str,"time:")!=0)
        {
            cout<<"something is wrong ."<<endl;
            cout<<"time: is "<<t_str<<endl;
            for(int pp=0;pp<100;pp++){
                fscanf(in, "%s", t_str);//topic:
                cout<<"\n "<<t_str<<endl;
            }
            for(;;);
        }
        fscanf(in, "%lld.0", &temp.time);
        int flag=0;
        if (temp.type == 1) {
            fscanf(in, "%s ", t_str);//body:
            if(strcmp(t_str,"body:")!=0)
            {
                cout<<"something is wrong ."<<endl;
                cout<<"body: is "<<t_str<<endl;
                for(int pp=0;pp<100;pp++){
                    fscanf(in, "%s", t_str);//topic:
                    cout<<"\n "<<t_str<<endl;
                }
                for(;;);
            }
            save = tweet.size();//가장 최근에 나온 root tweet을 저장함. //save most recent root tweet
            fgets(t_str, 1000000, in);
            temp.body = t_str;//body 내용을 입력받음.
            if(strcmp(t_str,"topic:")==0||strlen(t_str)==0)
            {
                flag=1;
                for(int pp=0;pp<100;pp++){
                    fscanf(in, "%s", t_str);//topic:
                    cout<<"\n "<<t_str<<endl;
                }
                for(;;);
            }
        }
        else {
            temp.body = "";
        }
        if(flag==0)
        {
            fscanf(in, "%s", t_str);//topic:
            
            if(strcmp(t_str,"topic:")!=0){
                cout<<"something is wrong ."<<endl;
                cout<<"topic: is "<<t_str<<endl;
                for(int pp=0;pp<100;pp++){
                    fscanf(in, "%s", t_str);//topic:
                    cout<<"\n "<<t_str<<endl;
                }
                for(;;);
            }
        }
        else flag=0;
        fscanf(in, "%d", &t_tnum);//topic num 입력받음 //get topic num input
        temp.topic.clear();
        for(i=0;i<t_tnum;i++){
            fscanf(in, "%f", &t_int);
            temp.topic.push_back(t_int);
        }
        fscanf(in, "%s", t_str);//rtw_count:
        if(strcmp(t_str,"rtw_count:")!=0)
        {
            cout<<"something is wrong ."<<endl;
            cout<<"rtw_count: is "<<t_str<<endl;
            for(int pp=0;pp<100;pp++){
                fscanf(in, "%s", t_str);//topic:
                cout<<"\n "<<t_str<<endl;
            }
            for(;;);
        }
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
        else {//부모일 경우엔 identifier을 입력받는다. //get identifier in the case of parent
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
    cout << tweet.size() << endl;
}




