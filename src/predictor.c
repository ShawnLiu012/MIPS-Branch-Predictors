//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include <string.h> // for memset()
#include "predictor.h"

//==================================================================//
//  Custom Predictor is based on                                    //
//  The Bi-Mode Branch Predictor (Lee, Chen, and Mudge, 1997)       //
//  https://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=645792     //
//  Hardware Cost:                                                  //
//  Chooser BHT (3-bits)            : 3 * 2 ^ 11 = 6144 bits        //
//  Direction Taken BHT (2-bits)    : 2 * 2 ^ 11 = 4096 bits        //
//  Direction Not Taken BHT (2-bits): 2 * 2 ^ 11 = 4096 bits        //
//  Global History Register         :                11 bits        //
//  Total                           :             14347 bits        //
//==================================================================//

//
// TODO:Student Information
//
const char *studentName = "Sha Liu";
const char *studentID   = "A53239717";
const char *email       = "shl237@ucsd.edu";

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
uint32_t  ghistoryReg;

uint8_t  *globalBHT;
uint8_t  *localBHT;
uint32_t *localPHT;
uint8_t  *chooserBHT;

uint8_t  *takenBHT;
uint8_t  *notTakenBHT;

uint8_t   gOutcome;
uint8_t   lOutcome;

uint32_t  globalIdx;
uint32_t  localIdx;
uint32_t  patternIdx;

uint32_t  directionIdx;
uint32_t  chooserIdx;


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
            init_tournament();
            break;
        case CUSTOM:
            init_bimode();
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
            return pred_tournament(pc);
            break;
        case CUSTOM:
            return pred_bimode(pc);
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
            train_tournament(pc, outcome);
            break;
        case CUSTOM:
            train_bimode(pc, outcome);
            break;
        default:
            break;
    }
}

//------------------------------------//
//     Helper Functions For Gshare    //
//------------------------------------//

void init_gshare() 
{
    globalBHT = malloc((1 << ghistoryBits) * sizeof(uint8_t));
    memset(globalBHT, WN, (1 << ghistoryBits) * sizeof(uint8_t)); 
    ghistoryReg = 0;
}

uint8_t pred_gshare(uint32_t pc) 
{
    globalIdx = (pc ^ ghistoryReg) & ((1 << ghistoryBits) - 1);  
    if(globalBHT[globalIdx] == SN || globalBHT[globalIdx] == WN)
        return NOTTAKEN;
    else
        return TAKEN;
}

void train_gshare(uint32_t pc, uint8_t outcome)
{ 
    if(outcome == TAKEN)
    {
        if(globalBHT[globalIdx] < ST)
            globalBHT[globalIdx]++;
    }
    else
    {
        if(globalBHT[globalIdx] > SN)
            globalBHT[globalIdx]--;
    }

    ghistoryReg = ghistoryReg << 1;
    ghistoryReg += outcome; 
}


//------------------------------------//
//  Helper Functions For Tournament   //
//------------------------------------//

void init_tournament() 
{
    globalBHT  = malloc((1 << ghistoryBits) * sizeof(uint8_t));
    localBHT   = malloc((1 << lhistoryBits) * sizeof(uint8_t));
    localPHT   = malloc((1 << pcIndexBits) * sizeof(uint32_t));
    chooserBHT = malloc((1 << ghistoryBits) * sizeof(uint8_t));
    
    memset(globalBHT, WN, (1 << ghistoryBits) * sizeof(uint8_t));
    memset(localBHT, WN, (1 << lhistoryBits) * sizeof(uint8_t));
    memset(localPHT, 0, (1 << pcIndexBits) * sizeof(uint32_t));
    memset(chooserBHT, WN, (1 << ghistoryBits) * sizeof(uint8_t));

    ghistoryReg = 0;
}

uint8_t pred_tournament_global(uint32_t pc) 
{
    globalIdx = ghistoryReg & ((1 << ghistoryBits) - 1);

    if(globalBHT[globalIdx] < WT)
        return NOTTAKEN;
    else
        return TAKEN;
}

uint8_t pred_tournament_local(uint32_t pc) 
{
    patternIdx = pc & ((1 << pcIndexBits) - 1);
    localIdx = localPHT[patternIdx];

    if(localBHT[localIdx] < WT)
        return NOTTAKEN;
    else
        return TAKEN;
}

uint8_t pred_tournament(uint32_t pc) 
{ 
    gOutcome = pred_tournament_global(pc);
    lOutcome = pred_tournament_local(pc);

    globalIdx = ghistoryReg & ((1 << ghistoryBits) - 1); 

    if(chooserBHT[globalIdx] < WT)
        return gOutcome;
    else
        return lOutcome;
}


