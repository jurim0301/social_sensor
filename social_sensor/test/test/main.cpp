//
//  main.cpp
//  test
//
//  Created by Jurim Lee on 2018. 5. 9..
//  Copyright © 2018년 Jurim Lee. All rights reserved.
//

#define MAX_INP 30
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

using namespace std;
int main(int argc, const char * argv[]) {
    // insert code here...
    int idx = 0;
    string inp = "1 Hello 2 inp";
    string arr[MAX_INP];
    stringstream ssin(inp);
    while (ssin.good() && idx < MAX_INP) {
        ssin >> arr[idx];
        ++idx;
    }
    for (int i=0;i<idx;i++)
        cout << arr[i] << endl;
    cout << idx << endl;
    
    string temp;
    
    
    cout << temp << endl;
    
    return 0;
}
