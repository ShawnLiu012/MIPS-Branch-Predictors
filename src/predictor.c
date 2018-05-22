//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include <math.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "Sha Liu";
const char *studentID   = "A53239717";
const char *email       = "{shl237@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//
uint8_t *globalBHT;
uint8_t *localBHT;
uint8_t *localPHT;
uint8_t *chooserTable;
uint8_t  globalOutcome;
uint8_t  localOutcome;
uint32_t ghistoryReg;
uint32_t globalIdx;
uint32_t localIdx;
uint32_t patternIdx;


void init_gshare() 
{
    globalBHT = malloc((1 << ghistoryBits) * sizeof(uint8_t));
    memset(globalBHT, WN, (1 << ghistoryBits) * sizeof(uint8_t));  // Initiating as weakly not taken 
    ghistoryReg = 0;
}

uint8_t pred_gshare(uint32_t pc) 
{
    uint32_t temp = pc ^ ghistoryReg;
    uint32_t mask = (1 << ghistoryBits) - 1;
    globalIdx = temp & mask;  
    if(globalBHT[globalIdx] == SN || globalBHT[globalIdx] == WN)
        return NOTTAKEN;
    else
        return TAKEN;
}

void train_gshare(uint32_t pc, uint8_t outcome)
{
    // uint32_t temp = pc ^ ghistoryReg;
    // uint32_t mask = (1 << ghistoryBits) - 1;
    // uint32_t index = temp & mask;  
    if(outcome)
    {
        if(globalBHT[globalIdx] != ST)
            globalBHT[globalIdx]++;
    }
    else
    {
        if(globalBHT[globalIdx] != SN)
            globalBHT[globalIdx]--;
    }
    ghistoryReg = ghistoryReg << 1;
    ghistoryReg += outcome; 
}

void init_tournament() 
{
    globalBHT = malloc((1 << ghistoryBits) * sizeof(uint8_t));
    localBHT = malloc((1 << lhistoryBits) * sizeof(uint8_t));
    localPHT = malloc((1 << pcIndexBits) * sizeof(uint32_t));
    chooserTable = malloc((1 << ghistoryBits) * sizeof(uint8_t));
    
    memset(globalBHT, WN, (1 << ghistoryBits) * sizeof(uint8_t));
    memset(localBHT, WN, (1 << lhistoryBits) * sizeof(uint8_t));
    memset(localPHT, 0, (1 << pcIndexBits) * sizeof(uint32_t));
    memset(chooserTable, WN, (1 << ghistoryBits) * sizeof(uint8_t));

    ghistoryReg = 0;
}

uint8_t pred_tournament(uint32_t pc) 
{
    uint32_t mask = (1 << ghistoryBits) - 1;
    globalIdx = ghistoryReg & mask;  
    if(chooserTable[globalIdx] == SN || chooserTable[globalIdx] == WN)
        return globalOutcome;
    else
        return localOutcome;
}

void train_tournament(uint32_t pc, uint8_t outcome)
{
    if(outcome)
    {
        if(globalBHT[globalIdx] != ST)
            globalBHT[globalIdx]++;
    }
    else
    {
        if(globalBHT[globalIdx] != SN)
            globalBHT[globalIdx]--;
    }

    patternIdx = pc & ((1 << pcIndexBits) - 1);
    localPHT[patternIdx] = localPHT[patternIdx] << 1;
    localPHT[patternIdx] += outcome;

    localIdx = localPHT[patternIdx] & ((1 << lhistoryBits) - 1);
    if(outcome)
    {
        if(localBHT[localIdx] != ST)
            localBHT[localIdx]++;
    }
    else
    {
        if(localBHT[localIdx] != SN)
            localBHT[localIdx]--;   
    }

    // Updating the chooser table  
    if(globalOutcome == outcome && localOutcome != outcome)
    {
        if(chooserTable[globalIdx] != SN)
            chooserTable[globalIdx]--;
    }

    if(globalOutcome != outcome && localOutcome == outcome)
    {
        if(chooserTable[globalIdx] != ST)
            chooserTable[globalIdx]++;
    }

    ghistoryReg = ghistoryReg << 1;
    ghistoryReg += outcome; 
}


//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void init_predictor()
{
    //
    //TODO: Initialize Branch Predictor Data Structures
    //
    switch (bpType) {
        case STATIC:
            break;
        case GSHARE:
            init_gshare();
            break;
        case TOURNAMENT:
            // init_tournament();
            break;
        case CUSTOM:
            // init_custom();
            break;
        default:
            break;
    }

}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t make_prediction(uint32_t pc)
{
    //
    //TODO: Implement prediction scheme
    //

    // Make a prediction based on the bpType
    switch (bpType)
    {
        case STATIC:
            return TAKEN;
        case GSHARE:
            return pred_gshare(pc);
        case TOURNAMENT:
            // gpred = pred_global(pc);
            // lpred = pred_local(pc);
            // return pred_tournament(pc);
            break;
        case CUSTOM:
            // return pred_bimode(pc);
            break;
        default:
            break;
    }

    // If there is not a compatable bpType then return NOTTAKEN
    return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void train_predictor(uint32_t pc, uint8_t outcome)
{
    //
    //TODO: Implement Predictor training
    //
    switch (bpType)
    {
        case STATIC:
            break;
        case GSHARE:
            train_gshare(pc, outcome);
            break;
        case TOURNAMENT:
            // pred_tournament(outcome);
            break;
        case CUSTOM:
            // pred_bimode(outcome);
            break;
        default:
            break;
    }
}