void train_tournament(uint32_t pc, uint8_t outcome)
{ 
    if(gOutcome != outcome && lOutcome == outcome)
    {
        if(chooserBHT[globalIdx] < ST)
            chooserBHT[globalIdx]++;
    }
    if(gOutcome == outcome && lOutcome != outcome)
    {
        if(chooserBHT[globalIdx] > SN)
            chooserBHT[globalIdx]--;
    }
    
    patternIdx = pc & ((1 << pcIndexBits) - 1);
    localIdx = localPHT[patternIdx];

    if(outcome == TAKEN)
    {
        if(localBHT[localIdx] < ST)
            localBHT[localIdx]++;
    }
    else
    {
        if(localBHT[localIdx] > SN)
            localBHT[localIdx]--;   
    }

    localPHT[patternIdx] = localPHT[patternIdx] << 1;
    localPHT[patternIdx] &= ((1 << lhistoryBits) - 1);
    localPHT[patternIdx] += outcome;

    if(outcome == TAKEN)
    {
        if(globalBHT[globalIdx] < ST)
            globalBHT[globalIdx]++;
    }
    else
    {
        if(globalBHT[globalIdx] > SN)
            globalBHT[globalIdx]--;
    }

    ghistoryReg = ghistoryReg << 1;
    ghistoryReg &= ((1 << ghistoryBits) - 1);
    ghistoryReg += outcome; 
}


//------------------------------------//
//     Helper Functions For Bimode    //
//------------------------------------//

void init_bimode() 
{
    ghistoryReg  = 0;

    ghistoryBits = 11;
    pcIndexBits  = 11;

    takenBHT     = malloc((1 << ghistoryBits) * sizeof(uint8_t));
    notTakenBHT  = malloc((1 << ghistoryBits) * sizeof(uint8_t));
    chooserBHT   = malloc((1 << pcIndexBits) * sizeof(uint8_t));
    
    memset(takenBHT, WT, (1 << ghistoryBits) * sizeof(uint8_t));
    memset(notTakenBHT, WN, (1 << ghistoryBits) * sizeof(uint8_t));
    memset(chooserBHT, WWN, (1 << pcIndexBits) * sizeof(uint8_t));   
}

uint8_t pred_bimode(uint32_t pc) 
{
    directionIdx = (pc ^ ghistoryReg) & ((1 << ghistoryBits) - 1);
    chooserIdx   = pc & ((1 << pcIndexBits) - 1);

    if(chooserBHT[chooserIdx] < WWT)
    {
        // Use direction not-taken BHT
        if(notTakenBHT[directionIdx] < WT)
            return NOTTAKEN;
        else
            return TAKEN;
    }
    else
    {
        // Use direction taken BHT
        if(takenBHT[directionIdx] < WT)
            return NOTTAKEN;
        else
            return TAKEN;
    }
}

void train_bimode(uint32_t pc, uint8_t outcome)
{
    // Update chooser BHT
    if(outcome == TAKEN)
    {
        if(chooserBHT[chooserIdx] < WWT)
        {
            if(notTakenBHT[directionIdx] < WT)
                chooserBHT[chooserIdx]++;
        }
        else
        {
            if(chooserBHT[chooserIdx] < SST)
                chooserBHT[chooserIdx]++;
        }
    }
    else
    {
        if(chooserBHT[chooserIdx] > WWN)
        {
            if(takenBHT[directionIdx] > WN)
                chooserBHT[chooserIdx]--;
        }
        else
        {
            if(chooserBHT[chooserIdx] > SSN)
                chooserBHT[chooserIdx]--;
        }
    }

    // Update direction taken BHT and direction not-taken BHT
    if(chooserBHT[chooserIdx] < WWT)
    {
        if(outcome == TAKEN)
        {
            if(notTakenBHT[directionIdx] < ST)
                notTakenBHT[directionIdx]++;
        }
        else
        {
            if(notTakenBHT[directionIdx] > SN)
                notTakenBHT[directionIdx]--;
        }

    }
    else
    {
        if(outcome == TAKEN)
        {
            if(takenBHT[directionIdx] < ST)
                takenBHT[directionIdx]++;
        }
        else
        {
            if(takenBHT[directionIdx] > SN)
                takenBHT[directionIdx]--;
        }
    }

    // Shift and mask GHR
    ghistoryReg = ghistoryReg << 1;
    ghistoryReg &= ((1 << ghistoryBits) - 1);
    ghistoryReg += outcome; 
}

