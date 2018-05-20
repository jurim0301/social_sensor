//
//  evaluation.cpp
//  social_sensor
//
//  Created by Jurim Lee on 2018. 4. 30..
//  Copyright © 2018년 Jurim Lee. All rights reserved.
//

#include "evaluation.hpp"

void ev1_objective_function_comparison(int mode) {
    //  1. Objective Function Comparison
    //      4개의 다른 값 (covered, time, topic coverage, out-degree)의 비교
    //      x축 : 선택할 수 있는 social sensor의 개수
    //      y축 : reward (normalize한 값의 비교)
    
    
}

void ev2_objective_functions_comparison() {
    //  2. 1에서 한 실험에서 값을 2개 혹은 3개로 묶어서 비교
    //      x축 : 선택할 수 있는 social sensor의 개수
    //      y축 : reward (normalize한 값의 합?의 비교)
    
}

void ev3_train_test() {
    //  3. Jure 논문의 7번 실험 - 50% 데이터로 SS구하고 -> 나머지 50%로 reward
    //      x축 : cost (Jure에서는 cost로 함) 총 포함되는 데이터의 개수 (트윗 글의 개수)
    //      y축 : reward (normalize한 값의 합의 비교)
    
}

void ev4_subrationality() {
    //  4. Subrationality, bounded-reality 이론 적용
    //      이론 : 사람은 항상 같은 선택을 하지 않는다고 가정. 선택이 달라질 수 있음
    //      적용 : test set을 조금씩 바꿈 (retweet pattern swapping, crossover - genetic algorithm) => 이걸로 실험해보기
    //      실험 : pure strategy로 social sensor구한 경우와 mixed strategy로 social sensor구한 경우의 reward 비교
    //              (만약 시간 있으면 Weibo VIP을 ground-truth로 한 실험 진행)
    
}

